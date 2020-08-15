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
 *  $Id$
 */

/* 
 *  K210 LEDデモプログラム
 *  このデモプログラムは、デフォルトでSDEV_SENSE_ONETIMEコンパイルが
 *  有効となっておりSDカード中に東雲漢字フォントが書き込まれて
 *  いることを前提としています。
 *  TOPPERS BASE PLATFORM(CV)中のui/snfont_dispディレクトリ中の
 *  以下のフォントをSDカードに書き込んで起動してください。
 *    shinonome_font12.fnt
 *    shinonome_font16.fnt
 *  プログラム中に漢字フォントをROMファイルして取り込んで漢字の表示も
 *  可能です。この場合、SDEV_SENSE_ONETIMEコンパイルスイッチを無効にし
 *  USE_ROMFONTコンパイルスイッチを有効にして、Makefile中に
 *  gdic/rom_file/Makefile.configを取り込んでビルドしてください。
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
#include "demo.h"
#include "topamelogo.h"

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

#define SWAP_16(x) ((x >> 8 & 0xff) | (x << 8))

#ifndef SPI1DMATX_SEM
#define SPI1DMATX_SEM   0
#endif

#if !defined(NOT_USEFILFONT)
static const char string1[] = {
	'T', 'O', 'P', 'P', 'E', 'R', 'S',0xE3,0x83,0x97,0xE3,0x83,0xAD,0xE3,0x82,0xB8,
	0xE3,0x82,0xA7,0xE3,0x82,0xAF,0xE3,0x83,0x88,0
};
static const char string2[] = {
	0xE6,0x95,0x99,0xE8,0x82,0xB2,0xE3,0x83,0xAF,0xE3,0x83,0xBC,0xE3,0x82,0xAD,
	0xE3,0x83,0xB3,0xE3,0x82,0xB0,0xE3,0x83,0xAB,0xE3,0x83,0xBC,0xE3,0x83,0x97,0
};
#else
static const char string1[] = "TOPPERS PROJECT";
static const char string2[] = "Educational Working Group.";
#endif

static uint32_t heap_area[256*1024];

intptr_t heap_param[2] = {
	(intptr_t)heap_area,
	sizeof(heap_area)
};

/*
 *  使用する変数の定義
 */
static const Point testPolygon[7] = {
	{0, 0},
	{319, 239},
	{160, 0},
	{160, 239},
	{319, 0},
	{0, 239},
	{0, 0}
};

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

/*
 *  周期ハンドラ
 */
void cyclic_handler(intptr_t exinf)
{
	static bool_t b = 0;
	b ^= 1;

	digitalWrite(LED_PIN, b);
}

/*
 *  グラフィック表示テスト
 */
