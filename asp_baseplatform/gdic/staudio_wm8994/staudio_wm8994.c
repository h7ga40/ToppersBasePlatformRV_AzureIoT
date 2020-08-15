/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2018 by TOPPERS PROJECT
 *                             Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id$
 */
/**
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "staudio_wm8994.h"
#include <sil.h>
#include <target_syssvc.h>
#include <stdlib.h>

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/* Uncomment this line to enable verifying data sent to codec after each write 
   operation (for debug purpose) */
#if !defined (VERIFY_WRITTENDATA)  
/* #define VERIFY_WRITTENDATA */
#endif /* VERIFY_WRITTENDATA */

typedef struct
{
  uint16_t      *pRecBuf;       /* Pointer to record user buffer */
  uint32_t      RecSize;        /* Size to record in mono, double size to record in stereo */
}AUDIOIN_TypeDef;

#define AUDIO_FREQUENCY_192K          ((uint32_t)192000)
#define AUDIO_FREQUENCY_96K           ((uint32_t)96000)
#define AUDIO_FREQUENCY_48K           ((uint32_t)48000)
#define AUDIO_FREQUENCY_44K           ((uint32_t)44100)
#define AUDIO_FREQUENCY_32K           ((uint32_t)32000)
#define AUDIO_FREQUENCY_22K           ((uint32_t)22050)
#define AUDIO_FREQUENCY_16K           ((uint32_t)16000)
#define AUDIO_FREQUENCY_11K           ((uint32_t)11025)
#define AUDIO_FREQUENCY_8K            ((uint32_t)8000)  

/*### RECORD ###*/
#define DFSDM_OVER_SAMPLING(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? 256 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? 256 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? 128 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? 128 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? 64 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? 64  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? 40 : 20  \

#define DFSDM_CLOCK_DIVIDER(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? 24 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? 4 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? 24 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? 4 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? 24 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? 4  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? 25 : 25  \

#define DFSDM_FILTER_ORDER(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? DFSDM_FILTER_SINC4_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? DFSDM_FILTER_SINC3_ORDER  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? DFSDM_FILTER_SINC3_ORDER : DFSDM_FILTER_SINC5_ORDER  \

#define DFSDM_RIGHT_BIT_SHIFT(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? 8 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? 8 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? 3 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? 4 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? 7 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? 0  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? 0 : 4  \

/* Saturate the record PCM sample */
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))


#ifdef TOPPERS_STM32F769_DISCOVERY

#define GPIO_AF3_DFSDM1         ((uint8_t)0x03U)  /* DFSDM1 Alternate Function mapping */

#define AUDIO_DFSDMx_DMIC_DATIN_GPIO_CLK_ENABLE()       sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOCEN)
#define AUDIO_DFSDMx_CKOUT_DMIC_GPIO_CLK_ENABLE()       sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIODEN)

#define AUDIO_DFSDMx_TOP_RIGHT_CHANNEL                  DFSDM_CHANNEL_0
#define AUDIO_DFSDMx_TOP_LEFT_CHANNEL                   DFSDM_CHANNEL_1
#define AUDIO_DFSDMx_BUTTOM_RIGHT_CHANNEL               DFSDM_CHANNEL_4
#define AUDIO_DFSDMx_BUTTOM_LEFT_CHANNEL                DFSDM_CHANNEL_5

#define AUDIO_DFSDMx_TOP_LEFT_FILTER                    TADR_DFSDM1_FILTER0_BASE
#define AUDIO_DFSDMx_TOP_RIGHT_FILTER                   TADR_DFSDM1_FILTER1_BASE
#define AUDIO_DFSDMx_BUTTOM_LEFT_FILTER                 TADR_DFSDM1_FILTER2_BASE
#define AUDIO_DFSDMx_BUTTOM_RIGHT_FILTER                TADR_DFSDM1_FILTER3_BASE

#define AUDIO_DFSDMx_CKOUT_DMIC_GPIO_PORT               TADR_GPIOD_BASE
#define AUDIO_DFSDMx_CKOUT_PINNO                        3
#define AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT               TADR_GPIOC_BASE
#define AUDIO_DFSDMx_DMIC_DATIN1_PINNO                  3
#define AUDIO_DFSDMx_DMIC_DATIN5_PINNO                  11
#define AUDIO_DFSDMx_DMIC_DATIN_AF                      GPIO_AF3_DFSDM1
#define AUDIO_DFSDMx_CKOUT_AF                           GPIO_AF3_DFSDM1

#define AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_STREAM            TADR_DMA2_STM6_BASE
#define AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_STREAM           TADR_DMA2_STM7_BASE

#define AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE              DMA_PDATAALIGN_WORD
#define AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE                 DMA_MDATAALIGN_WORD

#define AUDIO_DFSDMx_DMAx_CHANNEL                       DMA_CHANNEL_8


/* RECORD */
static STAUDIO_Handle_t     *hstaudio;
AUDIOIN_TypeDef             hAudioIn;

DFSDM_Channel_Handle_t      hAudioInTopLeftChannel;
DFSDM_Channel_Handle_t      hAudioInTopRightChannel;
DFSDM_Filter_Handle_t       hAudioInTopLeftFilter;
DFSDM_Filter_Handle_t       hAudioInTopRightFilter;
DMA_Handle_t                hDmaTopLeft;
DMA_Handle_t                hDmaTopRight;

DFSDM_Channel_Handle_t      hAudioInButtomLeftChannel;
DFSDM_Channel_Handle_t      hAudioInButtomRightChannel;
DFSDM_Filter_Handle_t       hAudioInButtomLeftFilter;
DFSDM_Filter_Handle_t       hAudioInButtomRightFilter;
DMA_Handle_t                hDmaButtomLeft;
DMA_Handle_t                hDmaButtomRight;

/* Buffers for right and left samples */
static int32_t              *pScratchBuff[2*DEFAULT_AUDIO_IN_CHANNEL_NBR];
static volatile int32_t     ScratchSize;
/* Cannel number to be used: 2 channels by default */
static uint8_t              AudioIn_ChannelNumber = DEFAULT_AUDIO_IN_CHANNEL_NBR;

/* Buffers status flags */
static uint32_t             DmaTopLeftRecHalfCplt  = 0;
static uint32_t             DmaTopLeftRecCplt      = 0;
static uint32_t             DmaTopRightRecHalfCplt = 0;
static uint32_t             DmaTopRightRecCplt     = 0;
static uint32_t             DmaButtomLeftRecHalfCplt  = 0;
static uint32_t             DmaButtomLeftRecCplt      = 0;
static uint32_t             DmaButtomRightRecHalfCplt = 0;
static uint32_t             DmaButtomRightRecCplt     = 0;

/* Application Buffer Trigger */
static volatile uint32_t    AppBuffTrigger          = 0;
static volatile uint32_t    AppBuffHalf             = 0;
#endif


/** @defgroup WM8994_Function_Prototypes
  * @{
  */
static uint32_t wm8994_SetVolume(STAUDIO_Handle_t *haudio, uint8_t Volume);
static uint32_t wm8994_SetMute(STAUDIO_Handle_t *haudio, uint32_t Cmd);


/**
  * @brief  Writes/Read a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  * @retval None
  */
static uint8_t
CODEC_IO_Write(STAUDIO_Handle_t *haudio, uint16_t Reg, uint16_t Value)
{
	ER ercd;
	uint8_t  buffer[4];
	uint32_t result = 0;

	buffer[0] = (uint8_t)(Value >> 8);
	buffer[1] = (uint8_t)Value;
	ercd = i2c_memwrite(haudio->hi2c, haudio->i2caddress, (uint16_t)Reg, I2C_MEMADD_SIZE_16BIT, buffer, 2, 1000);

	/* Check the communication status */
	if(ercd != E_OK){
#if 1	/* ROI DEBUG */
		syslog_2(LOG_NOTICE, "## I2Cx_Error(%08x, %02x) ##", haudio->hi2c, haudio->i2caddress);
#endif	/* ROI DEBUG */
		/* De-initialize the I2C communication bus */
		i2c_deinit(haudio->hi2c);

		/* Re-Initialize the I2C communication bus */
		i2c_init(haudio->hi2c->i2cid, &haudio->hi2c->Init);
	}

#ifdef VERIFY_WRITTENDATA
	{
		uint16_t  value;
		ercd = i2c_memread(haudio->hi2c, haudio->i2caddress, (uint16_t)Reg, I2C_MEMADD_SIZE_16BIT, buffer, 2, 1000);
		value = (buffer[0] << 8) | buffer[1];
		/* Verify that the data has been correctly written */
		result = (value == Value)? 0:1;
	}
#endif /* VERIFY_WRITTENDATA */
	return result;
}

