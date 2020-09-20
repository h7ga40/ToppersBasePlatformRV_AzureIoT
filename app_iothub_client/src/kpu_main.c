/*
*  TOPPERS/ASP Kernel
*      Toyohashi Open Platform for Embedded Real-Time Systems/
*      Advanced Standard Profile Kernel
* 
*  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
*                              Toyohashi Univ. of Technology, JAPAN
*  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
*              Graduate School of Information Science, Nagoya Univ., JAPAN
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
*  $Id: kpu_main.c 2176 2020-08-19 23:50:05Z coas-nagasima $
*/

#include <kernel.h>
#include <stdlib.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include "pinmode.h"
#include "sipeed_st7789.h"
#include "sipeed_ov2640.h"
#include "storagedevice.h"
#ifdef SDEV_SENSE_ONETIME
#include "spi_driver.h"
#include "sddiskio.h"
#else
#include "rom_file.h"
#endif
#include "kernel_cfg.h"
#include "kpu_main.h"
#include "kpu.h"
#include "region_layer.h"

/*
*  サービスコールのエラーのログ出力
*/
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

#define SWAP_32(x) ((x >> 24) | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) | (x << 24))

#ifndef SPI1DMATX_SEM
#define SPI1DMATX_SEM   0
#endif

#define CLASS_NUMBER 20

LCD_Handler_t  LcdHandle;
LCD_DrawProp_t DrawProp;
DVP_Handle_t   DvpHandle;
OV2640_t       CameraHandle;

uint8_t  sTxBuffer[16];

static uint8_t time_string[12];

static void
set_value(uint8_t *buf, int value)
{
	buf[1] = (value % 10) + '0';
	buf[0] = (value / 10) + '0';
}

static void
set_time(uint8_t *buf, struct tm2 *tm)
{
	buf[8] = 0;
	set_value(&buf[6], tm->tm_sec);
	buf[5] = ':';
	set_value(&buf[3], tm->tm_min);
	buf[2] = ':';
	set_value(&buf[0], tm->tm_hour);
}

/*
*  ダイレクトデジタルピン設定
*/
void
pinMode(uint8_t Pin, uint8_t dwMode){ 
	int gpionum = gpio_get_gpiohno(Pin, false);
	GPIO_Init_t init = {0};

	syslog_2(LOG_NOTICE, "## pinMode Pin(%d) gpionum(%d) ##", Pin, gpionum);
	if(gpionum >= 0){
		uint8_t function = FUNC_GPIOHS0 + gpionum;
		fpioa_set_function(Pin, function);
		switch(dwMode){
		case INPUT:
			init.mode = GPIO_MODE_INPUT;
			init.pull = GPIO_NOPULL;
			break;
		case INPUT_PULLDOWN:
			init.mode = GPIO_MODE_INPUT;
			init.pull = GPIO_PULLDOWN;
			break;
		case INPUT_PULLUP:
			init.mode = GPIO_MODE_INPUT;
			init.pull = GPIO_PULLUP;
			break;
		case OUTPUT:
		default:
			init.mode = GPIO_MODE_OUTPUT;
			init.pull = GPIO_PULLDOWN;
			break;
		}
		gpio_setup(TADR_GPIOHS_BASE, &init, (uint8_t)gpionum);
	}
	return ;
}

/*
 *  ダイレクトデジタルピン出力
 */
void
digitalWrite(uint8_t Pin, int dwVal){
    int8_t gpio_pin = gpio_get_gpiohno(Pin, false);

    if( gpio_pin >= 0){
        gpio_set_pin(TADR_GPIOHS_BASE, (uint8_t)gpio_pin, dwVal);
    }
}

#if (CLASS_NUMBER > 1)
typedef struct
{
	char *str;
	uint16_t color;
	uint16_t height;
	uint16_t width;
} class_lable_t;

