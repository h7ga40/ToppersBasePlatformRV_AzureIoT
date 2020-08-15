/* Copyright (c) 2014, Nordic Semiconductor ASA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** @file
@brief Implementation of the ACI transport layer module
*/

#include "hal_platform.h"
#include "hal_aci_tl.h"
#include "aci_queue.h"

/*
PIC32 supports only MSbit transfer on SPI and the nRF8001 uses LSBit
Use the REVERSE_BITS macro to convert from MSBit to LSBit
The outgoing command and the incoming event needs to be converted
*/
//Board dependent defines
//For ChipKit as the transmission has to be reversed, the next definitions have to be added
#ifdef __STM32__
#define REVERSE_BITS(byte) (byte)
#else
#define REVERSE_BITS(byte) (((reverse_lookup[(byte & 0x0F)]) << 4) + reverse_lookup[((byte & 0xF0) >> 4)])
static const uint8_t reverse_lookup[] = { 0, 8,  4, 12, 2, 10, 6, 14,1, 9, 5, 13,3, 11, 7, 15 };
#endif

static void m_aci_data_print(hal_aci_data_t *p_data);
static void m_aci_event_check(void);
static void m_aci_pins_set(aci_pins_t *a_pins_ptr);
static inline void m_aci_reqn_disable (void);
static inline void m_aci_reqn_enable (void);
static void m_aci_q_flush(void);
static bool m_aci_spi_transfer(hal_aci_data_t * data_to_send, hal_aci_data_t * received_data);

static uint8_t        spi_readwrite(uint8_t aci_byte);


static bool           aci_debug_print = false;

aci_queue_t    aci_tx_q;
aci_queue_t    aci_rx_q;

static aci_pins_t	 *a_pins_local_ptr;

static SPI_Handle_t *hspi;
static int evt_taskid;
static int aci_activate;

/* Buffer used for transmission */
uint8_t aTxBuffer[4];

/* Buffer used for reception */
uint8_t aRxBuffer[4];

static uint8_t spi_transfer(uint8_t a)
{
  ER ercd;

  aTxBuffer[0] = a;
#if SPI_WAIT_TIME != 0
  ercd = spi_transrecv(hspi, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, 1);
#else
  if((ercd = spi_transrecv(hspi, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, 1)) == E_OK)
    ercd = spi_wait(hspi, 100);
#endif
  if(ercd != E_OK){
    /* Transfer error in transmission process */
    syslog_1(LOG_NOTICE, "## call Error_Handler(2)(%d) ##", ercd);
  }
  return aRxBuffer[0];
}

/*
 *  GPIO設定関数
 */
static void pinMode(uint32_t no, int mode)
{
  Arduino_PortControlBlock *ppcb = getGpioTable(DIGITAL_PIN, no);
  GPIO_Init_t GPIO_Init_Data;

  syslog_2(LOG_NOTICE, "## pinMode(%d, %d) ##", no, mode);
  syslog_2(LOG_NOTICE, "## [%08x](%d) ##", ppcb->giobase, ppcb->giopin);
  if(ppcb == NULL)
    return;
  pinClock(no);
  if(mode == OUTPUT){
    GPIO_Init_Data.mode      = GPIO_MODE_OUTPUT;
    GPIO_Init_Data.pull      = GPIO_PULLUP;
    GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
#ifdef TOPPERS_BASE_PLATFORM_RV
	GPIO_Init_Data.oxor  = GPIO_OUTPUT_NORMAL;
	GPIO_Init_Data.iof   = GPIO_NOIOF;
#else
    GPIO_Init_Data.speed     = GPIO_SPEED_FAST;
#endif
    gpio_setup(ppcb->giobase, &GPIO_Init_Data, ppcb->giopin);
  }
  else{
    GPIO_Init_Data.mode      = GPIO_MODE_IT_RISING_FALLING;
    if(mode == INPUT_PULLUP)
      GPIO_Init_Data.pull    = GPIO_PULLUP;
    else
      GPIO_Init_Data.pull    = GPIO_NOPULL;
    gpio_setup(ppcb->giobase, &GPIO_Init_Data, ppcb->giopin);
  }
}