/**
  * @brief Initializes the audio codec and the control interface.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param OutputInputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *  OUTPUT_DEVICE_BOTH, OUTPUT_DEVICE_AUTO, INPUT_DEVICE_DIGITAL_MICROPHONE_1,
  *  INPUT_DEVICE_DIGITAL_MICROPHONE_2, INPUT_DEVICE_DIGITAL_MIC1_MIC2, 
  *  INPUT_DEVICE_INPUT_LINE_1 or INPUT_DEVICE_INPUT_LINE_2.
  * @param Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param AudioFreq: Audio Frequency 
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t
wm8994_Init(STAUDIO_Handle_t *haudio, uint16_t OutputInputDevice, uint8_t Volume, uint32_t AudioFreq)
{
	uint32_t counter = 0;
	uint16_t output_device = OutputInputDevice & 0xFF;
	uint16_t input_device = OutputInputDevice & 0xFF00;
	uint16_t power_mgnt_reg_1 = 0;

	/* wm8994 Errata Work-Arounds */
	counter += CODEC_IO_Write(haudio, 0x102, 0x0003);
	counter += CODEC_IO_Write(haudio, 0x817, 0x0000);
	counter += CODEC_IO_Write(haudio, 0x102, 0x0000);

	/* Enable VMID soft start (fast), Start-up Bias Current Enabled */
	counter += CODEC_IO_Write(haudio, 0x39, 0x006C);

	/* Enable bias generator, Enable VMID */
	if(input_device > 0){
		counter += CODEC_IO_Write(haudio, 0x01, 0x0013);
	}
	else{
		counter += CODEC_IO_Write(haudio, 0x01, 0x0003);
	}

	/* Add Delay */
	dly_tsk(50);

	/* Path Configurations for output */
	if(output_device > 0){
		haudio->outputEnabled = 1;

		switch(output_device){
		case OUTPUT_DEVICE_SPEAKER:
			/* Enable DAC1 (Left), Enable DAC1 (Right),
			   Disable DAC2 (Left), Disable DAC2 (Right)*/
			counter += CODEC_IO_Write(haudio, 0x05, 0x0C0C);

			/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x601, 0x0000);

			/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x602, 0x0000);

			/* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x604, 0x0002);

			/* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x605, 0x0002);
			break;

		case OUTPUT_DEVICE_HEADPHONE:
			/* Disable DAC1 (Left), Disable DAC1 (Right),
			   Enable DAC2 (Left), Enable DAC2 (Right)*/
			counter += CODEC_IO_Write(haudio, 0x05, 0x0303);

			/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x601, 0x0001);

			/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x602, 0x0001);

			/* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x604, 0x0000);

			/* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x605, 0x0000);
			break;

		case OUTPUT_DEVICE_BOTH:
			if(input_device == INPUT_DEVICE_DIGITAL_MIC1_MIC2){
				/* Enable DAC1 (Left), Enable DAC1 (Right),
				   also Enable DAC2 (Left), Enable DAC2 (Right)*/
				counter += CODEC_IO_Write(haudio, 0x05, 0x0303 | 0x0C0C);

				/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path
				  Enable the AIF1 Timeslot 1 (Left) to DAC 1 (Left) mixer path */
				counter += CODEC_IO_Write(haudio, 0x601, 0x0003);

				/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path
				   Enable the AIF1 Timeslot 1 (Right) to DAC 1 (Right) mixer path */
				counter += CODEC_IO_Write(haudio, 0x602, 0x0003);

				/* Enable the AIF1 Timeslot 0 (Left) to DAC 2 (Left) mixer path
				   Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path  */
				counter += CODEC_IO_Write(haudio, 0x604, 0x0003);

				/* Enable the AIF1 Timeslot 0 (Right) to DAC 2 (Right) mixer path
				   Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
				counter += CODEC_IO_Write(haudio, 0x605, 0x0003);
			}
			else{
				/* Enable DAC1 (Left), Enable DAC1 (Right),
				   also Enable DAC2 (Left), Enable DAC2 (Right)*/
				counter += CODEC_IO_Write(haudio, 0x05, 0x0303 | 0x0C0C);

				/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
				counter += CODEC_IO_Write(haudio, 0x601, 0x0001);

				/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
				counter += CODEC_IO_Write(haudio, 0x602, 0x0001);

				/* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
				counter += CODEC_IO_Write(haudio, 0x604, 0x0002);

				/* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
				counter += CODEC_IO_Write(haudio, 0x605, 0x0002);      
			}
			break;

		case OUTPUT_DEVICE_AUTO :
		default:
			/* Disable DAC1 (Left), Disable DAC1 (Right),
			   Enable DAC2 (Left), Enable DAC2 (Right)*/
			counter += CODEC_IO_Write(haudio, 0x05, 0x0303);

			/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x601, 0x0001);

			/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x602, 0x0001);

			/* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x604, 0x0000);

			/* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x605, 0x0000);
			break;
		}
	}
	else{
		haudio->outputEnabled = 0;
	}

	/* Path Configurations for input */
	if(input_device > 0){
		haudio->inputEnabled = 1;
		switch(input_device){
		case INPUT_DEVICE_DIGITAL_MICROPHONE_2 :
			/* Enable AIF1ADC2 (Left), Enable AIF1ADC2 (Right)
			 * Enable DMICDAT2 (Left), Enable DMICDAT2 (Right)
			 * Enable Left ADC, Enable Right ADC */
			counter += CODEC_IO_Write(haudio, 0x04, 0x0C30);

			/* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC2 Left/Right Timeslot 1 */
			counter += CODEC_IO_Write(haudio, 0x450, 0x00DB);

			/* Disable IN1L, IN1R, IN2L, IN2R, Enable Thermal sensor & shutdown */
			counter += CODEC_IO_Write(haudio, 0x02, 0x6000);

			/* Enable the DMIC2(Left) to AIF1 Timeslot 1 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x608, 0x0002);

			/* Enable the DMIC2(Right) to AIF1 Timeslot 1 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x609, 0x0002);

			/* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC2 signal detect */
			counter += CODEC_IO_Write(haudio, 0x700, 0x000E);
			break;

		case INPUT_DEVICE_INPUT_LINE_1 :
			/* IN1LN_TO_IN1L, IN1LP_TO_VMID, IN1RN_TO_IN1R, IN1RP_TO_VMID */
			counter += CODEC_IO_Write(haudio, 0x28, 0x0011);

			/* Disable mute on IN1L_TO_MIXINL and +30dB on IN1L PGA output */
			counter += CODEC_IO_Write(haudio, 0x29, 0x0035);

			/* Disable mute on IN1R_TO_MIXINL, Gain = +30dB */
			counter += CODEC_IO_Write(haudio, 0x2A, 0x0035);

			/* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
			 * Enable Left ADC, Enable Right ADC */
			counter += CODEC_IO_Write(haudio, 0x04, 0x0303);

			/* Enable AIF1 DRC1 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
			counter += CODEC_IO_Write(haudio, 0x440, 0x00DB);

			/* Enable IN1L and IN1R, Disable IN2L and IN2R, Enable Thermal sensor & shutdown */
			counter += CODEC_IO_Write(haudio, 0x02, 0x6350);

			/* Enable the ADCL(Left) to AIF1 Timeslot 0 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x606, 0x0002);

			/* Enable the ADCR(Right) to AIF1 Timeslot 0 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x607, 0x0002);

			/* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
			counter += CODEC_IO_Write(haudio, 0x700, 0x000D);
			break;

		case INPUT_DEVICE_DIGITAL_MICROPHONE_1 :
			/* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
			 * Enable DMICDAT1 (Left), Enable DMICDAT1 (Right)
			 * Enable Left ADC, Enable Right ADC */
			counter += CODEC_IO_Write(haudio, 0x04, 0x030C);

			/* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
			counter += CODEC_IO_Write(haudio, 0x440, 0x00DB);

			/* Disable IN1L, IN1R, IN2L, IN2R, Enable Thermal sensor & shutdown */
			counter += CODEC_IO_Write(haudio, 0x02, 0x6350);

			/* Enable the DMIC2(Left) to AIF1 Timeslot 0 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x606, 0x0002);

			/* Enable the DMIC2(Right) to AIF1 Timeslot 0 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x607, 0x0002);

			/* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
			counter += CODEC_IO_Write(haudio, 0x700, 0x000D);
			break;
		case INPUT_DEVICE_DIGITAL_MIC1_MIC2 :
			/* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
			 * Enable DMICDAT1 (Left), Enable DMICDAT1 (Right)
			 * Enable Left ADC, Enable Right ADC */
			counter += CODEC_IO_Write(haudio, 0x04, 0x0F3C);

			/* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC2 Left/Right Timeslot 1 */
			counter += CODEC_IO_Write(haudio, 0x450, 0x00DB);

			/* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
			counter += CODEC_IO_Write(haudio, 0x440, 0x00DB);

			/* Disable IN1L, IN1R, Enable IN2L, IN2R, Thermal sensor & shutdown */
			counter += CODEC_IO_Write(haudio, 0x02, 0x63A0);

			/* Enable the DMIC2(Left) to AIF1 Timeslot 0 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x606, 0x0002);

			/* Enable the DMIC2(Right) to AIF1 Timeslot 0 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x607, 0x0002);

			/* Enable the DMIC2(Left) to AIF1 Timeslot 1 (Left) mixer path */
			counter += CODEC_IO_Write(haudio, 0x608, 0x0002);

			/* Enable the DMIC2(Right) to AIF1 Timeslot 1 (Right) mixer path */
			counter += CODEC_IO_Write(haudio, 0x609, 0x0002);

			/* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
			counter += CODEC_IO_Write(haudio, 0x700, 0x000D);
			break;
		case INPUT_DEVICE_INPUT_LINE_2 :
		default:
			/* Actually, no other input devices supported */
			counter++;
			break;
		}
	}
	else{
		haudio->inputEnabled = 0;
	}

	/*  Clock Configurations */
	switch (AudioFreq){
	case  AUDIO_FREQUENCY_8K:
		/* AIF1 Sample Rate = 8 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0003);
		break;

	case  AUDIO_FREQUENCY_16K:
		/* AIF1 Sample Rate = 16 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0033);
		break;

	case  AUDIO_FREQUENCY_32K:
		/* AIF1 Sample Rate = 32 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0063);
		break;

	case  AUDIO_FREQUENCY_48K:
		/* AIF1 Sample Rate = 48 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0083);
		break;

	case  AUDIO_FREQUENCY_96K:
		/* AIF1 Sample Rate = 96 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x00A3);
		break;

	case  AUDIO_FREQUENCY_11K:
		/* AIF1 Sample Rate = 11.025 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0013);
		break;

	case  AUDIO_FREQUENCY_22K:
		/* AIF1 Sample Rate = 22.050 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0043);
		break;

	case  AUDIO_FREQUENCY_44K:
		/* AIF1 Sample Rate = 44.1 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0073);
		break;

	default:
		/* AIF1 Sample Rate = 48 (KHz), ratio=256 */ 
		counter += CODEC_IO_Write(haudio, 0x210, 0x0083);
		break;
	}

	if(input_device == INPUT_DEVICE_DIGITAL_MIC1_MIC2){
		/* AIF1 Word Length = 16-bits, AIF1 Format = DSP mode */
		counter += CODEC_IO_Write(haudio, 0x300, 0x4018);    
	}
	else{
		/* AIF1 Word Length = 16-bits, AIF1 Format = I2S (Default Register Value) */
		counter += CODEC_IO_Write(haudio, 0x300, 0x4010);
	}

	/* slave mode */
	counter += CODEC_IO_Write(haudio, 0x302, 0x0000);

	/* Enable the DSP processing clock for AIF1, Enable the core clock */
	counter += CODEC_IO_Write(haudio, 0x208, 0x000A);

	/* Enable AIF1 Clock, AIF1 Clock Source = MCLK1 pin */
	counter += CODEC_IO_Write(haudio, 0x200, 0x0001);

	if(output_device > 0){	/* Audio output selected */
		if(output_device == OUTPUT_DEVICE_HEADPHONE){
			/* Select DAC1 (Left) to Left Headphone Output PGA (HPOUT1LVOL) path */
			counter += CODEC_IO_Write(haudio, 0x2D, 0x0100);

			/* Select DAC1 (Right) to Right Headphone Output PGA (HPOUT1RVOL) path */
			counter += CODEC_IO_Write(haudio, 0x2E, 0x0100);    

			/* Startup sequence for Headphone */
			if(haudio->ColdStartup){
				counter += CODEC_IO_Write(haudio,0x110,0x8100);

				haudio->ColdStartup = 0;
				/* Add Delay */
				dly_tsk(300);
			}
			else{	/* Headphone Warm Start-Up */
				counter += CODEC_IO_Write(haudio,0x110,0x8108);
				/* Add Delay */
				dly_tsk(50);
			}

			/* Soft un-Mute the AIF1 Timeslot 0 DAC1 path L&R */
			counter += CODEC_IO_Write(haudio, 0x420, 0x0000);
		}
		/* Analog Output Configuration */

		/* Enable SPKRVOL PGA, Enable SPKMIXR, Enable SPKLVOL PGA, Enable SPKMIXL */
		counter += CODEC_IO_Write(haudio, 0x03, 0x0300);

		/* Left Speaker Mixer Volume = 0dB */
		counter += CODEC_IO_Write(haudio, 0x22, 0x0000);

		/* Speaker output mode = Class D, Right Speaker Mixer Volume = 0dB ((0x23, 0x0100) = class AB)*/
		counter += CODEC_IO_Write(haudio, 0x23, 0x0000);

		/* Unmute DAC2 (Left) to Left Speaker Mixer (SPKMIXL) path,
		   Unmute DAC2 (Right) to Right Speaker Mixer (SPKMIXR) path */
		counter += CODEC_IO_Write(haudio, 0x36, 0x0300);

		/* Enable bias generator, Enable VMID, Enable SPKOUTL, Enable SPKOUTR */
		counter += CODEC_IO_Write(haudio, 0x01, 0x3003);

		/* Headphone/Speaker Enable */
		if(input_device == INPUT_DEVICE_DIGITAL_MIC1_MIC2){
			/* Enable Class W, Class W Envelope Tracking = AIF1 Timeslots 0 and 1 */
			counter += CODEC_IO_Write(haudio, 0x51, 0x0205);
		}
		else{
			/* Enable Class W, Class W Envelope Tracking = AIF1 Timeslot 0 */
			counter += CODEC_IO_Write(haudio, 0x51, 0x0005);      
		}

		/* Enable bias generator, Enable VMID, Enable HPOUT1 (Left) and Enable HPOUT1 (Right) input stages */
		/* idem for Speaker */
		power_mgnt_reg_1 |= 0x0303 | 0x3003;
		counter += CODEC_IO_Write(haudio, 0x01, power_mgnt_reg_1);

		/* Enable HPOUT1 (Left) and HPOUT1 (Right) intermediate stages */
		counter += CODEC_IO_Write(haudio, 0x60, 0x0022);

		/* Enable Charge Pump */
		counter += CODEC_IO_Write(haudio, 0x4C, 0x9F25);

		/* Add Delay */
		dly_tsk(15);

		/* Select DAC1 (Left) to Left Headphone Output PGA (HPOUT1LVOL) path */
		counter += CODEC_IO_Write(haudio, 0x2D, 0x0001);

		/* Select DAC1 (Right) to Right Headphone Output PGA (HPOUT1RVOL) path */
		counter += CODEC_IO_Write(haudio, 0x2E, 0x0001);

		/* Enable Left Output Mixer (MIXOUTL), Enable Right Output Mixer (MIXOUTR) */
		/* idem for SPKOUTL and SPKOUTR */
		counter += CODEC_IO_Write(haudio, 0x03, 0x0030 | 0x0300);

		/* Enable DC Servo and trigger start-up mode on left and right channels */
		counter += CODEC_IO_Write(haudio, 0x54, 0x0033);

		/* Add Delay */
		dly_tsk(257);

		/* Enable HPOUT1 (Left) and HPOUT1 (Right) intermediate and output stages. Remove clamps */
		counter += CODEC_IO_Write(haudio, 0x60, 0x00EE);

		/* Unmutes */

		/* Unmute DAC 1 (Left) */
		counter += CODEC_IO_Write(haudio, 0x610, 0x00C0);

		/* Unmute DAC 1 (Right) */
		counter += CODEC_IO_Write(haudio, 0x611, 0x00C0);

		/* Unmute the AIF1 Timeslot 0 DAC path */
		counter += CODEC_IO_Write(haudio, 0x420, 0x0010);

		/* Unmute DAC 2 (Left) */
		counter += CODEC_IO_Write(haudio, 0x612, 0x00C0);

		/* Unmute DAC 2 (Right) */
		counter += CODEC_IO_Write(haudio, 0x613, 0x00C0);

		/* Unmute the AIF1 Timeslot 1 DAC2 path */
		counter += CODEC_IO_Write(haudio, 0x422, 0x0010);

		/* Volume Control */
		wm8994_SetVolume(haudio, Volume);
	}

	if(input_device > 0){	/* Audio input selected */
		if((input_device == INPUT_DEVICE_DIGITAL_MICROPHONE_1) || (input_device == INPUT_DEVICE_DIGITAL_MICROPHONE_2)){
			/* Enable Microphone bias 1 generator, Enable VMID */
			power_mgnt_reg_1 |= 0x0013;
			counter += CODEC_IO_Write(haudio, 0x01, power_mgnt_reg_1);

			/* ADC oversample enable */
			counter += CODEC_IO_Write(haudio, 0x620, 0x0002);

			/* AIF ADC2 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
			counter += CODEC_IO_Write(haudio, 0x411, 0x3800);
		}
		else if(input_device == INPUT_DEVICE_DIGITAL_MIC1_MIC2){
			/* Enable Microphone bias 1 generator, Enable VMID */
			power_mgnt_reg_1 |= 0x0013;
			counter += CODEC_IO_Write(haudio, 0x01, power_mgnt_reg_1);

			/* ADC oversample enable */
			counter += CODEC_IO_Write(haudio, 0x620, 0x0002);

			/* AIF ADC1 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
			counter += CODEC_IO_Write(haudio, 0x410, 0x1800);

			/* AIF ADC2 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
			counter += CODEC_IO_Write(haudio, 0x411, 0x1800);      
		}
		else if((input_device == INPUT_DEVICE_INPUT_LINE_1) || (input_device == INPUT_DEVICE_INPUT_LINE_2)){
			/* Disable mute on IN1L, IN1L Volume = +0dB */
			counter += CODEC_IO_Write(haudio, 0x18, 0x000B);

			/* Disable mute on IN1R, IN1R Volume = +0dB */
			counter += CODEC_IO_Write(haudio, 0x1A, 0x000B);

			/* AIF ADC1 HPF enable, HPF cut = hifi mode fc=4Hz at fs=48kHz */
			counter += CODEC_IO_Write(haudio, 0x410, 0x1800);
		}
		/* Volume Control */
		wm8994_SetVolume(haudio, Volume);
	}
	/* Return communication control value */
	return counter;
}

