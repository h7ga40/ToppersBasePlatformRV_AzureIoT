/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2019 by TOPPERS PROJECT
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

/*
 *  Written by Limor Fried/Ladyada for Adafruit Industries.
 *  MIT license, all text above must be included in any redistribution
 *　http://opensource.org/licenses/mit-license.php 
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"
#include "pinmode.h"
#include "adafruitts_stmp610.h"

#define PORT_HIGH           1
#define PORT_LOW            0

#define TS_MINX             150
#define TS_MINY             130
#define TS_MAXX             3800
#define TS_MAXY             4000

#define cs_set(sw)  digitalWrite(hts->cs_pin, (sw))

static uint8_t aTxBuffer[4];

/*
 *  STMP610レジスタ読み出し
 */
static ER_UINT
touchscreen_readRegister(TouchScreen_Handle_t *hts, uint8_t reg)
{
	ER_UINT ercd = E_OK;

	if(hts->spi_lock != 0){
		if((ercd = wai_sem(hts->spi_lock)) != E_OK)
			return ercd;
	}
	cs_set(PORT_LOW);

	aTxBuffer[0] = 0x80 | reg;
	aTxBuffer[1] = 0;
#if SPI_WAIT_TIME != 0
	ercd = spi_transmit(hts->hspi, (uint8_t*)aTxBuffer, 2);
#else
	if((ercd = spi_transmit(hts->hspi, (uint8_t*)aTxBuffer, 2)) == E_OK){
		ercd = spi_wait(hts->hspi, 100);
	}
#endif

	if(ercd == E_OK){
#if SPI_WAIT_TIME != 0
		ercd = spi_receive(hts->hspi, (uint8_t*)aTxBuffer, 1);
#else
		if((ercd = spi_receive(hts->hspi, (uint8_t*)aTxBuffer, 1)) == E_OK){
			ercd = spi_wait(hts->hspi, 100);
		}
#endif
	}
	if(ercd == E_OK)
		ercd = aTxBuffer[0];

	cs_set(PORT_HIGH);
	if(hts->spi_lock != 0)
		sig_sem(hts->spi_lock);
	return ercd;
}

/*
 *  STMP610レジスタ書き込み
 */
ER
touchscreen_writeRegister(TouchScreen_Handle_t *hts, uint8_t reg, uint8_t val)
{
	ER ercd = E_OK;

	if(hts->spi_lock != 0){
		if((ercd = wai_sem(hts->spi_lock)) != E_OK)
			return ercd;
	}
	cs_set(PORT_LOW);

	aTxBuffer[0] = reg;
	aTxBuffer[1] = val;
#if SPI_WAIT_TIME != 0
	ercd = spi_transmit(hts->hspi, (uint8_t*)aTxBuffer, 2);
#else
	if((ercd = spi_transmit(hts->hspi, (uint8_t*)aTxBuffer, 2)) == E_OK){
		ercd = spi_wait(hts->hspi, 100);
	}
#endif

	cs_set(PORT_HIGH);
	if(hts->spi_lock != 0)
		sig_sem(hts->spi_lock);
	return ercd;
}


/*
 *  STMP610 初期化
 */