void m_aci_data_print(hal_aci_data_t *p_data)
{
  const uint8_t length = p_data->buffer[0];
  uint8_t i, j, len, data[5];

  len = length % 5;
  if(len == 0)
    len = length;
  else
    len = length + 5 - len;
  for (i=j=0; i< len; i++){
    if(i < length)
      data[j] = p_data->buffer[i+1];
    else
      data[j] = 0xff;
    if(j == 4){
       syslog_5(LOG_NOTICE, "%02x, %02x, %02x, %02x, %02x,", data[0], data[1], data[2], data[3], data[4]);
       j = 0;
    }
    else
       j++;
  }
}

/*
  Interrupt service routine called when the RDYN line goes low. Runs the SPI transfer.
*/
void
m_aci_isr(void)
{
  if(aci_activate == 0){
    if(evt_taskid != 0)
      iact_tsk(evt_taskid);
    else
      aci_task(0);
  }
}

void aci_task(int arg)
{
  hal_aci_data_t data_to_send;
  hal_aci_data_t received_data;
#ifdef NOACT_TASK	/* ROI DEBUG */
	syslog_1(LOG_NOTICE, "## aci_task(%d) ##", arg);
#endif	/* ROI DEBUG */

  aci_activate = 1;
  // Receive from queue
  if (!aci_queue_dequeue/*_from_isr*/(&aci_tx_q, &data_to_send))
  {
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }

  // Receive and/or transmit data
  if(!m_aci_spi_transfer(&data_to_send, &received_data)){
    m_aci_reqn_enable();
    aci_activate = 0;
    return;
  }

  if (!aci_queue_is_full/*_from_isr*/(&aci_rx_q) && !aci_queue_is_empty/*_from_isr*/(&aci_tx_q))
  {
    m_aci_reqn_enable();
  }

  // Check if we received data
  if (received_data.buffer[0] > 0)
  {
    if (!aci_queue_enqueue/*_from_isr*/(&aci_rx_q, &received_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
      */
      syslog_0(LOG_ERROR, "aci_task Buffer full !");
      while(1);
    }
  }
  aci_activate = 0;

  return;
}

/*
  Checks the RDYN line and runs the SPI transfer if required.
*/
static void m_aci_event_check(void)
{
  hal_aci_data_t data_to_send;
  hal_aci_data_t received_data;

  // No room to store incoming messages
  if (aci_queue_is_full(&aci_rx_q))
  {
    return;
  }

  // If the ready line is disabled and we have pending messages outgoing we enable the request line
  if (HIGH == digitalRead(a_pins_local_ptr->rdyn_pin))
  {
    if (!aci_queue_is_empty(&aci_tx_q))
    {
      m_aci_reqn_enable();
    }

    return;
  }

  // Receive from queue
  if (!aci_queue_dequeue(&aci_tx_q, &data_to_send))
  {
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }

  // Receive and/or transmit data
  m_aci_spi_transfer(&data_to_send, &received_data);

  /* If there are messages to transmit, and we can store the reply, we request a new transfer */
  if (!aci_queue_is_full(&aci_rx_q) && !aci_queue_is_empty(&aci_tx_q))
  {
    m_aci_reqn_enable();
  }

  // Check if we received data
  if (received_data.buffer[0] > 0)
  {
    if (!aci_queue_enqueue(&aci_rx_q, &received_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
      */
      syslog_0(LOG_ERROR, "m_aci_event_check Buffer full !");
      while(1);
    }
  }

  return;
}

/** @brief Point the low level library at the ACI pins specified
 *  @details
 *  The ACI pins are specified in the application and a pointer is made available for
 *  the low level library to use
 */
static void m_aci_pins_set(aci_pins_t *a_pins_ptr)
{
  a_pins_local_ptr = a_pins_ptr;
}

static inline void m_aci_reqn_disable (void)
{
  digitalWrite(a_pins_local_ptr->reqn_pin, 1);
}

static inline void m_aci_reqn_enable (void)
{
  digitalWrite(a_pins_local_ptr->reqn_pin, 0);
}

static void m_aci_q_flush(void)
{
  noInterrupts();
  /* re-initialize aci cmd queue and aci event queue to flush them*/
  aci_queue_init(&aci_tx_q);
  aci_queue_init(&aci_rx_q);
  interrupts();
}

static bool m_aci_spi_transfer(hal_aci_data_t * data_to_send, hal_aci_data_t * received_data)
{
  uint8_t byte_cnt;
  uint8_t byte_sent_cnt;
  uint8_t max_bytes;

  m_aci_reqn_enable();

  // Send length, receive header
  byte_sent_cnt = 0;
  received_data->status_byte = spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  // Send first byte, receive length from slave
  received_data->buffer[0] = spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  if(received_data->buffer[0] == 0xff)
    return false;
  if (0 == data_to_send->buffer[0])
  {
    max_bytes = received_data->buffer[0];
  }
  else
  {
    // Set the maximum to the biggest size. One command byte is already sent
    max_bytes = (received_data->buffer[0] > (data_to_send->buffer[0] - 1))
                                          ? received_data->buffer[0]
                                          : (data_to_send->buffer[0] - 1);
  }

  if (max_bytes > HAL_ACI_MAX_LENGTH)
  {
    max_bytes = HAL_ACI_MAX_LENGTH;
  }

  // Transmit/receive the rest of the packet
  for (byte_cnt = 0; byte_cnt < max_bytes; byte_cnt++)
  {
    received_data->buffer[byte_cnt+1] =  spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  }

  // RDYN should follow the REQN line in approx 100ns
  m_aci_reqn_disable();

  return (max_bytes > 0);
}

void hal_aci_tl_debug_print(bool enable)
{
	aci_debug_print = enable;
}

void hal_aci_tl_pin_reset(void)
{
    if (UNUSED != a_pins_local_ptr->reset_pin)
    {
        pinMode(a_pins_local_ptr->reset_pin, OUTPUT);

        if ((REDBEARLAB_SHIELD_V1_1     == a_pins_local_ptr->board_name) ||
            (REDBEARLAB_SHIELD_V2012_07 == a_pins_local_ptr->board_name))
        {
            //The reset for the Redbearlab v1.1 and v2012.07 boards are inverted and has a Power On Reset
            //circuit that takes about 100ms to trigger the reset
            digitalWrite(a_pins_local_ptr->reset_pin, 1);
            delay(100);
            digitalWrite(a_pins_local_ptr->reset_pin, 0);
        }
        else
        {
            digitalWrite(a_pins_local_ptr->reset_pin, 1);
            digitalWrite(a_pins_local_ptr->reset_pin, 0);
            digitalWrite(a_pins_local_ptr->reset_pin, 1);
        }
    }
}

bool hal_aci_tl_event_peek(hal_aci_data_t *p_aci_data)
{
  if (!a_pins_local_ptr->interface_is_interrupt)
  {
    m_aci_event_check();
  }

  if (aci_queue_peek(&aci_rx_q, p_aci_data))
  {
    return true;
  }

  return false;
}

bool hal_aci_tl_event_get(hal_aci_data_t *p_aci_data)
{
  bool was_full;

  if (!a_pins_local_ptr->interface_is_interrupt && !aci_queue_is_full(&aci_rx_q))
  {
    m_aci_event_check();
  }

  was_full = aci_queue_is_full(&aci_rx_q);

  if (aci_queue_dequeue(&aci_rx_q, p_aci_data))
  {
    if (aci_debug_print)
    {
      syslog_1(LOG_NOTICE, "EVT %d:", p_aci_data->buffer[0]);
      m_aci_data_print(p_aci_data);
    }

    /* Attempt to pull REQN LOW since we've made room for new messages */
    if (!aci_queue_is_full(&aci_rx_q) && !aci_queue_is_empty(&aci_tx_q))
    {
      m_aci_reqn_enable();
    }

    return true;
  }

  (void)(was_full);
  return false;
}

void hal_aci_tl_init(aci_pins_t *a_pins, bool debug)
{
  aci_debug_print = debug;

  /* Needs to be called as the first thing for proper intialization*/
  m_aci_pins_set(a_pins);

  /*
  The SPI lines used are mapped directly to the hardware SPI
  MISO MOSI and SCK
  Change here if the pins are mapped differently

  The SPI library assumes that the hardware pins are used
  */
  hspi = a_pins->spihandle;
  evt_taskid = a_pins->event_taskid;

  /* Initialize the ACI Command queue. This must be called after the delay above. */
  aci_queue_init(&aci_tx_q);
  aci_queue_init(&aci_rx_q);

  //Configure the IO lines
  pinMode(a_pins->rdyn_pin,		INPUT_PULLUP);
  pinMode(a_pins->reqn_pin,		OUTPUT);

  if (UNUSED != a_pins->active_pin)
  {
    pinMode(a_pins->active_pin,	INPUT);
  }
  /* Pin reset the nRF8001, required when the nRF8001 setup is being changed */
  hal_aci_tl_pin_reset();

  /* Set the nRF8001 to a known state as required by the datasheet*/
  digitalWrite(a_pins->reqn_pin, 1);

  dly_tsk(30); //Wait for the nRF8001 to get hold of its lines - the lines float for a few ms after the reset
}

bool hal_aci_tl_send(hal_aci_data_t *p_aci_cmd)
{
  const uint8_t length = p_aci_cmd->buffer[0];
  bool ret_val = false;

  if (length > HAL_ACI_MAX_LENGTH)
  {
    return false;
  }

  ret_val = aci_queue_enqueue(&aci_tx_q, p_aci_cmd);
  if (ret_val)
  {
    if(!aci_queue_is_full(&aci_rx_q))
    {
      // Lower the REQN only when successfully enqueued
      m_aci_reqn_enable();
    }

    if (aci_debug_print)
    {
      syslog_1(LOG_NOTICE, "CMD %d:", p_aci_cmd->buffer[0]);
      m_aci_data_print(p_aci_cmd);
    }
  }

  return ret_val;
}

static uint8_t spi_readwrite(const uint8_t aci_byte)
{
  //Board dependent defines
  //For ChipKit the transmission has to be reversed
  uint8_t tmp_bits;
  tmp_bits = spi_transfer(REVERSE_BITS(aci_byte));
#if 0	/* ROI DEBUG */
	{
		uint32_t tim;
		get_tim(&tim);
		syslog_5(LOG_NOTICE, "## spi_readwrite in1[%02x] in2[%02x] out1[%02x] out2[%02x] tim(%d) ##", aci_byte, REVERSE_BITS(aci_byte), tmp_bits, REVERSE_BITS(tmp_bits), tim);
	}
#endif	/* ROI DEBUG */
  return REVERSE_BITS(tmp_bits);
}

bool hal_aci_tl_rx_q_empty (void)
{
  return aci_queue_is_empty(&aci_rx_q);
}

bool hal_aci_tl_rx_q_full (void)
{
  return aci_queue_is_full(&aci_rx_q);
}

bool hal_aci_tl_tx_q_empty (void)
{
  return aci_queue_is_empty(&aci_tx_q);
}

bool hal_aci_tl_tx_q_full (void)
{
  return aci_queue_is_full(&aci_tx_q);
}

void hal_aci_tl_q_flush (void)
{
  m_aci_q_flush();
}