/**
  * @brief Pauses playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t
wm8994_Pause(STAUDIO_Handle_t *haudio)
{
	uint32_t counter = 0;

	/* Pause the audio file playing */
	/* Mute the output first */
	counter += wm8994_SetMute(haudio, AUDIO_MUTE_ON);

	/* Put the Codec in Power save mode */
	counter += CODEC_IO_Write(haudio, 0x02, 0x01);
	return counter;
}

/**
  * @brief Stops audio Codec playing. It powers down the codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @param CodecPdwnMode: selects the  power down mode.
  *          - CODEC_PDWN_SW: only mutes the audio codec. When resuming from this 
  *                           mode the codec keeps the previous initialization
  *                           (no need to re-Initialize the codec registers).
  *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration 
  *                           (user should re-Initialize the codec in order to 
  *                            play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t
wm8994_Stop(STAUDIO_Handle_t *haudio, uint32_t CodecPdwnMode)
{
	uint32_t counter = 0;

	if(haudio->outputEnabled != 0){
		/* Mute the output first */
		counter += wm8994_SetMute(haudio, AUDIO_MUTE_ON);

		if(CodecPdwnMode == CODEC_PDWN_SW){
			/* Only output mute required*/
		}
		else{	/* CODEC_PDWN_HW */
			/* Mute the AIF1 Timeslot 0 DAC1 path */
			counter += CODEC_IO_Write(haudio, 0x420, 0x0200);

			/* Mute the AIF1 Timeslot 1 DAC2 path */
			counter += CODEC_IO_Write(haudio, 0x422, 0x0200);

			/* Disable DAC1L_TO_HPOUT1L */
			counter += CODEC_IO_Write(haudio, 0x2D, 0x0000);

			/* Disable DAC1R_TO_HPOUT1R */
			counter += CODEC_IO_Write(haudio, 0x2E, 0x0000);

			/* Disable DAC1 and DAC2 */
			counter += CODEC_IO_Write(haudio, 0x05, 0x0000);

			/* Reset Codec by writing in 0x0000 address register */
			counter += CODEC_IO_Write(haudio, 0x0000, 0x0000);
			haudio->outputEnabled = 0;
		}
	}
	return counter;
}

/**
  * @brief Sets higher or lower the codec volume level.
  * @param DeviceAddr: Device address on communication Bus.
  * @param Volume: a byte value from 0 to 255 (refer to codec registers 
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t
wm8994_SetVolume(STAUDIO_Handle_t *haudio, uint8_t Volume)
{
	uint32_t counter = 0;
	uint8_t convertedvol = VOLUME_CONVERT(Volume);

	/* Output volume */
	if(haudio->outputEnabled != 0){
		if(convertedvol > 0x3E){
			/* Unmute audio codec */
			counter += wm8994_SetMute(haudio, AUDIO_MUTE_OFF);

			/* Left Headphone Volume */
			counter += CODEC_IO_Write(haudio, 0x1C, 0x3F | 0x140);

			/* Right Headphone Volume */
			counter += CODEC_IO_Write(haudio, 0x1D, 0x3F | 0x140);

			/* Left Speaker Volume */
			counter += CODEC_IO_Write(haudio, 0x26, 0x3F | 0x140);

			/* Right Speaker Volume */
			counter += CODEC_IO_Write(haudio, 0x27, 0x3F | 0x140);
		}
		else if(Volume == 0){
			/* Mute audio codec */
			counter += wm8994_SetMute(haudio, AUDIO_MUTE_ON);
		}
		else{
			/* Unmute audio codec */
			counter += wm8994_SetMute(haudio, AUDIO_MUTE_OFF);

			/* Left Headphone Volume */
			counter += CODEC_IO_Write(haudio, 0x1C, convertedvol | 0x140);

			/* Right Headphone Volume */
			counter += CODEC_IO_Write(haudio, 0x1D, convertedvol | 0x140);

			/* Left Speaker Volume */
			counter += CODEC_IO_Write(haudio, 0x26, convertedvol | 0x140);

			/* Right Speaker Volume */
			counter += CODEC_IO_Write(haudio, 0x27, convertedvol | 0x140);
		}
	}

	/* Input volume */
	if(haudio->inputEnabled != 0){
		convertedvol = VOLUME_IN_CONVERT(Volume);

		/* Left AIF1 ADC1 volume */
		counter += CODEC_IO_Write(haudio, 0x400, convertedvol | 0x100);

		/* Right AIF1 ADC1 volume */
		counter += CODEC_IO_Write(haudio, 0x401, convertedvol | 0x100);

		/* Left AIF1 ADC2 volume */
		counter += CODEC_IO_Write(haudio, 0x404, convertedvol | 0x100);

		/* Right AIF1 ADC2 volume */
		counter += CODEC_IO_Write(haudio, 0x405, convertedvol | 0x100);
	}
	return counter;
}

/**
  * @brief Enables or disables the mute feature on the audio codec.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *             mute mode.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t
wm8994_SetMute(STAUDIO_Handle_t *haudio, uint32_t Cmd)
{
	uint32_t counter = 0;

	if(haudio->outputEnabled != 0){
		/* Set the Mute mode */
		if(Cmd == AUDIO_MUTE_ON){
			/* Soft Mute the AIF1 Timeslot 0 DAC1 path L&R */
			counter += CODEC_IO_Write(haudio, 0x420, 0x0200);

			/* Soft Mute the AIF1 Timeslot 1 DAC2 path L&R */
			counter += CODEC_IO_Write(haudio, 0x422, 0x0200);
		}
		else{	/* AUDIO_MUTE_OFF Disable the Mute */
			/* Unmute the AIF1 Timeslot 0 DAC1 path L&R */
			counter += CODEC_IO_Write(haudio, 0x420, 0x0010);

			/* Unmute the AIF1 Timeslot 1 DAC2 path L&R */
			counter += CODEC_IO_Write(haudio, 0x422, 0x0010);
		}
	}
	return counter;
}

/**
  * @brief Switch dynamically (while audio file is played) the output target 
  *         (speaker or headphone).
  * @param DeviceAddr: Device address on communication Bus.
  * @param Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO 
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t
wm8994_SetOutputMode(STAUDIO_Handle_t *haudio, uint8_t Output)
{
	uint32_t counter = 0;

	switch(Output){
	case OUTPUT_DEVICE_SPEAKER:
		/* Enable DAC1 (Left), Enable DAC1 (Right), 
		  Disable DAC2 (Left), Disable DAC2 (Right)*/
		counter += CODEC_IO_Write(haudio, 0x05, 0x0C0C);

		/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x601, 0x0000);

		/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x602, 0x0000);

		/* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x604, 0x0002);

		/* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x605, 0x0002);
		break;

	case OUTPUT_DEVICE_HEADPHONE:
		/* Disable DAC1 (Left), Disable DAC1 (Right), 
		  Enable DAC2 (Left), Enable DAC2 (Right)*/
		counter += CODEC_IO_Write(haudio, 0x05, 0x0303);

		/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x601, 0x0001);

		/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x602, 0x0001);

		/* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x604, 0x0000);

		/* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x605, 0x0000);
		break;

	case OUTPUT_DEVICE_BOTH:
		/* Enable DAC1 (Left), Enable DAC1 (Right), 
		 also Enable DAC2 (Left), Enable DAC2 (Right)*/
		counter += CODEC_IO_Write(haudio, 0x05, 0x0303 | 0x0C0C);

		/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x601, 0x0001);

		/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x602, 0x0001);

		/* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x604, 0x0002);

		/* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x605, 0x0002);
		break;

	default:
		/* Disable DAC1 (Left), Disable DAC1 (Right), 
		  Enable DAC2 (Left), Enable DAC2 (Right)*/
		counter += CODEC_IO_Write(haudio, 0x05, 0x0303);

		/* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x601, 0x0001);

		/* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x602, 0x0001);

		/* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
		counter += CODEC_IO_Write(haudio, 0x604, 0x0000);

		/* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
		counter += CODEC_IO_Write(haudio, 0x605, 0x0000);
		break;
	}
	return counter;
}

/**
  * @brief Resets wm8994 registers.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
static ER
wm8994_Reset(STAUDIO_Handle_t *haudio)
{
	uint32_t deviceid = 0;
	ER ercd;
	uint8_t  buffer[4];

	/* Get the WM8994 ID. */
	ercd = i2c_memread(haudio->hi2c, haudio->i2caddress, (uint16_t)WM8994_CHIPID_ADDR, I2C_MEMADD_SIZE_16BIT, buffer, 2, 1000);
	if(ercd != E_OK)
		return ercd;
	deviceid = (buffer[0] << 8) | buffer[1];

	/* wm8994 codec initialization */
#if 1	/* ROI DEBUG */
	syslog_2(LOG_NOTICE, "## wm8994_Reset deviceid[%08x][%08x] ##", deviceid, WM8994_ID);
#endif	/* ROI DEBUG */

	if(deviceid != WM8994_ID)
		return E_SYS;

	/* Reset Codec by writing in 0x0000 address register */
	CODEC_IO_Write(haudio, 0x0000, 0x0000);
	haudio->outputEnabled = 0;
	haudio->inputEnabled  = 0;
	haudio->ColdStartup   = 1;
	return E_OK;
}