class_lable_t class_lable[CLASS_NUMBER] =
{
	{"aeroplane", ST7789_GREEN},
	{"bicycle", ST7789_GREEN},
	{"bird", ST7789_GREEN},
	{"boat", ST7789_GREEN},
	{"bottle", 0xF81F},
	{"bus", ST7789_GREEN},
	{"car", ST7789_GREEN},
	{"cat", ST7789_GREEN},
	{"chair", 0xFD20},
	{"cow", ST7789_GREEN},
	{"diningtable", ST7789_GREEN},
	{"dog", ST7789_GREEN},
	{"horse", ST7789_GREEN},
	{"motorbike", ST7789_GREEN},
	{"person", 0xF800},
	{"pottedplant", ST7789_GREEN},
	{"sheep", ST7789_GREEN},
	{"sofa", ST7789_GREEN},
	{"train", ST7789_GREEN},
	{"tvmonitor", 0xF9B6}
};

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];
#endif
extern const uint8_t model_data[];
kpu_model_context_t g_task;
static region_layer_t detect_rl;
yolo_result_t yolo_result;
yolo_result_t yolo_result_frame;

volatile uint8_t g_ai_done_flag;

void ai_done(void *ctx)
{
	g_ai_done_flag = 2;
}

#define ANCHOR_NUM 5
float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};

static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class, float prob)
{
	if (x1 >= 320)
		x1 = 319;
	if (x2 >= 320)
		x2 = 319;
	if (y1 >= 240)
		y1 = 239;
	if (y2 >= 240)
		y2 = 239;

#if (CLASS_NUMBER > 1)
	DrawProp.TextColor = class_lable[class].color;
	lcd_drawRect(&DrawProp, x1, y1, x2-x1, y2-y1);
	DrawProp.TextColor = ST7789_WHITE;
	DrawProp.BackColor = class_lable[class].color;
	DrawProp.pFont = &Font12;
	lcd_DisplayStringAt(&DrawProp, x1 + 1, y1 + 1, class_lable[class].str, LEFT_MODE);
#else
	lcd_drawRect(&DrawProp, x1, y1, x2, y2, 2, ST7789_RED);
#endif
	if (strcmp(class_lable[class].str, "person") == 0)
		yolo_result_frame.person++; 
	if (strcmp(class_lable[class].str, "car") == 0)
		yolo_result_frame.car++; 
}

