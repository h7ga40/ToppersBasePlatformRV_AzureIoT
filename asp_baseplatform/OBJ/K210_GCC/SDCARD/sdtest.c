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
 *  SD-CARDテストの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "device.h"
#include "spi.h"
#include "spi_driver.h"
#ifdef SDEV_SENSE_ONETIME
#include "storagedevice.h"
#include "sddiskio.h"
#endif
#include "sdtest.h"

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


static uint32_t heap_area[16*1024];

intptr_t heap_param[2] = {
	(intptr_t)heap_area,
	(4*16*1024)
};

uint8_t  sdbuffer[512];

/*
 *  SW1割込み
 */
void sw_int(int arg)
{
	syslog_1(LOG_NOTICE, "## sw_int(%d) ##", arg);
}

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

void
digitalWrite(uint8_t Pin, int dwVal){
    int8_t gpio_pin = gpio_get_gpiohno(Pin, false);

    if( gpio_pin >= 0){
        gpio_set_pin(TADR_GPIOHS_BASE, (uint8_t)gpio_pin, dwVal);
    }
}


/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
#ifdef SDEV_SENSE_ONETIME
	StorageDevice_t *psdev;
#endif
	SPI_Init_t Init;
	SPI_Handle_t *hspi;
	SDCARD_Handler_t *hsd = NULL;
#ifndef SDEV_SENSE_ONETIME
	SDCARD_CardInfo_t CardInfo;
#endif
	ER_UINT	ercd;
	int i;

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

	dly_tsk(100);
	syslog_0(LOG_NOTICE, "SPI SEND/RECV START");
	pinMode(SPI_SS_PIN, OUTPUT);
	digitalWrite(SPI_SS_PIN, HIGH);
#if 1	/* ROI DEBUG */
	dly_tsk(100);
#endif	/* ROI DEBUG */

	/*##-1- Configure the SPI peripheral #######################################*/
	/* Set the SPI parameters */
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
#ifdef USE_TXDMA*/
	Init.TxDMAChannel = SPI_DMA2_CH;
#else
	Init.TxDMAChannel = -1;
#endif
	Init.RxDMAChannel = SPI_DMA1_CH;
	Init.semid        = SPITRN_SEM;
	Init.semlock      = SPILOCK_SEM;
	Init.semdmaid     = SPIDMATX_SEM;
	hspi = spi_init(SPI_PORTID, &Init);
	if(hspi == NULL){
		syslog_0(LOG_ERROR, "SPI INIT ERROR");
		slp_tsk();
	}
	sdcard_setspi2(SPISDCARD_PORTID, hspi, SPI_SS_PIN);

	/*
	 *  SD-CARD SPI通信設定
	 */
	for(i = 0 ; i < 10 ; i++)
		sdbuffer[i] = 0xff;
	if((ercd = spi_transmit(hspi, (uint8_t*)sdbuffer, 10)) != E_OK){
		/* Transfer error in transmission process */
		syslog_1(LOG_NOTICE, "## call Error_Handler(2)(%d) ##", ercd);
	}
#if SPI_WAIT_TIME == 0
	if((ercd = spi_wait(hspi, 100)) != E_OK){
		syslog_0(LOG_NOTICE, "## call Error_Handler(3) ##");
	}
#endif
	dly_tsk(100);

#ifndef SDEV_SENSE_ONETIME
	if(sdcard_init(SPISDCARD_PORTID)){
		hsd = sdcard_open(SPISDCARD_PORTID);
		syslog_1(LOG_NOTICE, "## card(%d) ##", hsd->cardtype);
		SVC_PERROR(sdcard_checkCID(hsd));
		SVC_PERROR(sdcard_sendCSD(hsd));
		SVC_PERROR(sdcard_getcardinfo(hsd, &CardInfo));
		SVC_PERROR(sdcard_configuration(hsd));
	}
#else
	SDMSence_task(0);
	psdev = SDMGetStorageDevice(SDCARD_DEVNO);
	hsd = (SDCARD_Handler_t *)psdev->_sdev_local[1];
#endif
	if(hsd == NULL){
		syslog_0(LOG_NOTICE, "SD-CARD INITIAL ERROR !");
		slp_tsk();
	}
#ifndef SDEV_SENSE_ONETIME
	if(hsd != NULL && hsd->hspi != NULL){
		unsigned char *p;
		int j, bpb;
		int SecPerCls, Rsv, NumFat, Fsize, RootDirC, RootSec;
		for(i = 0 ; i < 1 ; i++){
			ercd = sdcard_blockread(hsd, sdbuffer, i*512, 512, 1);
			syslog_3(LOG_NOTICE, "## ercd(%d) i(%d) sdbuffer[%08x] ##", ercd, i, sdbuffer);
			dly_tsk(300);
			p = (unsigned char *)sdbuffer;
			if(i == 0){
				if(p[0x36] == 'F' && p[0x37] == 'A' && p[0x38] == 'T')
					bpb = 0;
				else{
					bpb = p[0x1c6];
					bpb |= p[0x1c7]<<8;
					bpb |= p[0x1c8]<<16;
					bpb |= p[0x1c9]<<24;
				}
			}
			printf("\nsec(%d) ", i);
			for(j = 0 ; j < 512 ; j++){
				if((j % 16) == 0)
					printf("\n%03x ", j);
				printf("%02x ", p[j]);
			}
			printf("\n");
		}
		syslog_1(LOG_NOTICE, "## bpb(%08x) ##", bpb);
		if(bpb != 0){
			ercd = sdcard_blockread(hsd, sdbuffer, bpb*512, 512, 1);
			syslog_3(LOG_NOTICE, "## ercd(%d) i(%d) sdbuffer[%08x] ##", ercd, bpb, sdbuffer);
			dly_tsk(300);
			printf("\nsec(%d) ", bpb);
			for(j = 0 ; j < 512 ; j++){
				if((j % 16) == 0)
					printf("\n%03x ", j);
				printf("%02x ", p[j]);
			}
			printf("\n");
		}
		p = (unsigned char *)sdbuffer;
		SecPerCls = p[13];
		Rsv = p[14] + (p[15] << 8);
		NumFat = p[16];
		RootDirC = p[17] + (p[18]<<8);
		Fsize = p[22] + (p[23] << 8);
		syslog_4(LOG_NOTICE, "## SecPerCls(%d) Rsv(%d) NumFat(%d) RootDirC(%d) ##", SecPerCls, Rsv, NumFat, RootDirC);
		RootSec = bpb + Rsv + (NumFat * Fsize);
		syslog_2(LOG_NOTICE, "## Fsize(%d) RootSec(%d) ##", Fsize, RootSec);

		for(i = 0 ; i < 2 ; i++){
			ercd = sdcard_blockread(hsd, sdbuffer, (i+RootSec)*512, 512, 1);
			syslog_3(LOG_NOTICE, "## ercd(%d) i(%d) sdbuffer[%08x] ##", ercd, (i+RootSec), sdbuffer);
			dly_tsk(300);
			p = (unsigned char *)sdbuffer;
			printf("\nsec(%d) ", (i+RootSec));
			for(j = 0 ; j < 512 ; j++){
				if((j % 16) == 0)
					printf("\n%03x ", j);
				printf("%02x ", p[j]);
			}
			printf("\n");
		}
	}
#endif
	(void)(hsd);

	syslog(LOG_NOTICE, "Sample program ends.");
	slp_tsk();
//	SVC_PERROR(ext_ker());
}