static void SAIx_Out_Init(STAUDIO_Handle_t *haudio, uint32_t AudioFreq, uint16_t framesize)
{
	AUDIO_Handle_t *hsai = haudio->hsai;

	/* Configure SAI_Block_x 
	  LSBFirst: Disabled 
	  DataSize: 16 */
	hsai->OutInit.MonoStereoMode = SAI_STEREOMODE;
	hsai->OutInit.AudioFrequency = AudioFreq;
	hsai->OutInit.AudioMode = SAI_MODEMASTER_TX;
	hsai->OutInit.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	hsai->OutInit.Protocol = SAI_FREE_PROTOCOL;
	hsai->OutInit.DataSize = SAI_DATASIZE_16;
	hsai->OutInit.FirstBit = SAI_FIRSTBIT_MSB;
	hsai->OutInit.Synchro = SAI_ASYNCHRONOUS;
#if defined(TOPPERS_STM32F769_DISCOVERY) || defined(TOPPERS_STM32F7_DISCOVERY)
	hsai->OutInit.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
	hsai->OutInit.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
#else
	hsai->OutInit.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
	hsai->OutInit.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
#endif
	hsai->OutInit.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
	hsai->OutInit.SynchroExt    = SAI_SYNCEXT_DISABLE;
	hsai->OutInit.CompandingMode= SAI_NOCOMPANDING;
	hsai->OutInit.TriState      = SAI_OUTPUT_NOTRELEASED;
	hsai->OutInit.Mckdiv        = 0;

	/* Configure SAI_Block_x Frame 
	  Frame Length: 64
	  Frame active Length: 32
	  FS Definition: Start frame + Channel Side identification
	  FS Polarity: FS active Low
	  FS Offset: FS asserted one bit before the first bit of slot 0 */ 
	hsai->OutInit.FrameLength = framesize;
	hsai->OutInit.ActiveFrameLength = framesize / 2;
	hsai->OutInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	hsai->OutInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	hsai->OutInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

	/* Configure SAI Block_x Slot 
	  Slot First Bit Offset: 0
	  Slot Size  : 16
	  Slot Number: 4
	  Slot Active: All slot actives */
	hsai->OutInit.FirstBitOffset = 0;
	hsai->OutInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	hsai->OutInit.SlotNumber = 4; 
	hsai->OutInit.SlotActive = CODEC_AUDIOFRAME_SLOT_0123;
}


/*
 *  オーディオ出力初期化設定
 *  param1: STAUDIOハンドラ
 *  param2: 出力デバイス設定
 *  param3: ボリューム設定値
 *  param4: オーディオ周波数値
 *  param5: フレームサイズ(バイト)
 *  return: E_OKで正常設定
 */
ER
staudio_out_init(STAUDIO_Handle_t *haudio, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq, uint16_t framesize)
{
	AUDIO_Handle_t *hsai;
	ER   ercd = E_OK;

	/* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */ 
	if(haudio->hsai == NULL){
		haudio->hsai = audio_init(haudio->saiid);
	}
	else{
		/* Disable SAI peripheral */
		audio_disable(haudio->hsai, AUDIO_OUT_BLOCK);
		audio_end(haudio->hsai, AUDIO_OUT_BLOCK);
	}
	hsai = haudio->hsai;
	audio_clockconfig(hsai, AudioFreq, NULL);

	/* SAI data transfer preparation:
	  Prepare the Media to be used for the audio transfer from memory to SAI peripheral */
	/* Disable SAI peripheral to allow access to SAI internal registers */
	audio_disable(hsai, AUDIO_OUT_BLOCK);

	SAIx_Out_Init(haudio, AudioFreq, framesize);

	audio_start(hsai, AUDIO_OUT_BLOCK);

	/* Enable SAI peripheral to generate MCLK */
	audio_enable(hsai, AUDIO_OUT_BLOCK);

	/* wm8994 codec initialization */
	/* Reset the Codec Registers */
	ercd = wm8994_Reset(haudio);
	if(ercd == E_OK){
		/* Initialize the codec internal registers */
		wm8994_Init(haudio, OutputDevice, Volume, AudioFreq);
	}
	return ercd;
}

/*
 *  オーディオ出力演奏
 *  param1: STAUDIOハンドラ
 *  param2: 出力バッファポインタ
 *  param3: 出力バッファサイズ
 *  return: E_OKで正常設定
 */
ER
staudio_out_play(STAUDIO_Handle_t *haudio, uint16_t* pBuffer, uint32_t Size)
{
	AUDIO_Handle_t *hsai = haudio->hsai;

	/* Call the audio Codec Play function */
	if(wm8994_SetMute(haudio, AUDIO_MUTE_OFF) != 0){
		return E_SYS;
	}
	else{
		/* Update the Media layer and enable it for play */  
		hsai->transcallback = haudio->audio_fullout_func;
		hsai->transhalfcallback = haudio->audio_halfout_func;
		hsai->errorcallback = haudio->audio_error_func;
		audio_transmit(hsai, (uint8_t*) pBuffer, DMA_MAX(Size / AUDIODATA_SIZE));
		return E_OK;
	}
}

/*
 *  オーディオPAUSE設定
 *  param1: STAUDIOハンドラ
 *  return: E_OKで正常設定
 */
ER
staudio_out_pause(STAUDIO_Handle_t *haudio)
{
	/* Call the Audio Codec Pause/Resume function */
	if(wm8994_Pause(haudio) != 0){
		return E_SYS;
	}
	else{
		/* Call the Media layer pause function */
		audio_dmapause(haudio->hsai, AUDIO_OUT_BLOCK);
		return E_OK;
	}
}

/*
 *  オーディオRESUME設定
 *  param1: STAUDIOハンドラ
 *  return: E_OKで正常設定
 */
ER
staudio_out_resume(STAUDIO_Handle_t *haudio)
{
	/* Call the Audio Codec Pause/Resume function */
	if(wm8994_SetMute(haudio, AUDIO_MUTE_OFF) != 0){
		return E_SYS;
	}
	else{
		/* Call the Media layer pause/resume function */
		audio_dmaresume(haudio->hsai, AUDIO_OUT_BLOCK);
		return E_OK;
	}
}

/*
 *  オーディオ出力停止設定
 *  param1: STAUDIOハンドラ
 *  param2: 停止設定条件
 *  return: E_OKで正常設定
 */
ER
staudio_out_stop(STAUDIO_Handle_t *haudio, uint32_t Option)
{
	/* Call the Media layer stop function */
	audio_dmastop(haudio->hsai, AUDIO_OUT_BLOCK);

	/* Call Audio Codec Stop function */
	if(wm8994_Stop(haudio, Option) != 0){
		return E_SYS;
	}
	else{
		if(Option == CODEC_PDWN_HW){
			/* Wait at least 100us */
			dly_tsk(1);
		}
		return E_OK;
	}
}

/*
 *  オーディオボリューム設定
 *  param1: STAUDIOハンドラ
 *  param2: volumeパセンテージ(0%-100%)
 *  return: E_OKで正常設定
 */
ER
staudio_out_setvolume(STAUDIO_Handle_t *haudio, uint8_t Volume)
{
	/* Call the codec volume control function with converted volume value */
	if(wm8994_SetVolume(haudio, Volume) != 0)
		return E_SYS;
	else
		return E_OK;
}

/*
 *  オーディオMUTE設定
 *  param1: STAUDIOハンドラ
 *  param2: MUTE設定コマンド(AUDIO_MUTE_ONまたはAUDIO_MUTE_OFF)
 *  return: E_OKで正常設定
 */
ER
staudio_out_setmute(STAUDIO_Handle_t *haudio, uint32_t Cmd)
{
	/* Call the Codec Mute function */
	if(wm8994_SetMute(haudio, Cmd) != 0)
		return E_SYS;
	else
		return E_OK;
}

/*
 *  オーディオ出力モード設定
 *  param1: STAUDIOハンドラ
 *  param2: 出力モード(OUTPUT_DEVICE_SPEAKER,UTPUT_DEVICE_HEADPHONEまたはOUTPUT_DEVICE_BOTH)
 *  return: E_OKで正常設定
 */
ER
staudio_out_setoutputmode(STAUDIO_Handle_t *haudio, uint8_t Output)
{
	/* Call the Codec output device function */
	if(wm8994_SetOutputMode(haudio, Output) != 0)
		return E_SYS;
	else
		return E_OK;
}

/*
 *  オーディオ周波数設定
 *  param1: STAUDIOハンドラ
 *  param2: 周波数値
 *  return: E_OKで正常設定
 */
void
staudio_out_setfrequency(STAUDIO_Handle_t *haudio, uint32_t AudioFreq)
{
	/* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */ 
	audio_clockconfig(haudio->hsai, AudioFreq, NULL);

	/* Disable SAI peripheral to allow access to SAI internal registers */
	audio_disable(haudio->hsai, AUDIO_OUT_BLOCK);

	/* Update the SAI audio frequency configuration */
	haudio->hsai->OutInit.AudioFrequency = AudioFreq;
	audio_start(haudio->hsai, AUDIO_OUT_BLOCK);

	/* Enable SAI peripheral to generate MCLK */
	audio_enable(haudio->hsai, AUDIO_OUT_BLOCK);
}

/*
 *  オーディオフレームスロット設定
 *  param1: STAUDIOハンドラ
 *  param2: フレームスロット値
 */
void
staudio_out_setframeslot(STAUDIO_Handle_t *haudio, uint32_t AudioFrameSlot)
{
	/* Disable SAI peripheral to allow access to SAI internal registers */
	audio_disable(haudio->hsai, AUDIO_OUT_BLOCK);

	/* Update the SAI audio frame slot configuration */
	haudio->hsai->OutInit.SlotActive = AudioFrameSlot;
	audio_start(haudio->hsai, AUDIO_OUT_BLOCK);

	/* Enable SAI peripheral to generate MCLK */
	audio_enable(haudio->hsai, AUDIO_OUT_BLOCK);
}

/*
 *  オーディオ出力バッファ変更
 *  param1: STAUDIOハンドラ
 *  param2: 出力バッファポインタ
 *  param3: バッファサイズ
 */
void
staudio_out_changebuffer(STAUDIO_Handle_t *haudio, uint16_t *pData, uint16_t Size)
{
	haudio->hsai->transcallback = haudio->audio_fullout_func;
	haudio->hsai->transhalfcallback = haudio->audio_halfout_func;
	haudio->hsai->errorcallback = haudio->audio_error_func;
	audio_transmit(haudio->hsai, (uint8_t*)pData, Size);
}

/*
 *  オーディオ出力処理終了
 *  param1: STAUDIOハンドラ
 */