/*
*  メインタスク
*/
void kpu_task(intptr_t exinf)
{
	SPI_Init_t Init;
	SPI_Handle_t    *hspi;
	LCD_Handler_t   *hlcd;
	OV2640_t        *hcmr;
	DVP_Handle_t    *hdvp;
	ER_UINT	ercd;
	uint32_t i;
	struct tm2 time;
	unsigned long atmp;
#ifdef SDEV_SENSE_ONETIME
	StorageDevice_t *psdev;
	SDCARD_Handler_t *hsd = NULL;
#else
	uint32_t esize, fsize, headlen;
	uint8_t  *addr;
#endif

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	pinMode(LED_G_PIN, OUTPUT);
	digitalWrite(LED_G_PIN, HIGH);
	pinMode(LED_R_PIN, OUTPUT);
	digitalWrite(LED_R_PIN, HIGH);
	pinMode(LED_B_PIN, OUTPUT);
	digitalWrite(LED_B_PIN, HIGH);

	select_spi0_dvp_mode(1);

#ifdef SDEV_SENSE_ONETIME
	pinMode(SPI_SS_PIN, OUTPUT);
	digitalWrite(SPI_SS_PIN, HIGH);
#else	/* USE ROMFILE */
	rom_file_init();
	esize = 1*1024*1024;
	addr  = (uint8_t *)romfont;
	while(esize > 4){
		fsize = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | addr[3];
		if((fsize & 3) != 0 || fsize < 16 || fsize > esize)
			break;
		for(i = 0 ; i < 28 ; i++){
			if(addr[i+4] == 0)
				break;
		}
		if(i >= 28)
			break;
		if(i < 12)
			headlen = 16;
		else
			headlen = 32;
		if(create_rom_file((const char *)(addr+4), (uintptr_t)(addr+headlen), fsize-headlen) < 0)
			break;
		addr += fsize;
		esize -= fsize;
	}
#endif

	hcmr = &CameraHandle;
	hcmr->frameSize = FRAMESIZE_QVGA;
	hcmr->pixFormat = PIXFORMAT_RGB565;
	ov2640_getResolition(hcmr, FRAMESIZE_QVGA);
	hcmr->_resetPoliraty  = ACTIVE_HIGH;
	hcmr->_pwdnPoliraty   = ACTIVE_HIGH;
	hcmr->_slaveAddr      = 0x00;
	hcmr->_dataBuffer     = NULL;
	hcmr->_aiBuffer       = NULL;

	// just support RGB565 and YUV442 on k210

	// Initialize the camera bus, 8bit reg
	hdvp = &DvpHandle;
	hcmr->hdvp = hdvp;
	hdvp->Init.Freq         = 24000000;
	hdvp->Init.Width        = hcmr->_width;
	hdvp->Init.Height       = hcmr->_height;
	hdvp->Init.Format       = DVP_FORMAT_RGB;
	hdvp->Init.BurstMode    = DVP_BURST_ENABLE;
	hdvp->Init.AutoMode     = DVP_AUTOMODE_DISABLE;
	hdvp->Init.GMMlen       = 4;

	hdvp->Init.num_sccb_reg = 8;
	hdvp->Init.CMosPClkPin  = 47;
	hdvp->Init.CMosXClkPin  = 46;
	hdvp->Init.CMosHRefPin  = 45;
	hdvp->Init.CMosPwDnPin  = 44;
	hdvp->Init.CMosVSyncPin = 43;
	hdvp->Init.CMosRstPin   = 42;
	hdvp->Init.SccbSClkPin  = 41;
	hdvp->Init.SccbSdaPin   = 40;
	hdvp->Init.IntNo        = INTNO_DVP;
	syslog_1(LOG_NOTICE, "## DvpHandle[%08x] ##", &DvpHandle);

	syslog_3(LOG_NOTICE, "## hcmr->_width(%d) hcmr->_height(%d) size[%08x] ##", hcmr->_width, hcmr->_height, (hcmr->_width * hcmr->_height * 2));
	atmp = (unsigned long)malloc(hcmr->_width * hcmr->_height * 2 + 64); //RGB565
	hcmr->_dataBuffer = (uint32_t *)((atmp + 63) & ~63);
	if(hcmr->_dataBuffer == NULL){
		hcmr->_width = 0;
		hcmr->_height = 0;
		syslog_0(LOG_ERROR, "Can't allocate _dataBuffer !");
		slp_tsk();
	}
	atmp = (unsigned long)malloc(hcmr->_width * hcmr->_height * 3 + 128);   //RGB888
	hcmr->_aiBuffer = (uint32_t *)((atmp + 127) & ~127);
	if(hcmr->_aiBuffer == NULL){
		hcmr->_width = 0;
		hcmr->_height = 0;
		free(hcmr->_dataBuffer);
		hcmr->_dataBuffer = NULL;
		syslog_0(LOG_ERROR, "Can't allocate _aiBuffer !");
		slp_tsk();
	}
	syslog_2(LOG_NOTICE, "## hcmr->_dataBuffer[%08x] hcmr->_aiBuffer[%08x] ##", hcmr->_dataBuffer, hcmr->_aiBuffer);
	atmp = (unsigned long)hcmr->_aiBuffer - IOMEM;
	hdvp->Init.RedAddr    = (uint32_t)atmp;
	atmp = (unsigned long)hcmr->_aiBuffer + hcmr->_width * hcmr->_height - IOMEM;
	hdvp->Init.GreenAddr  = (uint32_t)atmp;
	atmp = (unsigned long)hcmr->_aiBuffer + hcmr->_width * hcmr->_height * 2 - IOMEM;
	hdvp->Init.BlueAddr   = (uint32_t)atmp;
	atmp = (unsigned long)hcmr->_dataBuffer - IOMEM;
	hdvp->Init.RGBAddr    = (uint32_t)atmp;
	dvp_init(hdvp);

	if(ov2640_sensor_ov_detect(hcmr) == E_OK){
		syslog_0(LOG_NOTICE, "find ov sensor !");
	}
	else if(ov2640_sensro_gc_detect(hcmr) == E_OK){
		syslog_0(LOG_NOTICE, "find gc3028 !");
	}
	if(ov2640_reset(hcmr) != E_OK){
		syslog_0(LOG_ERROR, "ov2640 reset error !");
		slp_tsk();
	}
	if(ov2640_set_pixformat(hcmr) != E_OK){
		syslog_0(LOG_ERROR, "set pixformat error !");
		slp_tsk();
	}
	if(ov2640_set_framesize(hcmr) != E_OK){
		syslog_0(LOG_ERROR, "set frame size error !");
		slp_tsk();
	}
	if(ov2640_setInvert(hcmr, false) != E_OK){
		syslog_0(LOG_ERROR, "set invert error !");
		slp_tsk();
	}
	syslog_1(LOG_NOTICE, "OV2640 id(%d)", ov2640_id(hcmr));

	Init.WorkMode     = SPI_WORK_MODE_2;
	Init.FrameFormat  = SPI_FF_OCTAL;
	Init.DataSize     = 8;
	Init.Prescaler    = 15000000;
	Init.SignBit      = 0;
	Init.InstLength   = 8;
	Init.AddrLength   = 0;
	Init.WaitCycles   = 0;
	Init.IATransMode  = SPI_AITM_AS_FRAME_FORMAT;
	Init.SclkPin      = SIPEED_ST7789_SCLK_PIN;
	Init.MosiPin      = -1;
	Init.MisoPin      = -1;
	Init.SsPin        = SIPEED_ST7789_SS_PIN;
	Init.SsNo         = SIPEED_ST7789_SS;
	Init.TxDMAChannel = -1;
	Init.RxDMAChannel = -1;
	Init.semid        = SPI1TRN_SEM;
	Init.semlock      = SPI1LOCK_SEM;
	Init.semdmaid     = SPI1DMATX_SEM;
	hspi = spi_init(SPI_PORTID, &Init);
	if(hspi == NULL){
		syslog_0(LOG_ERROR, "SPI INIT ERROR");
		slp_tsk();
	}

	hlcd = &LcdHandle;
	hlcd->hspi    = hspi;
//	hlcd->spi_lock= SPI1LOCK_SEM;
	hlcd->dir     = DIR_YX_LRUD;
	hlcd->dcx_pin = SIPEED_ST7789_DCX_PIN;
	hlcd->rst_pin = SIPEED_ST7789_RST_PIN;
	hlcd->cs_sel  = SIPEED_ST7789_SS;
	hlcd->rst_no  = SIPEED_ST7789_RST_GPIONUM;
	hlcd->dcx_no  = SIPEED_ST7789_DCX_GPIONUM;
	DrawProp.hlcd = hlcd;
	lcd_init(hlcd);
	syslog_2(LOG_NOTICE, "width(%d) height(%d)", hlcd->_width, hlcd->_height);

	DrawProp.BackColor = ST7789_WHITE;
	DrawProp.TextColor = ST7789_BLACK;
	lcd_fillScreen(&DrawProp);

#ifdef SDEV_SENSE_ONETIME
	Init.WorkMode     = SPI_WORK_MODE_0;
	Init.FrameFormat  = SPI_FF_STANDARD;
	Init.DataSize     = 8;
	Init.Prescaler    = 5000000;
	Init.SignBit      = 0;
	Init.InstLength   = 0;
	Init.AddrLength   = 0;
	Init.WaitCycles   = 0;
	Init.IATransMode  = SPI_AITM_STANDARD;
	Init.SclkPin      = SPI_SCK_PIN;
	Init.MosiPin      = SPI_MOSI_PIN;
	Init.MisoPin      = SPI_MISO_PIN;
	Init.SsPin        = -1;
	Init.SsNo         = -1;
	Init.TxDMAChannel = -1;
	Init.RxDMAChannel = SPI_DMA1_CH;
	Init.semid        = SPI2TRN_SEM;
	Init.semlock      = SPI2LOCK_SEM;
	Init.semdmaid     = SPI2DMATX_SEM;
	hspi = spi_init(SPICARD_PORTID, &Init);
	if(hspi == NULL){
		syslog_0(LOG_ERROR, "SPI-CARD INIT ERROR");
		slp_tsk();
	}
	sdcard_setspi2(SPISDCARD_PORTID, hspi, SPI_SS_PIN);

	/*
	*  SD-CARD SPI通信設定
	*/
	for(i = 0 ; i < 10 ; i++)
		sTxBuffer[i] = 0xff;
	if((ercd = spi_core_transmit(hspi, -1, (uint8_t*)sTxBuffer, 10)) != E_OK){
		/* Transfer error in transmission process */
		syslog_1(LOG_NOTICE, "## call Error_Handler(2)(%d) ##", ercd);
	}
#if SPI_WAIT_TIME == 0
	if((ercd = spi_wait(hspi, 100)) != E_OK){
		syslog_0(LOG_NOTICE, "## call Error_Handler(3) ##");
	}
#endif
	dly_tsk(100);
	SDMSence_task(0);
	psdev = SDMGetStorageDevice(SDCARD_DEVNO);
	hsd = (SDCARD_Handler_t *)psdev->_sdev_local[1];
	if(hsd == NULL)
		syslog_0(LOG_ERROR, "SD-CARD INITAIL ERROR !");
#endif
	if ((ercd = kpu_init(&g_task)) != E_OK) {
		syslog_0(LOG_ERROR, "kpu init error");
		slp_tsk();
	}

	if (kpu_load_kmodel(&g_task, model_data) != 0) {
		syslog_0(LOG_ERROR, "kmodel init error");
		slp_tsk();
	}

	detect_rl.anchor_number = ANCHOR_NUM;
	detect_rl.anchor = g_anchor;
	detect_rl.threshold = 0.7;
	detect_rl.nms_value = 0.3;
	if (region_layer_init(&detect_rl, 10, 7, 125, 320, 240) != 0) {
		syslog_0(LOG_ERROR, "region layer init error");
		slp_tsk();
	}

	bool_t camok = true;
	if((ercd = ov2640_activate(hcmr, true)) != E_OK){
		syslog_2(LOG_NOTICE, "ov2640 activate error result(%d) id(%d) ##", ercd, ov2640_id(hcmr));
		camok = false;
	}

	int tmo = 0;
	g_ai_done_flag = 0;
	while (camok) {
		if (g_ai_done_flag == 0) {
			ercd = ov2640_snapshot(hcmr);
			if (ercd != E_OK) {
				camok = false;
				continue;
			}
			g_ai_done_flag = 1;

			/* start to calculate */
			atmp = (unsigned long)hcmr->_aiBuffer - IOMEM;
			kpu_run_kmodel(&g_task, (const uint8_t *)atmp, AI_DMA_CH, ai_done, NULL);
		}

		if (g_ai_done_flag == 1) {
			tmo++;
			if (tmo >= 10) {
				tmo = 0;
				g_ai_done_flag = 0;
			}
			else {
				dly_tsk(1);
			}
			continue;
		}
		else if (g_ai_done_flag == 2) {
			g_ai_done_flag = 0;
			float *output;
			size_t output_size;
			kpu_get_output(&g_task, 0, &output, &output_size);
			detect_rl.input = output;

			/* start region layer */
			region_layer_run(&detect_rl, NULL);
		}

		lcd_drawPicture(hlcd, 0, 0, hcmr->_width, hcmr->_height, (uint16_t *)hcmr->_dataBuffer);

		yolo_result_frame.person = 0;
		yolo_result_frame.car = 0;

		/* draw boxs */
		region_layer_draw_boxes(&detect_rl, drawboxes);

		if (yolo_result.reset) {
			yolo_result.reset = 0;
			yolo_result.person = 0;
			yolo_result.car = 0;
		}
		if (yolo_result.person < yolo_result_frame.person)
			yolo_result.person = yolo_result_frame.person;
		if (yolo_result.car < yolo_result_frame.car)
			yolo_result.car = yolo_result_frame.car;
	}
	ov2640_activate(hcmr, false);

	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
	syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ext_ker());
	assert(0);
}