void
grapics_test(LCD_Handler_t *hlcd)
{
	uint16_t x, y;
	int      i;

	DrawProp.BackColor = ST7789_BLACK;
	lcd_fillScreen(&DrawProp);
	dly_tsk(1000);
	lcd_drawPixel(hlcd, hlcd->_width/2, hlcd->_height/2, ST7789_GREEN);
	dly_tsk(1000);

	lcd_fillScreen(&DrawProp);
	for(y=0; y < hlcd->_height; y+=5) {
		lcd_drawFastHLine(hlcd, 0, y, hlcd->_width, ST7789_RED);
	}
	for (x=0; x < hlcd->_width; x+=5) {
    	lcd_drawFastVLine(hlcd, x, 0, hlcd->_height, ST7789_BLUE);
	}
	dly_tsk(1000);

	lcd_fillScreen(&DrawProp);
	DrawProp.TextColor = ST7789_GREEN;
	lcd_drawPolygon(&DrawProp, (Point *)testPolygon, sizeof(testPolygon)/sizeof(Point));
	dly_tsk(1000);

	lcd_fillScreen(&DrawProp);
	DrawProp.TextColor = ST7789_GREEN;
	for (y=0; y < hlcd->_height; y+=6) {
		lcd_drawRect(&DrawProp, hlcd->_width/2 -y/2, hlcd->_height/2 -y/2 , y, y);
	}
	for(i = 0 ; i < 3 ; i++){
		lcd_invertDisplay(hlcd, true);
		dly_tsk(500);
		lcd_invertDisplay(hlcd, false);
		dly_tsk(500);
	}
	lcd_fillScreen(&DrawProp);
	lcd_drawBitmap(hlcd, 33, 48, (uint8_t *)topamelogo);
	dly_tsk(1000);
}

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	SPI_Init_t Init;
	SPI_Handle_t    *hspi;
	LCD_Handler_t   *hlcd;
	OV2640_t        *hcmr;
	DVP_Handle_t    *hdvp;
	uint16_t        *lcd_buffer;
	ER_UINT	ercd;
	uint32_t i, count;
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

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	pinMode(LED_PIN, OUTPUT);
	SVC_PERROR(sta_cyc(CYCHDR1));

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
	hcmr->_dataBuffer = (uint32_t*)malloc(hcmr->_width * hcmr->_height * 2); //RGB565
	if(hcmr->_dataBuffer == NULL){
		hcmr->_width = 0;
		hcmr->_height = 0;
		syslog_0(LOG_ERROR, "Can't allocate _dataBuffer !");
		slp_tsk();
	}
	hcmr->_aiBuffer = (uint32_t*)malloc(hcmr->_width * hcmr->_height * 3 + 64 * 1024);   //RGB888
	if(hcmr->_aiBuffer == NULL){
		hcmr->_width = 0;
		hcmr->_height = 0;
		free(hcmr->_dataBuffer);
		hcmr->_dataBuffer = NULL;
		syslog_0(LOG_ERROR, "Can't allocate _aiBuffer !");
		slp_tsk();
	}
	syslog_2(LOG_NOTICE, "## hcmr->_dataBuffer[%08x] hcmr->_aiBuffer[%08x] ##", hcmr->_dataBuffer, hcmr->_aiBuffer);
	atmp = (unsigned long)hcmr->_aiBuffer;
	hdvp->Init.RedAddr    = (uint32_t)atmp;
	atmp = (unsigned long)(hcmr->_aiBuffer + hcmr->_width * hcmr->_height);
	hdvp->Init.GreenAddr  = (uint32_t)atmp;
	atmp = (unsigned long)(hcmr->_aiBuffer + hcmr->_width * hcmr->_height * 2);
	hdvp->Init.BlueAddr   = (uint32_t)atmp;
	atmp = (unsigned long)hcmr->_dataBuffer;
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
	syslog_1(LOG_NOTICE, "OV2640 id(%d)", ov2640_id(hcmr));

	Init.WorkMode     = SPI_WORK_MODE_0;
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
	Init.TxDMAChannel = SIPEED_DMA_CH;
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
	count = hcmr->_width * hcmr->_height;
	lcd_buffer = (uint16_t *)malloc(count * 2);
	if(lcd_buffer == NULL){
		syslog_0(LOG_ERROR, "no lcd buffer !");
		slp_tsk();
	}
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

	for(;;){
		DrawProp.BackColor = ST7789_WHITE;
		DrawProp.TextColor = ST7789_BLACK;
		lcd_fillScreen(&DrawProp);
		lcd_fillRect(hlcd, 0, 0, hlcd->_width, (hlcd->_height/3)*2-20, ST7789_BLUE);
		DrawProp.TextColor = ST7789_WHITE;
		DrawProp.BackColor = ST7789_BLUE;
		DrawProp.pFont = &Font16;
		lcd_DisplayStringAt(&DrawProp, 0, 16, (uint8_t *)string1, CENTER_MODE);
		lcd_DisplayStringAt(&DrawProp, 0, 40, (uint8_t *)string2, CENTER_MODE);
		lcd_DisplayStringAt(&DrawProp, 0, 64, (uint8_t *)"TOPPERS BASE PLATFORM(RV)", CENTER_MODE);

		DrawProp.TextColor = ST7789_GREEN;
		lcd_DisplayStringAt(&DrawProp, 0, 88, (uint8_t *)"TOPPERS/ASP Kernel", CENTER_MODE);
		DrawProp.TextColor = ST7789_RED;
		lcd_DisplayStringAt(&DrawProp, 0, 112, (uint8_t *)"RISC-V/64 K210", CENTER_MODE);
		lcd_drawBitmap(hlcd, hlcd->_width/2-30, (hlcd->_height/3)*2, (uint8_t *)topamelogo);
		dly_tsk(5000);

		DrawProp.BackColor = ST7789_BLACK;
		DrawProp.TextColor = ST7789_WHITE;
		grapics_test(hlcd);

		DrawProp.TextColor = ST7789_BLACK;
		DrawProp.BackColor = ST7789_WHITE;
		lcd_fillScreen(&DrawProp);
		lcd_fillRect(hlcd, 0, 0, hlcd->_width, hlcd->_height/3, ST7789_BLUE);
		DrawProp.TextColor = ST7789_WHITE;
		DrawProp.BackColor = ST7789_BLUE;
		DrawProp.pFont = &Font16;
		lcd_DisplayStringAt(&DrawProp, 0, 16, (uint8_t *)"TOPPERS BASE PLATFORM TEST", CENTER_MODE);

		DrawProp.TextColor = ST7789_BLACK;
		DrawProp.BackColor = ST7789_WHITE;
		DrawProp.pFont = &Font8;
		lcd_DisplayStringAt(&DrawProp, 0, 100, (uint8_t *)"Helle World !", CENTER_MODE);
		DrawProp.pFont = &Font12;
		lcd_DisplayStringAt(&DrawProp, 0, 120, (uint8_t *)"Helle World !", CENTER_MODE);
		DrawProp.pFont = &Font16;
		lcd_DisplayStringAt(&DrawProp, 0, 140, (uint8_t *)"Helle World !", CENTER_MODE);

		DrawProp.TextColor = ST7789_RED;
		for(i = 20 ; i < 80 ; i++){
			lcd_DrawCircle(&DrawProp, hlcd->_width/2, hlcd->_height/2+30, i);
			if(i == 25)
				i += 10;
		}
		for(i = 0 ; i < 30 ; i++){
			rtc_get_time(&time);
			set_time(time_string, &time);
			lcd_DisplayStringAt(&DrawProp, 0, 160, time_string, CENTER_MODE);
			dly_tsk(1000);
		}

		if((ercd = ov2640_activate(hcmr, true)) != E_OK){
			syslog_2(LOG_NOTICE, "ov2640 activate error result(%d) id(%d) ##", ercd, ov2640_id(hcmr));
			continue;
		}

		for(i = 0 ; i < 2000 ; i++){
			if((i % 100) == 0)
				syslog_1(LOG_NOTICE, "camera count(%d)", i);
			ercd = ov2640_snapshot(hcmr);
			if(ercd == E_OK){
				uint16_t *p = (uint16_t *)hcmr->_dataBuffer;
				uint32_t no;
			    for (no = 0; no < count ; no += 2){
					lcd_buffer[no]   = SWAP_16(*(p + 1));
					lcd_buffer[no+1] = SWAP_16(*(p));
					p += 2;
				}
				lcd_drawPicture(hlcd, 0, 0, hcmr->_width, hcmr->_height, lcd_buffer);
			}
		}
		ov2640_activate(hcmr, false);
	}
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
	syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ext_ker());
	assert(0);
}