void
staudio_out_deinit(STAUDIO_Handle_t *haudio)
{
	if(haudio->hsai != NULL){
		/* Disable SAI peripheral */
		audio_disable(haudio->hsai, AUDIO_OUT_BLOCK);
		audio_end(haudio->hsai, AUDIO_OUT_BLOCK);
		if(audio_status(haudio->hsai, AUDIO_IN_BLOCK) == AUDIO_STATUS_RESET){
			/* DeInit the SAI MSP : this __weak function can be rewritten by the application */
			audio_deinit(haudio->hsai, AUDIO_OUT_BLOCK);
			haudio->hsai = NULL;
		}
	}
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/
/**
  * @brief  Initializes the input Audio Codec audio interface (SAI).
  * @param  SaiOutMode: SAI_MODEMASTER_TX (for record and playback in parallel)
  *                     or SAI_MODEMASTER_RX (for record only).
  * @param  SlotActive: CODEC_AUDIOFRAME_SLOT_02, CODEC_AUDIOFRAME_SLOT_13 or
  *                     CODEC_AUDIOFRAME_SLOT_0123
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @retval None
  */
static void SAIx_In_Init(STAUDIO_Handle_t *haudio, uint32_t SaiOutMode, uint32_t SlotActive, uint32_t AudioFreq)
{
	/* Initialize the haudio_out_sai instance parameters */
	AUDIO_Handle_t *hsai = haudio->hsai;

	/* Disable SAI peripheral to allow access to SAI internal registers */
	audio_disable(hsai, AUDIO_OUT_BLOCK);

	/* Configure SAI_Block_x
	LSBFirst: Disabled
	DataSize: 16 */
	hsai->OutInit.AudioFrequency = AudioFreq;
#ifdef TOPPERS_STM32F769_DISCOVERY
	hsai->OutInit.AudioMode = SAI_MODEMASTER_RX;
#else
	hsai->OutInit.AudioMode = SaiOutMode;
#endif
	hsai->OutInit.MonoStereoMode = SAI_STEREOMODE;
	hsai->OutInit.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	hsai->OutInit.Protocol = SAI_FREE_PROTOCOL;
	hsai->OutInit.DataSize = SAI_DATASIZE_16;
	hsai->OutInit.FirstBit = SAI_FIRSTBIT_MSB;
#if defined(TOPPERS_STM32F769_DISCOVERY) || defined(TOPPERS_STM32F7_DISCOVERY)
	hsai->OutInit.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
	hsai->OutInit.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
#else
	hsai->OutInit.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
	hsai->OutInit.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
#endif
	hsai->OutInit.Synchro = SAI_ASYNCHRONOUS;
	hsai->OutInit.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
#if defined(TOPPERS_STM32F769_DISCOVERY)
	hsai->OutInit.SynchroExt    = SAI_SYNCEXT_DISABLE;
#endif
#ifndef TOPPERS_STM32F7_DISCOVERY
	hsai->OutInit.CompandingMode= SAI_NOCOMPANDING;
	hsai->OutInit.TriState      = SAI_OUTPUT_NOTRELEASED;
	hsai->OutInit.Mckdiv        = 0;
#endif

	/* Configure SAI_Block_x Frame
	Frame Length: 64
	Frame active Length: 32
	FS Definition: Start frame + Channel Side identification
	FS Polarity: FS active Low
	FS Offset: FS asserted one bit before the first bit of slot 0 */
	hsai->OutInit.FrameLength = 64;
	hsai->OutInit.ActiveFrameLength = 32;
	hsai->OutInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	hsai->OutInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	hsai->OutInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

	/* Configure SAI Block_x Slot
	Slot First Bit Offset: 0
	Slot Size  : 16
	Slot Number: 4
	Slot Active: All slot actives */
	hsai->OutInit.FirstBitOffset = 0;
	hsai->OutInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	hsai->OutInit.SlotNumber = 4;
	hsai->OutInit.SlotActive = SlotActive;

	audio_start(hsai, AUDIO_OUT_BLOCK);

	/* Initialize SAI2 block B in SLAVE RX synchronous from SAI2 block A */
	/* Initialize the haudio_in_sai Instance parameter */

	/* Disable SAI peripheral to allow access to SAI internal registers */
	audio_disable(hsai, AUDIO_IN_BLOCK);

	/* Configure SAI_Block_x
	LSBFirst: Disabled
	DataSize: 16 */
	hsai->InInit.AudioFrequency = AudioFreq;
	hsai->InInit.AudioMode = SAI_MODESLAVE_RX;
	hsai->InInit.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	hsai->InInit.Protocol = SAI_FREE_PROTOCOL;
	hsai->InInit.DataSize = SAI_DATASIZE_16;
	hsai->InInit.FirstBit = SAI_FIRSTBIT_MSB;
	hsai->InInit.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
	hsai->InInit.Synchro = SAI_SYNCHRONOUS;
	hsai->InInit.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	hsai->InInit.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
#ifdef TOPPERS_STM32F7_DISCOVERY
	hsai->InInit.SynchroExt    = SAI_SYNCEXT_DISABLE;
#endif
#ifndef TOPPERS_STM32F7_DISCOVERY
	hsai->InInit.CompandingMode= SAI_NOCOMPANDING;
	hsai->InInit.TriState      = SAI_OUTPUT_NOTRELEASED;
	hsai->InInit.Mckdiv        = 0;
#endif

	/* Configure SAI_Block_x Frame
	Frame Length: 64
	Frame active Length: 32
	FS Definition: Start frame + Channel Side identification
	FS Polarity: FS active Low
	FS Offset: FS asserted one bit before the first bit of slot 0 */
	hsai->InInit.FrameLength = 64;
	hsai->InInit.ActiveFrameLength = 32;
	hsai->InInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	hsai->InInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	hsai->InInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

	/* Configure SAI Block_x Slot
	Slot First Bit Offset: 0
	Slot Size  : 16
	Slot Number: 4
	Slot Active: All slot active */
	hsai->InInit.FirstBitOffset = 0;
	hsai->InInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	hsai->InInit.SlotNumber = 4;
	hsai->InInit.SlotActive = SlotActive;

	audio_start(hsai, AUDIO_IN_BLOCK);

	/* Enable SAI peripheral to generate MCLK */
	audio_enable(hsai, AUDIO_OUT_BLOCK);

	/* Enable SAI peripheral */
	audio_enable(hsai, AUDIO_IN_BLOCK);
}


/**
  * @brief  Deinitializes the output Audio Codec audio interface (SAI).
  * @retval None
  */
static void SAIx_In_DeInit(STAUDIO_Handle_t *haudio)
{
	if(haudio->hsai != NULL){
		/* Disable SAI peripheral */
		audio_disable(haudio->hsai, AUDIO_IN_BLOCK);
		audio_end(haudio->hsai, AUDIO_IN_BLOCK);
		if(audio_status(haudio->hsai, AUDIO_OUT_BLOCK) == AUDIO_STATUS_RESET){
			/* DeInit the SAI MSP : this __weak function can be rewritten by the application */
			audio_deinit(haudio->hsai, AUDIO_IN_BLOCK);
			haudio->hsai = NULL;
		}
	}
}


#ifdef TOPPERS_STM32F769_DISCOVERY
/**
  * @brief  Regular conversion complete callback. 
  * @note   In interrupt mode, user has to read conversion value in this function
            using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
static void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_Handle_t *hdfsdm_filter)
{
	uint32_t index = 0;

	if(hdfsdm_filter == &hAudioInTopLeftFilter){
		DmaTopLeftRecCplt = 1;
	}
	else if(hdfsdm_filter == &hAudioInTopRightFilter){
		DmaTopRightRecCplt = 1;
	}
	else if(hdfsdm_filter == &hAudioInButtomLeftFilter){
		DmaButtomLeftRecCplt = 1;
	}
	else{
		DmaButtomRightRecCplt = 1;
	}

	if(AudioIn_ChannelNumber > 2){
		if((DmaTopLeftRecCplt == 1) && (DmaTopRightRecCplt == 1) && (DmaButtomLeftRecCplt == 1) && (DmaButtomRightRecCplt == 1)){
			for(index = (ScratchSize/2) ; index < ScratchSize; index++){
				hAudioIn.pRecBuf[AppBuffTrigger]     = (uint16_t)(SaturaLH((pScratchBuff[1][index] >> 8), -32760, 32760));
				hAudioIn.pRecBuf[AppBuffTrigger + 1] = (uint16_t)(SaturaLH((pScratchBuff[0][index] >> 8), -32760, 32760));       
				hAudioIn.pRecBuf[AppBuffTrigger + 2] = (uint16_t)(SaturaLH((pScratchBuff[3][index] >> 8), -32760, 32760));
				hAudioIn.pRecBuf[AppBuffTrigger + 3] = (uint16_t)(SaturaLH((pScratchBuff[2][index] >> 8), -32760, 32760));      
				AppBuffTrigger +=4;
			}
			DmaTopLeftRecCplt  = 0;
			DmaTopRightRecCplt = 0;
			DmaButtomLeftRecCplt  = 0;
			DmaButtomRightRecCplt = 0;
		}
	}
	else{
		if((DmaTopLeftRecCplt == 1) && (DmaTopRightRecCplt == 1)){
			for(index = (ScratchSize/2) ; index < ScratchSize; index++){
				hAudioIn.pRecBuf[AppBuffTrigger]     = (uint16_t)(SaturaLH((pScratchBuff[1][index] >> 8), -32760, 32760));
				hAudioIn.pRecBuf[AppBuffTrigger + 1] = (uint16_t)(SaturaLH((pScratchBuff[0][index] >> 8), -32760, 32760));
				AppBuffTrigger +=2;
			}
			DmaTopLeftRecCplt  = 0;
			DmaTopRightRecCplt = 0;  
		}
	}

	/* Call Half Transfer Complete callback */
	if((AppBuffTrigger == hAudioIn.RecSize/2) && (AppBuffHalf == 0)){
		AppBuffHalf = 1;
		if(hstaudio != NULL && hstaudio->audio_halfin_func)
			hstaudio->audio_halfin_func(hdfsdm_filter);
	}
	/* Call Transfer Complete callback */
	if(AppBuffTrigger == hAudioIn.RecSize){
		/* Reset Application Buffer Trigger */
		AppBuffTrigger = 0;
		AppBuffHalf = 0; 
		/* Call the record update function to get the next buffer to fill and its size (size is ignored) */
		if(hstaudio != NULL && hstaudio->audio_fullin_func)
			hstaudio->audio_fullin_func(hdfsdm_filter);
	}
}