ER
touchscreen_init(TouchScreen_Handle_t *hts, uint8_t orientation)
{
	Arduino_PortControlBlock *pgcb = getGpioTable(DIGITAL_PIN, hts->cs_pin);
	GPIO_Init_t GPIO_Init_Data;
	ER ercd;
	uint32_t i;

	if(hts == NULL)
		return E_PAR;
	if(hts->hspi == NULL)
		return E_PAR;
	pgcb = getGpioTable(DIGITAL_PIN, hts->cs_pin);
	if(pgcb == NULL)
		return E_PAR;

	GPIO_Init_Data.mode  = GPIO_MODE_OUTPUT;
	GPIO_Init_Data.pull  = hts->pullmode;
	GPIO_Init_Data.otype = hts->otype;
	GPIO_Init_Data.speed = GPIO_SPEED_FAST;
	gpio_setup(pgcb->giobase, &GPIO_Init_Data, pgcb->giopin);

	/*
	 *  VERSIONを検証
	 */
	if(touchscreen_getversion(hts) != 0x811)
		return E_SYS;
	hts->min_x = TS_MINX;
	hts->min_y = TS_MINY;
	hts->max_x = TS_MAXX;
	hts->max_y = TS_MAXY;

	ercd = touchscreen_writeRegister(hts, STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
	dly_tsk(10);

	for(i = 0 ; i < 65 ; i++){
		touchscreen_readRegister(hts, i);
	}

	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_SYS_CTRL2, 0x0);	/* turn on clocks! */
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_TSC_CTRL, STMPE_TSC_CTRL_XYZ | STMPE_TSC_CTRL_EN);	/* XYZ and enable! */
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_INT_EN, STMPE_INT_EN_TOUCHDET);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_10BIT | (0x6 << 4));	/* 96 clocks per conversion */
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_6_5MHZ);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE | STMPE_TSC_CFG_DELAY_1MS | STMPE_TSC_CFG_SETTLE_5MS);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_TSC_FRACTION_Z, 0x6);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_FIFO_TH, 1);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_FIFO_STA, 0);		/* unreset */
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_INT_STA, 0xFF);		/* reset all ints */
	if(ercd == E_OK)
		ercd = touchscreen_writeRegister(hts, STMPE_INT_CTRL, STMPE_INT_CTRL_POL_HIGH | STMPE_INT_CTRL_ENABLE);
	return ercd;
}

/*
 *  STMP610 タッチ判定
 *  param1  hts: Pointer to Touch Screen Handler
 *  result  エラーまたはbool判定
 */
ER_BOOL
touchscreen_touched(TouchScreen_Handle_t *hts)
{
	ER_BOOL ercd;

	if(hts == NULL)
		return E_PAR;
	if((ercd = touchscreen_readRegister(hts, STMPE_TSC_CTRL)) >= E_OK)
		ercd = (ercd & 0x80) != 0;
	return ercd;
}

/*
 *  STMP610 FIFOサイズ読み出し
 *  param1  hts: Pointer to Touch Screen Handler
 *  result  エラーまたは現在のアイテム数
 */
ER_UINT
touchscreen_buffersize(TouchScreen_Handle_t *hts)
{
	ER_BOOL ercd;

	if(hts == NULL)
		return E_PAR;
	if((ercd = touchscreen_readRegister(hts, STMPE_FIFO_STA)) >= E_OK){
		if((ercd & STMPE_FIFO_STA_EMPTY) == 0)
			ercd = touchscreen_readRegister(hts, STMPE_FIFO_SIZE);
		else
			ercd = 0;
	}
	return ercd;
}

/*
 *  STMP610 ポイント読み出し
 *  param1  hts: Pointer to Touch Screen Handler
 *  param2  pt:  Pointer to Point type arry
 *  param3  len: max number of arry items
 *  result  エラーまたは設定アイテム数
 */
ER_UINT
touchscreen_getpoint(TouchScreen_Handle_t *hts, TouchScreen_Point_t *pt, uint8_t len)
{
	ER_UINT  buffer[4], no = 0, ercd;
	uint32_t i;

	if(hts == NULL || pt == NULL)
		return E_PAR;
	while((ercd = touchscreen_buffersize(hts)) > 0){
		for(i = 0 ; i < 4 ; i++){
			if((buffer[i] = touchscreen_readRegister(hts, 0xD7)) < E_OK)
				return buffer[i];
		}
		if(no < len){
			pt->x  = buffer[0] << 4;
			pt->x |= (buffer[1] >> 4 & 0x0F);
			pt->y  = (buffer[1] & 0x0F) << 8;
			pt->y |= buffer[2] & 0xFF;
			pt->z = buffer[3];
			no++;
			pt++;
		}
	}
	return no;
}

/*
 *  STMP610 バージョン読み出し
 */
ER_UINT
touchscreen_getversion(TouchScreen_Handle_t *hts)
{
	ER_UINT ercd, ercd2;

	if(hts == NULL)
		return E_PAR;

	if((ercd = touchscreen_readRegister(hts, 0)) >= E_OK){
		ercd <<= 8;
		if((ercd2 = touchscreen_readRegister(hts, 1)) >= E_OK){
			ercd |= ercd2 & 0xFF;
			return ercd;
		}
		ercd = ercd2;
	}
	return ercd;
}