/**
  * @brief  Half regular conversion complete callback. 
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
static void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_Handle_t *hdfsdm_filter)
{
	uint32_t index = 0;

	if(hdfsdm_filter == &hAudioInTopLeftFilter){
		DmaTopLeftRecHalfCplt = 1;
	}
	else if(hdfsdm_filter == &hAudioInTopRightFilter){
		DmaTopRightRecHalfCplt = 1;
	}
	else if(hdfsdm_filter == &hAudioInButtomLeftFilter){
		DmaButtomLeftRecHalfCplt = 1;
	}
	else{
		DmaButtomRightRecHalfCplt = 1;
	}

	if(AudioIn_ChannelNumber > 2){
		if((DmaTopLeftRecHalfCplt == 1) && (DmaTopRightRecHalfCplt == 1) && (DmaButtomLeftRecHalfCplt == 1) && (DmaButtomRightRecHalfCplt == 1)){
			for(index = 0 ; index < ScratchSize/2; index++){
				hAudioIn.pRecBuf[AppBuffTrigger]     = (uint16_t)(SaturaLH((pScratchBuff[1][index] >> 8), -32760, 32760));
				hAudioIn.pRecBuf[AppBuffTrigger + 1] = (uint16_t)(SaturaLH((pScratchBuff[0][index] >> 8), -32760, 32760)); 
				hAudioIn.pRecBuf[AppBuffTrigger + 2] = (uint16_t)(SaturaLH((pScratchBuff[3][index] >> 8), -32760, 32760));
				hAudioIn.pRecBuf[AppBuffTrigger + 3] = (uint16_t)(SaturaLH((pScratchBuff[2][index] >> 8), -32760, 32760));      
 				AppBuffTrigger +=4;
			}
			DmaTopLeftRecHalfCplt  = 0;
			DmaTopRightRecHalfCplt = 0;
			DmaButtomLeftRecHalfCplt  = 0;
			DmaButtomRightRecHalfCplt = 0;
		}
	}
	else{
		if((DmaTopLeftRecHalfCplt == 1) && (DmaTopRightRecHalfCplt == 1)){
			for(index = 0 ; index < ScratchSize/2; index++){
				hAudioIn.pRecBuf[AppBuffTrigger]     = (uint16_t)(SaturaLH((pScratchBuff[1][index] >> 8), -32760, 32760));
				hAudioIn.pRecBuf[AppBuffTrigger + 1] = (uint16_t)(SaturaLH((pScratchBuff[0][index] >> 8), -32760, 32760));
				AppBuffTrigger +=2;
			}
			DmaTopLeftRecHalfCplt  = 0;
			DmaTopRightRecHalfCplt = 0;
		}
	}

	/* Call Half Transfer Complete callback */
	if((AppBuffTrigger == hAudioIn.RecSize/2) && (AppBuffHalf == 0)){
		AppBuffHalf = 1;
		if(hstaudio != NULL && hstaudio->audio_halfin_func)
			hstaudio->audio_halfin_func(hdfsdm_filter);
	}
	/* Call Transfer Complete callback */
	if(AppBuffTrigger == hAudioIn.RecSize){
		/* Reset Application Buffer Trigger */
		AppBuffTrigger = 0;
		AppBuffHalf = 0;
		/* Call the record update function to get the next buffer to fill and its size (size is ignored) */
		if(hstaudio != NULL && hstaudio->audio_fullin_func)
			hstaudio->audio_fullin_func(hdfsdm_filter);
	}
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/
/**
  * @brief  Initialize the Digital Filter for Sigma-Delta Modulators interface (DFSDM).
  * @param  AudioFreq: Audio frequency to be used to set correctly the DFSDM peripheral.
  * @note   Channel output Clock Divider and Filter Oversampling are calculated as follow: 
  *         - Clock_Divider = CLK(input DFSDM)/CLK(micro) with
  *           1MHZ < CLK(micro) < 3.2MHZ (TYP 2.4MHZ for MP34DT01TR)
  *         - Oversampling = CLK(input DFSDM)/(Clock_Divider * AudioFreq)  
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
static ER DFSDMx_Init(uint32_t AudioFreq)
{
	ER ercd = E_OK;

	/****************************************************************************/ 
	/********************** Channels configuration  *****************************/
	/****************************************************************************/ 
	/* CHANNEL 1 configuration */
	hAudioInTopLeftChannel.state                         = DFSDM_CHANNEL_STATE_RESET;
	hAudioInTopLeftChannel.base                          = (uint32_t)TADR_DFSDM1_CHANNEL1_BASE;
	hAudioInTopLeftChannel.Init.OutClockActivation       = 1 /* ENABLE */;
	hAudioInTopLeftChannel.Init.OutClockSelection        = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
	/* Set the DFSDM clock OUT audio frequency configuration */
	hAudioInTopLeftChannel.Init.OutClockDivider          = DFSDM_CLOCK_DIVIDER(AudioFreq);
	hAudioInTopLeftChannel.Init.InputMultiplexer         = DFSDM_CHANNEL_EXTERNAL_INPUTS;
	hAudioInTopLeftChannel.Init.InputDataPacking         = DFSDM_CHANNEL_STANDARD_MODE;
	hAudioInTopLeftChannel.Init.InputPins                = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
	/* Request to sample stable data for LEFT micro on Rising edge */
	hAudioInTopLeftChannel.Init.SerialType               = DFSDM_CHANNEL_SPI_RISING;
	hAudioInTopLeftChannel.Init.SerialSpiClock           = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
	hAudioInTopLeftChannel.Init.AwdFilterOrder           = DFSDM_CHANNEL_FASTSINC_ORDER;
	hAudioInTopLeftChannel.Init.AwdOversampling          = 10;
	hAudioInTopLeftChannel.Init.Offset                   = 0;
	hAudioInTopLeftChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
	if(E_OK != (ercd = dfsdm_channel_init(&hAudioInTopLeftChannel))){
		return ercd;
	}

	/* CHANNEL 0 configuration */
	hAudioInTopRightChannel.state                         = DFSDM_CHANNEL_STATE_RESET;
	hAudioInTopRightChannel.base                          = (uint32_t)TADR_DFSDM1_CHANNEL0_BASE;
	hAudioInTopRightChannel.Init.OutClockActivation       = 1 /* ENABLE*/;
	hAudioInTopRightChannel.Init.OutClockSelection        = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
	/* Set the DFSDM clock OUT audio frequency configuration */
	hAudioInTopRightChannel.Init.OutClockDivider          = DFSDM_CLOCK_DIVIDER(AudioFreq);
	hAudioInTopRightChannel.Init.InputMultiplexer         = DFSDM_CHANNEL_EXTERNAL_INPUTS;
	hAudioInTopRightChannel.Init.InputDataPacking         = DFSDM_CHANNEL_STANDARD_MODE;
	hAudioInTopRightChannel.Init.InputPins                = DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS;
	/* Request to sample stable data for RIGHT micro on Falling edge */
	hAudioInTopRightChannel.Init.SerialType               = DFSDM_CHANNEL_SPI_FALLING;
	hAudioInTopRightChannel.Init.SerialSpiClock           = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
	hAudioInTopRightChannel.Init.AwdFilterOrder           = DFSDM_CHANNEL_FASTSINC_ORDER;
	hAudioInTopRightChannel.Init.AwdOversampling          = 10;
	hAudioInTopRightChannel.Init.Offset                   = 0;
	hAudioInTopRightChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
	if(E_OK != (ercd = dfsdm_channel_init(&hAudioInTopRightChannel))){
		return ercd;
	}

	if(AudioIn_ChannelNumber > 2){
		/* CHANNEL 5 configuration */
		hAudioInButtomLeftChannel.state                         = DFSDM_CHANNEL_STATE_RESET;
		hAudioInButtomLeftChannel.base                          = (uint32_t)TADR_DFSDM1_CHANNEL5_BASE;
		hAudioInButtomLeftChannel.Init.OutClockActivation       = 1 /* ENABLE */;
		hAudioInButtomLeftChannel.Init.OutClockSelection        = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
		/* Set the DFSDM clock OUT audio frequency configuration */
		hAudioInButtomLeftChannel.Init.OutClockDivider          = DFSDM_CLOCK_DIVIDER(AudioFreq);
		hAudioInButtomLeftChannel.Init.InputMultiplexer         = DFSDM_CHANNEL_EXTERNAL_INPUTS;
		hAudioInButtomLeftChannel.Init.InputDataPacking         = DFSDM_CHANNEL_STANDARD_MODE;
		hAudioInButtomLeftChannel.Init.InputPins                = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
		/* Request to sample stable data for LEFT micro on Rising edge */
		hAudioInButtomLeftChannel.Init.SerialType               = DFSDM_CHANNEL_SPI_RISING;
		hAudioInButtomLeftChannel.Init.SerialSpiClock           = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
		hAudioInButtomLeftChannel.Init.AwdFilterOrder           = DFSDM_CHANNEL_FASTSINC_ORDER;
		hAudioInButtomLeftChannel.Init.AwdOversampling          = 10;
		hAudioInButtomLeftChannel.Init.Offset                   = 0;
		hAudioInButtomLeftChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
		if(E_OK != (ercd = dfsdm_channel_init(&hAudioInButtomLeftChannel))){
			return ercd;
		}

		/* CHANNEL 4 configuration */
		hAudioInButtomRightChannel.state                         = DFSDM_CHANNEL_STATE_RESET;
		hAudioInButtomRightChannel.base                          = (uint32_t)TADR_DFSDM1_CHANNEL4_BASE;
		hAudioInButtomRightChannel.Init.OutClockActivation       = 1 /* ENABLE */;
		hAudioInButtomRightChannel.Init.OutClockSelection        = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
		/* Set the DFSDM clock OUT audio frequency configuration */
		hAudioInButtomRightChannel.Init.OutClockDivider          = DFSDM_CLOCK_DIVIDER(AudioFreq);
		hAudioInButtomRightChannel.Init.InputMultiplexer         = DFSDM_CHANNEL_EXTERNAL_INPUTS;
		hAudioInButtomRightChannel.Init.InputDataPacking         = DFSDM_CHANNEL_STANDARD_MODE;
		hAudioInButtomRightChannel.Init.InputPins                = DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS;
		/* Request to sample stable data for RIGHT micro on Falling edge */
		hAudioInButtomRightChannel.Init.SerialType               = DFSDM_CHANNEL_SPI_FALLING;
		hAudioInButtomRightChannel.Init.SerialSpiClock           = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
		hAudioInButtomRightChannel.Init.AwdFilterOrder           = DFSDM_CHANNEL_FASTSINC_ORDER;
		hAudioInButtomRightChannel.Init.AwdOversampling          = 10;
		hAudioInButtomRightChannel.Init.Offset                   = 0;
		hAudioInButtomRightChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
		if(E_OK != (ercd = dfsdm_channel_init(&hAudioInButtomRightChannel))){
			return ercd;
		}
	}
	/****************************************************************************/ 
	/********************** Filters configuration  ******************************/
	/****************************************************************************/
	/* FILTER 0 configuration */
	hAudioInTopLeftFilter.state                             = DFSDM_FILTER_STATE_RESET;
	hAudioInTopLeftFilter.base                              = (uint32_t)AUDIO_DFSDMx_TOP_LEFT_FILTER;
	hAudioInTopLeftFilter.regconvhalfcallback               = HAL_DFSDM_FilterRegConvHalfCpltCallback;
	hAudioInTopLeftFilter.regconvcallback                   = HAL_DFSDM_FilterRegConvCpltCallback;
	hAudioInTopLeftFilter.Init.RegTrigger                   = DFSDM_FILTER_SW_TRIGGER;
	hAudioInTopLeftFilter.Init.RegFastMode                  = 1 /* ENABLE */;
	hAudioInTopLeftFilter.Init.RegDmaMode                   = 1 /* ENABLE */;
	hAudioInTopLeftFilter.Init.InjTrigger                   = DFSDM_FILTER_SW_TRIGGER;
	hAudioInTopLeftFilter.Init.InjScanMode                  = 1 /* ENABLE */;
	hAudioInTopLeftFilter.Init.InjDmaMode                   = 0 /* DISABLE */;
	hAudioInTopLeftFilter.Init.InjExtTrigger                = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
	hAudioInTopLeftFilter.Init.InjExtTriggerEdge            = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
	hAudioInTopLeftFilter.Init.FilterSincOrder              = DFSDM_FILTER_ORDER(AudioFreq);
	/* Set the DFSDM Filters Oversampling to have correct sample rate */
	hAudioInTopLeftFilter.Init.FilterOversampling           = DFSDM_OVER_SAMPLING(AudioFreq);
	hAudioInTopLeftFilter.Init.FilterIntOversampling        = 1;
	if(E_OK != (ercd = dfsdm_filter_init(&hAudioInTopLeftFilter))){
		return ercd;
	}

	/* Configure injected channel */
	if(E_OK != (ercd = dfsdm_filter_config_reg(&hAudioInTopLeftFilter, AUDIO_DFSDMx_TOP_LEFT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))){
		return ercd;
	}

	/* FILTER 1 configuration */
	hAudioInTopRightFilter.state                             = DFSDM_FILTER_STATE_RESET;
	hAudioInTopRightFilter.base                              = (uint32_t)AUDIO_DFSDMx_TOP_RIGHT_FILTER;
	hAudioInTopRightFilter.regconvhalfcallback               = HAL_DFSDM_FilterRegConvHalfCpltCallback;
	hAudioInTopRightFilter.regconvcallback                   = HAL_DFSDM_FilterRegConvCpltCallback;
	hAudioInTopRightFilter.Init.RegTrigger                   = DFSDM_FILTER_SYNC_TRIGGER;
	hAudioInTopRightFilter.Init.RegFastMode                  = 1 /* ENABLE */;
	hAudioInTopRightFilter.Init.RegDmaMode                   = 1 /* ENABLE */;
	hAudioInTopRightFilter.Init.InjTrigger                   = DFSDM_FILTER_SW_TRIGGER;
	hAudioInTopRightFilter.Init.InjScanMode                  = 0 /* DISABLE */;
	hAudioInTopRightFilter.Init.InjDmaMode                   = 0 /* DISABLE */;
	hAudioInTopRightFilter.Init.InjExtTrigger                = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
	hAudioInTopRightFilter.Init.InjExtTriggerEdge            = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
	hAudioInTopRightFilter.Init.FilterSincOrder              = DFSDM_FILTER_ORDER(AudioFreq);
	/* Set the DFSDM Filters Oversampling to have correct sample rate */
	hAudioInTopRightFilter.Init.FilterOversampling           = DFSDM_OVER_SAMPLING(AudioFreq);
	hAudioInTopRightFilter.Init.FilterIntOversampling        = 1;
	if(E_OK != (ercd = dfsdm_filter_init(&hAudioInTopRightFilter))){
		return ercd;
	}
	/* Configure injected channel */
	if(E_OK != (ercd = dfsdm_filter_config_reg(&hAudioInTopRightFilter, AUDIO_DFSDMx_TOP_RIGHT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))){
		return ercd;
	}

	if(AudioIn_ChannelNumber > 2){
		/* FILTER 2 configuration */
		hAudioInButtomLeftFilter.state                             = DFSDM_FILTER_STATE_RESET;
		hAudioInButtomLeftFilter.base                              = (uint32_t)AUDIO_DFSDMx_BUTTOM_LEFT_FILTER;
		hAudioInButtomLeftFilter.regconvhalfcallback               = HAL_DFSDM_FilterRegConvHalfCpltCallback;
		hAudioInButtomLeftFilter.regconvcallback                   = HAL_DFSDM_FilterRegConvCpltCallback;
		hAudioInButtomLeftFilter.Init.RegTrigger                   = DFSDM_FILTER_SYNC_TRIGGER;
		hAudioInButtomLeftFilter.Init.RegFastMode                  = 1 /* ENABLE */;
		hAudioInButtomLeftFilter.Init.RegDmaMode                   = 1 /* ENABLE */;
		hAudioInButtomLeftFilter.Init.InjTrigger                   = DFSDM_FILTER_SW_TRIGGER;
		hAudioInButtomLeftFilter.Init.InjScanMode                  = 1 /* ENABLE */;
		hAudioInButtomLeftFilter.Init.InjDmaMode                   = 0 /* DISABLE */;
		hAudioInButtomLeftFilter.Init.InjExtTrigger                = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
		hAudioInButtomLeftFilter.Init.InjExtTriggerEdge            = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
		hAudioInButtomLeftFilter.Init.FilterSincOrder              = DFSDM_FILTER_ORDER(AudioFreq);
		/* Set the DFSDM Filters Oversampling to have correct sample rate */
		hAudioInButtomLeftFilter.Init.FilterOversampling           = DFSDM_OVER_SAMPLING(AudioFreq);
		hAudioInButtomLeftFilter.Init.FilterIntOversampling        = 1;
		if(E_OK != (ercd = dfsdm_filter_init(&hAudioInButtomLeftFilter))){
			return ercd;
		}

		/* Configure injected channel */
		if(E_OK != (ercd = dfsdm_filter_config_reg(&hAudioInButtomLeftFilter, AUDIO_DFSDMx_BUTTOM_LEFT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))){
			return ercd;
		}

		/* FILTER 3 configuration */
		hAudioInButtomRightFilter.state                             = DFSDM_FILTER_STATE_RESET;
		hAudioInButtomRightFilter.base                              = (uint32_t)AUDIO_DFSDMx_BUTTOM_RIGHT_FILTER;
		hAudioInButtomRightFilter.regconvhalfcallback               = HAL_DFSDM_FilterRegConvHalfCpltCallback;
		hAudioInButtomRightFilter.regconvcallback                   = HAL_DFSDM_FilterRegConvCpltCallback;
		hAudioInButtomRightFilter.Init.RegTrigger                   = DFSDM_FILTER_SYNC_TRIGGER;
		hAudioInButtomRightFilter.Init.RegFastMode                  = 1 /* ENABLE */;
		hAudioInButtomRightFilter.Init.RegDmaMode                   = 1 /* ENABLE */;
		hAudioInButtomRightFilter.Init.InjTrigger                   = DFSDM_FILTER_SW_TRIGGER;
		hAudioInButtomRightFilter.Init.InjScanMode                  = 0 /* DISABLE */;
		hAudioInButtomRightFilter.Init.InjDmaMode                   = 0 /* DISABLE */;
		hAudioInButtomRightFilter.Init.InjExtTrigger                = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
		hAudioInButtomRightFilter.Init.InjExtTriggerEdge            = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
		hAudioInButtomRightFilter.Init.FilterSincOrder              = DFSDM_FILTER_ORDER(AudioFreq);
		/* Set the DFSDM Filters Oversampling to have correct sample rate */
		hAudioInButtomRightFilter.Init.FilterOversampling           = DFSDM_OVER_SAMPLING(AudioFreq);
		hAudioInButtomRightFilter.Init.FilterIntOversampling        = 1;
		if(E_OK != (ercd = dfsdm_filter_init(&hAudioInButtomRightFilter))){
			return ercd;
		}
		/* Configure injected channel */
		if(E_OK != (ercd = dfsdm_filter_config_reg(&hAudioInButtomRightFilter, AUDIO_DFSDMx_BUTTOM_RIGHT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))){
			return ercd;
		}
	}
	return ercd;
}

/**
  * @brief  De-initialize the Digital Filter for Sigma-Delta Modulators interface (DFSDM).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
static ER DFSDMx_DeInit(void)
{
	ER ercd = E_OK;

	/* De-initializes the DFSDM filters to allow access to DFSDM internal registers */
	if(E_OK != (ercd = dfsdm_filter_deinit(&hAudioInTopLeftFilter))){
		return ercd;
	}
	if(E_OK != (ercd = dfsdm_filter_deinit(&hAudioInTopRightFilter))){
		return ercd;
	}

	/* De-initializes the DFSDM channels to allow access to DFSDM internal registers */
	if(E_OK != (ercd = dfsdm_channel_deinit(&hAudioInTopLeftChannel))){
		return ercd;
	}
	if(E_OK != (ercd = dfsdm_channel_deinit(&hAudioInTopRightChannel))){
		return ercd;
  }

	if(AudioIn_ChannelNumber > 2){
		/* De-initializes the DFSDM filters to allow access to DFSDM internal registers */        
		if(E_OK != (ercd = dfsdm_filter_deinit(&hAudioInButtomLeftFilter))){
			return ercd;
		}
		if(E_OK != (ercd = dfsdm_filter_deinit(&hAudioInButtomRightFilter))){
			return ercd;
		}

		/* De-initializes the DFSDM channels to allow access to DFSDM internal registers */  
		if(E_OK != (ercd = dfsdm_channel_deinit(&hAudioInButtomLeftChannel))){
			return ercd;
		}
		if(E_OK != (ercd = dfsdm_channel_deinit(&hAudioInButtomRightChannel))){
			return ercd;
		}
	}
	return ercd;
}

/**
  * @brief  Initialize the DFSDM channel MSP.
  * @param  hdfsdm_channel : DFSDM channel handle.
  * @retval None
  */
static void DFSDMx_ChannelMspInit(void)
{
	GPIO_Init_t  GPIO_InitStruct;

	/* Enable DFSDM clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_DFSDM1EN);

	/* Enable GPIO clock */
	AUDIO_DFSDMx_DMIC_DATIN_GPIO_CLK_ENABLE();
	AUDIO_DFSDMx_CKOUT_DMIC_GPIO_CLK_ENABLE();

	/* DFSDM pins configuration: DFSDM_CKOUT, DMIC_DATIN1 pins ------------------*/
	GPIO_InitStruct.mode = GPIO_MODE_AF;
	GPIO_InitStruct.pull = GPIO_NOPULL;
	GPIO_InitStruct.otype = GPIO_OTYPE_PP;
	GPIO_InitStruct.speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.alternate = AUDIO_DFSDMx_CKOUT_AF;
	gpio_setup((uint32_t)AUDIO_DFSDMx_CKOUT_DMIC_GPIO_PORT, &GPIO_InitStruct, AUDIO_DFSDMx_CKOUT_PINNO);

	/* DFSDM pin configuration: DMIC_DATIN1 pin --------------------------------*/
	GPIO_InitStruct.alternate = AUDIO_DFSDMx_DMIC_DATIN_AF;
	gpio_setup((uint32_t)AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT, &GPIO_InitStruct, AUDIO_DFSDMx_DMIC_DATIN1_PINNO);

	if(AudioIn_ChannelNumber > 2){
		/* DFSDM pin configuration: DMIC_DATIN5 pin --------------------------------*/  
		GPIO_InitStruct.alternate = AUDIO_DFSDMx_DMIC_DATIN_AF;
		gpio_setup((uint32_t)AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT, &GPIO_InitStruct, AUDIO_DFSDMx_DMIC_DATIN5_PINNO);
	}
}

/**
  * @brief  Initialize the DFSDM filter MSP.
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
static void DFSDMx_FilterMspInit(void)
{
	/* Enable DFSDM clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_DFSDM1EN);

	/* Enable the DMA clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2EN);

	/*********** Configure DMA stream for TOP LEFT microphone *******************/
	hDmaTopLeft.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hDmaTopLeft.Init.PeriphInc           = DMA_PINC_DISABLE;
	hDmaTopLeft.Init.MemInc              = DMA_MINC_ENABLE;
	hDmaTopLeft.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
	hDmaTopLeft.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
	hDmaTopLeft.Init.Mode                = DMA_CIRCULAR;
	hDmaTopLeft.Init.Priority            = DMA_PRIORITY_HIGH;
	hDmaTopLeft.base                     = (uint32_t)TADR_DMA2_STM4_BASE;
	hDmaTopLeft.Init.Channel             = AUDIO_DFSDMx_DMAx_CHANNEL; 

	/* Associate the DMA handle */
	hAudioInTopLeftFilter.hdmaReg = &hDmaTopLeft;
	hDmaTopLeft.localdata = &hAudioInTopLeftFilter;

	/* Reset DMA handle state */
	hDmaTopLeft.status = 0;

	/* Configure the DMA Channel */
	dma_init(&hDmaTopLeft);

	/*********** Configure DMA stream for TOP RIGHT microphone ******************/
	hDmaTopRight.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hDmaTopRight.Init.PeriphInc           = DMA_PINC_DISABLE;
	hDmaTopRight.Init.MemInc              = DMA_MINC_ENABLE;
	hDmaTopRight.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
	hDmaTopRight.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
	hDmaTopRight.Init.Mode                = DMA_CIRCULAR;
	hDmaTopRight.Init.Priority            = DMA_PRIORITY_HIGH;  
	hDmaTopRight.base                     = (uint32_t)TADR_DMA2_STM1_BASE;
	hDmaTopRight.Init.Channel             = AUDIO_DFSDMx_DMAx_CHANNEL;

	/* Associate the DMA handle */
	hAudioInTopRightFilter.hdmaReg        = &hDmaTopRight;
	hDmaTopRight.localdata                = &hAudioInTopRightFilter;

	/* Reset DMA handle state */
	hDmaTopRight.status = 0;

	/* Configure the DMA Channel */
	dma_init(&hDmaTopRight);

	if(AudioIn_ChannelNumber > 2){
		/*********** Configure DMA stream for BUTTOM LEFT microphone ****************/
		hDmaButtomLeft.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hDmaButtomLeft.Init.PeriphInc           = DMA_PINC_DISABLE;
		hDmaButtomLeft.Init.MemInc              = DMA_MINC_ENABLE;
		hDmaButtomLeft.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
		hDmaButtomLeft.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
		hDmaButtomLeft.Init.Mode                = DMA_CIRCULAR;
		hDmaButtomLeft.Init.Priority            = DMA_PRIORITY_HIGH;
		hDmaButtomLeft.base                     = (uint32_t)AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_STREAM;
		hDmaButtomLeft.Init.Channel             = AUDIO_DFSDMx_DMAx_CHANNEL; 

		/* Associate the DMA handle */
		hAudioInButtomLeftFilter.hdmaReg        = &hDmaButtomLeft;
		hDmaButtomLeft.localdata                = &hAudioInButtomLeftFilter;

		/* Reset DMA handle state */
		hDmaButtomLeft.status = 0;

		/* Configure the DMA Channel */
		dma_init(&hDmaButtomLeft);

		/*********** Configure DMA stream for BUTTOM RIGHT microphone ***************/
		hDmaButtomRight.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hDmaButtomRight.Init.PeriphInc           = DMA_PINC_DISABLE;
		hDmaButtomRight.Init.MemInc              = DMA_MINC_ENABLE;
		hDmaButtomRight.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
		hDmaButtomRight.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
		hDmaButtomRight.Init.Mode                = DMA_CIRCULAR;
		hDmaButtomRight.Init.Priority            = DMA_PRIORITY_HIGH;  
		hDmaButtomRight.base                     = (uint32_t)AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_STREAM;
		hDmaButtomRight.Init.Channel             = AUDIO_DFSDMx_DMAx_CHANNEL;

		/* Associate the DMA handle */
		hAudioInButtomRightFilter.hdmaReg        = &hDmaButtomRight;
		hDmaButtomRight.localdata                = &hAudioInButtomRightFilter;

		/* Reset DMA handle state */
		hDmaButtomRight.status = 0;

		/* Configure the DMA Channel */
		dma_init(&hDmaButtomRight);
	}
}

/**
  * @brief  DeInitialize the DFSDM filter MSP.
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
static void DFSDMx_FilterMspDeInit(void)
{
	/* Configure the DMA Channel */
	dma_deinit(&hDmaTopLeft);
	dma_deinit(&hDmaTopRight);
	if(AudioIn_ChannelNumber > 2){
		dma_deinit(&hDmaButtomLeft);
		dma_deinit(&hDmaButtomRight);
	}
}

void dfsdm1fl0_handler(void)
{
	dfsdm_irqhandler(&hAudioInTopLeftFilter);
}

void dfsdm1fl1_handler(void)
{
	dfsdm_irqhandler(&hAudioInTopRightFilter);
}

void dfsdm1fl2_handler(void)
{
	dfsdm_irqhandler(&hAudioInButtomLeftFilter);
}

void dfsdm1fl3_handler(void)
{
	dfsdm_irqhandler(&hAudioInButtomRightFilter);
}

#endif

/*
 *  オーディオ入力初期化設定
 *  param1: STAUDIOハンドラ
 *  param2: 入力デバイス設定
 *  param3: オーディオ周波数値
 *  param4: BitRes
 *  param5: チャネル番号
 *  return: E_OKで正常設定
 */
ER
staudio_in_init(STAUDIO_Handle_t *haudio, uint16_t InputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr)
{
	ER       ercd = E_OK;
	uint32_t slot_active;

	haudio->InputDevice = InputDevice;
	if(InputDevice == INPUT_DEVICE_DIGITAL_MIC){
#ifdef TOPPERS_STM32F769_DISCOVERY
		AudioIn_ChannelNumber = ChnlNbr;
		/* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */ 
		dfsdm_clockconfig(AUDIO2_PORTID, AudioFreq);
		sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_ADFSDM1SEL, (uint32_t)(RCC_DCKCFGR1_ADFSDM1SEL));

		/* Init the SAI MSP: this __weak function can be redefined by the application*/
		/* MSP channels initialization */
		DFSDMx_ChannelMspInit();
		/* MSP filters initialization */
		DFSDMx_FilterMspInit();

		/* Initializes DFSDM peripheral */
		DFSDMx_Init(AudioFreq);
		hstaudio = haudio;
		ercd = E_OK;
#else
		ercd = E_PAR;
#endif
	}
	/* Only INPUT_LINE_1, MICROPHONE_1, MICROPHONE_2 and MIC1&MIC2 inputs supported */
	else if((InputDevice != INPUT_DEVICE_INPUT_LINE_1) &&       
		(InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_1) &&
		(InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_2) &&
		(InputDevice != INPUT_DEVICE_DIGITAL_MIC1_MIC2)){
		ercd = E_PAR;
	}
	else{
		/* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
		if(haudio->hsai == NULL){
			haudio->hsai = audio_init(haudio->saiid);
		}
		else{
			/* Disable SAI peripheral */
			audio_disable(haudio->hsai, AUDIO_IN_BLOCK);
			audio_end(haudio->hsai, AUDIO_IN_BLOCK);
		}
		audio_clockconfig(haudio->hsai, AudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */

		/* Configure SAI in master RX mode :
		 *   - SAI2_block_A in master RX mode
		 *   - SAI2_block_B in slave RX mode synchronous from SAI2_block_A
		 */
		if(InputDevice == INPUT_DEVICE_DIGITAL_MICROPHONE_2){
			slot_active = CODEC_AUDIOFRAME_SLOT_13;
		}
		else if(InputDevice == INPUT_DEVICE_DIGITAL_MIC1_MIC2){
			slot_active = CODEC_AUDIOFRAME_SLOT_0123;
		}
		else{
			slot_active = CODEC_AUDIOFRAME_SLOT_02;
		}
		SAIx_In_Init(haudio, SAI_MODEMASTER_RX, slot_active, AudioFreq);

		/* wm8994 codec initialization */
		ercd = wm8994_Reset(haudio);
		if(ercd == E_OK){
			/* Initialize the codec internal registers */
			wm8994_Init(haudio, InputDevice, haudio->AudioInVolume, AudioFreq);
		}
	}
	return ercd;
}

#ifndef TOPPERS_STM32F769_DISCOVERY
/*
 *  オーディオ入出力初期化設定
 *  param1: STAUDIOハンドラ
 *  param2: 入力デバイス設定
 *  param3: 出力デバイス設定
 *  param4: オーディオ周波数値
 *  param5: BitRes
 *  param6: チャネル番号
 *  return: E_OKで正常設定
 */
ER
staudio_in_out_init(STAUDIO_Handle_t *haudio, uint16_t InputDevice, uint16_t OutputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr)
{
	ER       ercd = E_OK;
	uint32_t slot_active;

	haudio->InputDevice = InputDevice;
	/* Only INPUT_LINE_1, MICROPHONE_1, MICROPHONE_2 and MIC1&MIC2 inputs supported */
	if ((InputDevice != INPUT_DEVICE_INPUT_LINE_1) &&       
		(InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_1) &&
		(InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_2) &&
		(InputDevice != INPUT_DEVICE_DIGITAL_MIC1_MIC2)){
		ercd = E_PAR;
	}
	else{
		/* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
		if(haudio->hsai == NULL){
			haudio->hsai = audio_init(haudio->saiid);
		}
		else{
			/* Disable SAI peripheral */
			audio_disable(haudio->hsai, AUDIO_IN_BLOCK);
			audio_end(haudio->hsai, AUDIO_IN_BLOCK);
		}
		audio_clockconfig(haudio->hsai, AudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */

		/* Configure SAI in master RX mode :
		 *   - SAI2_block_A in master RX mode
		 *   - SAI2_block_B in slave RX mode synchronous from SAI2_block_A
		 */
		if(InputDevice == INPUT_DEVICE_DIGITAL_MICROPHONE_2){
			slot_active = CODEC_AUDIOFRAME_SLOT_13;
		}
		else if(InputDevice == INPUT_DEVICE_DIGITAL_MIC1_MIC2){
			slot_active = CODEC_AUDIOFRAME_SLOT_0123;
		}
		else{
			slot_active = CODEC_AUDIOFRAME_SLOT_02;
		}
		SAIx_In_Init(haudio, SAI_MODEMASTER_RX, slot_active, AudioFreq);

		/* wm8994 codec initialization */
		ercd = wm8994_Reset(haudio);
		if(ercd == E_OK){
			/* Initialize the codec internal registers */
			wm8994_Init(haudio, InputDevice | OutputDevice, haudio->AudioInVolume, AudioFreq);
		}
	}
	return ercd;
}
#endif

/*
 *  オーディオ録音開始設定
 *  param1: STAUDIOハンドラ
 *  param2: 入力バッファポインタ
 *  param3: 入力バッファサイズ
 *  return: E_OKで正常設定
 */
ER
staudio_in_record(STAUDIO_Handle_t *haudio, uint16_t* pbuf, uint32_t size)
{
	AUDIO_Handle_t  *hsai = haudio->hsai;
	ER ercd = E_OK;

#ifdef TOPPERS_STM32F769_DISCOVERY
	if(haudio->InputDevice == INPUT_DEVICE_DIGITAL_MIC){
		hAudioIn.pRecBuf = pbuf;
		hAudioIn.RecSize = size;
		/* Reset Application Buffer Trigger */
		AppBuffTrigger = 0;
		AppBuffHalf = 0;

		if(AudioIn_ChannelNumber > 2){
			/* Call the Media layer start function for buttom right channel */
			if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInButtomRightFilter, pScratchBuff[2], ScratchSize))){
				return ercd;
			}
			/* Call the Media layer start function for buttom left channel */
			if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInButtomLeftFilter, pScratchBuff[3], ScratchSize))){
				return ercd;
			}
		}
		/* Call the Media layer start function for top right channel */
		if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInTopRightFilter, pScratchBuff[0], ScratchSize))){
			return ercd;
		}
		/* Call the Media layer start function for top left channel */
		if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInTopLeftFilter, pScratchBuff[1], ScratchSize))){
			return ercd;
		}
		return ercd;
	}
#endif
	/* Start the process receive DMA */
	hsai->recevcallback = (void (*)(AUDIO_Handle_t *hsai))haudio->audio_fullin_func;
	hsai->recevhalfcallback = (void (*)(AUDIO_Handle_t *hsai))haudio->audio_halfin_func;
	hsai->errorcallback = haudio->audio_error_func;
	ercd = audio_receive(hsai, (uint8_t*)pbuf, size);
	return ercd;
}

/*
 *  オーディオ録音停止設定
 *  param1: STAUDIOハンドラ
 *  param2: 終了オプション
 *  return: E_OKで正常設定
 */
ER
staudio_in_stop(STAUDIO_Handle_t *haudio, uint32_t Option)
{
#ifdef TOPPERS_STM32F769_DISCOVERY
	ER ercd = E_OK;
	if(haudio->InputDevice == INPUT_DEVICE_DIGITAL_MIC){
		AppBuffTrigger = 0;
		AppBuffHalf    = 0;
		if(AudioIn_ChannelNumber > 2){
			/* Call the Media layer stop function for buttom right channel */
			if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInButtomRightFilter))){
				return ercd;
			}
			/* Call the Media layer stop function for buttom left channel */
			if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInButtomLeftFilter))){
				return ercd;
			}
		}
		/* Call the Media layer stop function for top right channel */
		if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInTopRightFilter))){
			return ercd;
		}
		/* Call the Media layer stop function for top left channel */
		if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInTopLeftFilter))){
			return ercd;
		}
		return ercd;
	}
#endif
	/* Call the Media layer stop function */
	audio_dmastop(haudio->hsai, AUDIO_IN_BLOCK);

	/* Call Audio Codec Stop function */
	if(wm8994_Stop(haudio, Option) != 0){
		return E_SYS;
	}
	else{
		if(Option == CODEC_PDWN_HW){
			/* Wait at least 100us */
			dly_tsk(1);
		}
		/* Return AUDIO_OK when all operations are correctly done */
		return E_OK;
	}
}

/*
 *  オーディオ録音PAUSE設定
 *  param1: STAUDIOハンドラ
 *  return: E_OKで正常設定
 */
ER
staudio_in_pause(STAUDIO_Handle_t *haudio)
{
#ifdef TOPPERS_STM32F769_DISCOVERY
	ER ercd = E_OK;
	if(haudio->InputDevice == INPUT_DEVICE_DIGITAL_MIC){
		if(AudioIn_ChannelNumber > 2){
			/* Call the Media layer stop function */
			if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInButtomRightFilter))){
				return ercd;
			}
			/* Call the Media layer stop function */
			if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInButtomLeftFilter))){
				return ercd;
			}
		}
		/* Call the Media layer stop function */
		if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInTopRightFilter))){
			return ercd;
		}
		/* Call the Media layer stop function */
		if(E_OK != (ercd = dfsdm_filterRegularStop(&hAudioInTopLeftFilter))){
			return ercd;
		}
		/* Return AUDIO_OK when all operations are correctly done */
		return ercd;
	}
#endif
	/* Call the Media layer pause function */
	return audio_dmapause(haudio->hsai, AUDIO_IN_BLOCK);
}

/*
 *  オーディオ録音RESUME設定
 *  param1: STAUDIOハンドラ
 *  return: E_OKで正常設定
 */
ER
staudio_in_resume(STAUDIO_Handle_t *haudio)
{
#ifdef TOPPERS_STM32F769_DISCOVERY
	ER ercd = E_OK;
	if(haudio->InputDevice == INPUT_DEVICE_DIGITAL_MIC){
		if(AudioIn_ChannelNumber > 2){
			/* Call the Media layer start function for buttom right channel */
			if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInButtomRightFilter, pScratchBuff[2], ScratchSize))){
				return ercd;
			}
			/* Call the Media layer start function for buttom left channel */
			if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInButtomLeftFilter, pScratchBuff[3], ScratchSize))){
				return ercd;
			}
		}
		/* Call the Media layer start function for top right channel */
		if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInTopRightFilter, pScratchBuff[0], ScratchSize))){
			return ercd;
		}
		/* Call the Media layer start function for top left channel */
		if(E_OK != (ercd = dfsdm_filterRegularStart(&hAudioInTopLeftFilter, pScratchBuff[1], ScratchSize))){
			return ercd;
		}
		/* Return AUDIO_OK when all operations are correctly done */
		return ercd;
	}
#endif
	/* Call the Media layer pause/resume function */
	return audio_dmaresume(haudio->hsai, AUDIO_IN_BLOCK);
}

/*
 *  オーディオ録音終了設定
 *  param1: STAUDIOハンドラ
 */
void
staudio_in_deinit(STAUDIO_Handle_t *haudio)
{
#ifdef TOPPERS_STM32F769_DISCOVERY
	if(haudio->InputDevice == INPUT_DEVICE_DIGITAL_MIC){
		/* MSP filters initialization */
		DFSDMx_FilterMspDeInit();
		DFSDMx_DeInit();
	}
	else
#endif
	SAIx_In_DeInit(haudio);
}

#ifndef TOPPERS_STM32F769_DISCOVERY
/*
 *  オーディオ録音ボリューム設定
 *  param1: STAUDIOハンドラ
 *  param2: ボリューム値(0..100)
 *  return: E_OKで正常設定
 */
ER
staudio_in_setvolume(STAUDIO_Handle_t *haudio, uint8_t Volume)
{
	/* Call the codec volume control function with converted volume value */
	if(wm8994_SetVolume(haudio, Volume) != 0){
		return E_SYS;
	}
	else{
		/* Set the Global variable AudioInVolume  */
		haudio->AudioInVolume = Volume;
		/* Return AUDIO_OK when all operations are correctly done */
		return E_OK;
	}
}


#else	/* TOPPERS_STM32F769_DISCOVERY  */
/*
 *  オーディオ録音スクラッチ領域E設定
 *  param1: スクラッチ領域へのポインタ
 *  param2: スクラッチ領域サイズ
 *  return: E_OKで正常設定
 */
ER
BSP_AUDIO_IN_AllocScratch(int32_t *pScratch, uint32_t size)
{
	uint32_t idx;

	ScratchSize = (size / AudioIn_ChannelNumber);

	/* copy scratch pointers */
	for (idx = 0; idx < AudioIn_ChannelNumber; idx++){
		pScratchBuff[idx] = (int32_t *)(pScratch + (idx * ScratchSize));
	}
	/* Return AUDIO_OK */
	return E_OK;
}

/*
 *  オーディオチャネル番号取り出し
 *  return: チャネル番号
 */
uint8_t BSP_AUDIO_IN_GetChannelNumber(void)
{
  return AudioIn_ChannelNumber;
}

#endif

