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
 *  $Id$
 */

/*
 *  Written by Limor Fried/Ladyada for Adafruit Industries.
 *  MIT license, all text above must be included in any redistribution
 *　http://opensource.org/licenses/mit-license.php 
 */

/* 
 *  ADAFRUIT ILI9341 2.8"LCD制御プログラムの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/syslog.h"
#include "device.h"
#include "adafruit_ILI9341.h"

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
#define ABS(X)  ((X) > 0 ? (X) : -(X))


/* Buffer used for transmission */
static uint8_t aTxBuffer[4];

#define PORT_HIGH           1
#define PORT_LOW            0

#define MADCTL_MY           0x80
#define MADCTL_MX           0x40
#define MADCTL_MV           0x20
#define MADCTL_ML           0x10
#define MADCTL_RGB          0x00
#define MADCTL_BGR          0x08
#define MADCTL_MH           0x04


#define cs_set(sw)  digitalWrite(hlcd->cs_pin, sw)
#define dc_set(sw)  digitalWrite(hlcd->dc_pin, sw)
#define rst_set(sw) digitalWrite(hlcd->rst_pin, sw)
#define cs2_set(sw) digitalWrite(hlcd->cs2_pin, sw)


/*
 *  LCDへのコマンド送信関数
 */
ER
lcd_writecommand(LCD_Handler_t *hlcd, uint8_t c)
{
	ER ercd = E_OK;

	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	dc_set(PORT_LOW);
	cs_set(PORT_LOW);

	aTxBuffer[0] = c;
#if SPI_WAIT_TIME != 0
	ercd = spi_transmit(hlcd->hspi, (uint8_t*)aTxBuffer, 1);
#else
	if((ercd = spi_transmit(hlcd->hspi, (uint8_t*)aTxBuffer, 1)) == E_OK){
		ercd = spi_wait(hlcd->hspi, 100);
	}
#endif

	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへのデータ送信関数
 */
ER
lcd_writedata(LCD_Handler_t *hlcd, uint8_t c)
{
	ER ercd = E_OK;

	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	dc_set(PORT_HIGH);
	cs_set(PORT_LOW);
	sil_dly_nse(100);

	aTxBuffer[0] = c;
#if SPI_WAIT_TIME != 0
	ercd = spi_transmit(hlcd->hspi, (uint8_t*)aTxBuffer, 1);
#else
	if((ercd = spi_transmit(hlcd->hspi, (uint8_t*)aTxBuffer, 1)) == E_OK){
		ercd = spi_wait(hlcd->hspi, 100);
	}
#endif

	sil_dly_nse(100);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへの２バイトデータ送信関数
 */
ER
lcd_writedata2(LCD_Handler_t *hlcd, uint16_t c, int cnt)
{
	ER  ercd = E_OK;
	int i;

	if(cnt == 0)
		return ercd;
	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	dc_set(PORT_HIGH);
	cs_set(PORT_LOW);
	sil_dly_nse(100);
	aTxBuffer[0] = c >> 8;
	aTxBuffer[1] = (uint8_t)c;
	for(i = 0 ; i < cnt ; i++){
		if((ercd = spi_transmit(hlcd->hspi, (uint8_t*)aTxBuffer, 2)) != E_OK){
			break;
		}
#if SPI_WAIT_TIME == 0
		if((ercd = spi_wait(hlcd->hspi, 100)) != E_OK){
			break;
		}
#endif
	}
	sil_dly_nse(100);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへのRGBデータ送信関数
 */
ER
lcd_writedata3(LCD_Handler_t *hlcd, uint8_t *pbmp, uint16_t width, uint16_t height)
{
	ER  ercd = E_OK;
	int index, i;

	if(width == 0 || height == 0)
		return ercd;
	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	dc_set(PORT_HIGH);
	cs_set(PORT_LOW);
	sil_dly_nse(100);
	for(index=0; index < height; index++){
		uint8_t *p = pbmp;
		for (i = 0; i < width; i++){
			aTxBuffer[0]  = (*p++) & 0xf8;
			aTxBuffer[0] |= *p >> 5;
			aTxBuffer[1]  = (*p++ << 3) & 0xE0;
			aTxBuffer[1] |= (*p++ >> 3) & 0x1F;
			if((ercd = spi_transmit(hlcd->hspi, (uint8_t*)aTxBuffer, 2)) != E_OK){
				break;
			}
#if SPI_WAIT_TIME == 0
			if((ercd = spi_wait(hlcd->hspi, 100)) != E_OK){
				break;
			}
#endif
		}
		pbmp -= width*3;
	}
	sil_dly_nse(100);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}


/*
 *  表示ウィンドウ設定
 */
void
lcd_setAddrWindow(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	uint16_t tmp;

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_CASET)); // Column addr set
	tmp = x0 + hlcd->colstart;
	SVC_PERROR(lcd_writedata(hlcd, (tmp >> 8)));
	SVC_PERROR(lcd_writedata(hlcd, (tmp & 0xFF)));     // XSTART 
	tmp = x1 + hlcd->colstart;
	SVC_PERROR(lcd_writedata(hlcd, (tmp >> 8)));
	SVC_PERROR(lcd_writedata(hlcd, (tmp & 0xFF)));     // XEND

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_PASET)); // Row addr set
	tmp = y0 + hlcd->rowstart;
	SVC_PERROR(lcd_writedata(hlcd, (tmp >> 8)));
	SVC_PERROR(lcd_writedata(hlcd, (tmp & 0xFF)));     // YSTART
	tmp = y1 + hlcd->rowstart;
	SVC_PERROR(lcd_writedata(hlcd, (tmp >> 8)));
	SVC_PERROR(lcd_writedata(hlcd, (tmp & 0xFF)));     // YEND

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_RAMWR)); // write to RAM
}


/*
 *  ILI9341 screensの初期化関数
 */
void
lcd_init(LCD_Handler_t *hlcd, uint8_t rotation)
{
	hlcd->colstart  = hlcd->rowstart = 0; // May be overridden in init func
	hlcd->_width = ILI9341_TFTWIDTH;
	hlcd->_height = ILI9341_TFTHEIGHT;

	// toggle RST low to reset; CS low so it'll listen to us
	cs_set(PORT_LOW);
	if(hlcd->rst_pin < 14){
		rst_set(PORT_HIGH);
		dly_tsk(5);
		rst_set(PORT_LOW);
		dly_tsk(20);
		rst_set(PORT_HIGH);
		dly_tsk(150);
	}

	SVC_PERROR(lcd_writecommand(hlcd, 0xEF));
	SVC_PERROR(lcd_writedata(hlcd, 0x03));
	SVC_PERROR(lcd_writedata(hlcd, 0x80));
	SVC_PERROR(lcd_writedata(hlcd, 0x02));

	SVC_PERROR(lcd_writecommand(hlcd, 0xCF));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, 0xC1));
	SVC_PERROR(lcd_writedata(hlcd, 0x30));

	SVC_PERROR(lcd_writecommand(hlcd, 0xED));
	SVC_PERROR(lcd_writedata(hlcd, 0x64));
	SVC_PERROR(lcd_writedata(hlcd, 0x03));
	SVC_PERROR(lcd_writedata(hlcd, 0x12));
	SVC_PERROR(lcd_writedata(hlcd, 0x81));

	SVC_PERROR(lcd_writecommand(hlcd, 0xE8));
	SVC_PERROR(lcd_writedata(hlcd, 0x85));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, 0x78));

	SVC_PERROR(lcd_writecommand(hlcd, 0xCB));
	SVC_PERROR(lcd_writedata(hlcd, 0x39));
	SVC_PERROR(lcd_writedata(hlcd, 0x2C));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, 0x34));
	SVC_PERROR(lcd_writedata(hlcd, 0x02));

	SVC_PERROR(lcd_writecommand(hlcd, 0xF7));
	SVC_PERROR(lcd_writedata(hlcd, 0x20));

	SVC_PERROR(lcd_writecommand(hlcd, 0xEA));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_PWCTR1));	/* Power control */
	SVC_PERROR(lcd_writedata(hlcd, 0x23));				/* VRH[5:0] */

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_PWCTR2));	/* Power control */
	SVC_PERROR(lcd_writedata(hlcd, 0x10));				/* SAP[2:0];BT[3:0] */

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_VMCTR1));	/* VCM control */
	SVC_PERROR(lcd_writedata(hlcd, 0x3e));
	SVC_PERROR(lcd_writedata(hlcd, 0x28));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_VMCTR2));	/* VCM control2 */
	SVC_PERROR(lcd_writedata(hlcd, 0x86));				/* -- */

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_MADCTL));	/* Memory Access Control */
	SVC_PERROR(lcd_writedata(hlcd, 0x48));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_PIXFMT));
	SVC_PERROR(lcd_writedata(hlcd, 0x55));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_FRMCTR1));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, 0x18));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_DFUNCTR)); /* Display Function Control */
	SVC_PERROR(lcd_writedata(hlcd, 0x08));
	SVC_PERROR(lcd_writedata(hlcd, 0x82));
	SVC_PERROR(lcd_writedata(hlcd, 0x27));

	SVC_PERROR(lcd_writecommand(hlcd, 0xF2));			/* 3Gamma Function Disable */
	SVC_PERROR(lcd_writedata(hlcd, 0x00));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_GAMMASET)); /* Gamma curve selected */
	SVC_PERROR(lcd_writedata(hlcd, 0x01));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_GMCTRP1)); /* Set Gamma */
	SVC_PERROR(lcd_writedata(hlcd, 0x0F));
	SVC_PERROR(lcd_writedata(hlcd, 0x31));
	SVC_PERROR(lcd_writedata(hlcd, 0x2B));
	SVC_PERROR(lcd_writedata(hlcd, 0x0C));
	SVC_PERROR(lcd_writedata(hlcd, 0x0E));
	SVC_PERROR(lcd_writedata(hlcd, 0x08));
	SVC_PERROR(lcd_writedata(hlcd, 0x4E));
	SVC_PERROR(lcd_writedata(hlcd, 0xF1));
	SVC_PERROR(lcd_writedata(hlcd, 0x37));
	SVC_PERROR(lcd_writedata(hlcd, 0x07));
	SVC_PERROR(lcd_writedata(hlcd, 0x10));
	SVC_PERROR(lcd_writedata(hlcd, 0x03));
	SVC_PERROR(lcd_writedata(hlcd, 0x0E));
	SVC_PERROR(lcd_writedata(hlcd, 0x09));
	SVC_PERROR(lcd_writedata(hlcd, 0x00));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_GMCTRN1)); /* Set Gamma */
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, 0x0E));
	SVC_PERROR(lcd_writedata(hlcd, 0x14));
	SVC_PERROR(lcd_writedata(hlcd, 0x03));
	SVC_PERROR(lcd_writedata(hlcd, 0x11));
	SVC_PERROR(lcd_writedata(hlcd, 0x07));
	SVC_PERROR(lcd_writedata(hlcd, 0x31));
	SVC_PERROR(lcd_writedata(hlcd, 0xC1));
	SVC_PERROR(lcd_writedata(hlcd, 0x48));
	SVC_PERROR(lcd_writedata(hlcd, 0x08));
	SVC_PERROR(lcd_writedata(hlcd, 0x0F));
	SVC_PERROR(lcd_writedata(hlcd, 0x0C));
	SVC_PERROR(lcd_writedata(hlcd, 0x31));
	SVC_PERROR(lcd_writedata(hlcd, 0x36));
	SVC_PERROR(lcd_writedata(hlcd, 0x0F));

	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_SLPOUT));	/* Exit Sleep */
	dly_tsk(120);
	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_DISPON));	/* Display on */
	lcd_setRotation(hlcd, rotation);
}

/*
 *  RECTANGLEのフィル描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  w: width
 *  param5  h: height
 *  param6  color: color value
 */
void
lcd_fillRect(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, int16_t h, color_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x + w - 1) >= hlcd->_width)  w = hlcd->_width  - x;
	if((y + h - 1) >= hlcd->_height) h = hlcd->_height - y;

	lcd_setAddrWindow(hlcd, x, y, x+w-1, y+h-1);
	SVC_PERROR(lcd_writedata2(hlcd, color, h*w));
}

void
lcd_pushColor(LCD_Handler_t *hlcd, color_t color)
{
	SVC_PERROR(lcd_writedata2(hlcd, color, 1));
}

/*
 *  PIXEL描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 *  param4  color: color value
 */
void
lcd_drawPixel(LCD_Handler_t *hlcd, int16_t x, int16_t y, color_t color)
{
	if((x < 0) ||(x >= hlcd->_width) || (y < 0) || (y >= hlcd->_height)) return;

	lcd_setAddrWindow(hlcd, x, y, x, y);
	SVC_PERROR(lcd_writedata2(hlcd, color, 1));
}

/*
 *  垂直方向LINEの高速描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  h: height
 *  param5  color: color value
 */
void
lcd_drawFastVLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t h, color_t color)
{
	// Rudimentary clipping
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((y+h-1) >= hlcd->_height) h = hlcd->_height-y;
	lcd_setAddrWindow(hlcd, x, y, x, y+h-1);
	SVC_PERROR(lcd_writedata2(hlcd, color, h));
}

/*
 *  水平方向LINEの高速描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  w: width
 *  param5  color: color value
 */
void lcd_drawFastHLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, color_t color)
{
	// Rudimentary clipping
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x+w-1) >= hlcd->_width)  w = hlcd->_width-x;
	lcd_setAddrWindow(hlcd, x, y, x+w-1, y);
	SVC_PERROR(lcd_writedata2(hlcd, color, w));
}

/*
 *  DRAW IMAGE LINE描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 *  param4  w: width
 *  param5  pcolor: color value
 */
void
lcd_drawImageHLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, uint16_t w, color_t *pcolor)
{
	uint32_t i;

	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x+w-1) >= hlcd->_width)  w = hlcd->_width-x;

	lcd_setAddrWindow(hlcd, x, y, x+w-1, y);
	for(i = 0 ; i < w ; i++, pcolor++)
		SVC_PERROR(lcd_writedata2(hlcd, *pcolor, 1));
}

/*
 *  BITMAP描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x0: Bmp X position in the LCD
 *  param3  y0: Bmp Y position in the LCD
 *  param4  pbmp: Pointer to Bmp picture address in the internal Flash
 */
void
lcd_drawBitmap(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, uint8_t *pbmp)
{
	uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
	uint32_t input_color_mode = 0;

	/* Get bitmap data address offset */
	index = *(uint16_t *) (pbmp + 10);
	index |= (*(uint16_t *) (pbmp + 12)) << 16;

	/* Read bitmap width */
	width = *(uint16_t *) (pbmp + 18);
	width |= (*(uint16_t *) (pbmp + 20)) << 16;

	/* Read bitmap height */
	height = *(uint16_t *) (pbmp + 22);
	height |= (*(uint16_t *) (pbmp + 24)) << 16;

	/* Read bit/pixel */
	bit_pixel = *(uint16_t *) (pbmp + 28);

	/* Get the layer pixel format */
	if ((bit_pixel/8) == 4){
		input_color_mode = CM_ARGB8888;
	}
	else if ((bit_pixel/8) == 2){
		input_color_mode = CM_RGB565;
	}
	else{
		input_color_mode = CM_RGB888;
	}

	/* Bypass the bitmap header */
	pbmp += (index + (width * (height - 1) * (bit_pixel/8)));  
	syslog_4(LOG_INFO, "lcd_drawBitmap input_color_mode(%d) width(%d) height(%d) bit_pixel(%d)", input_color_mode, width, height, bit_pixel);

	lcd_setAddrWindow(hlcd, x0, y0, x0+width-1, y0+height-1);
	lcd_writedata3(hlcd, pbmp, width, height);
}

/*
 *  ROTATION設定
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  m: 0:0, 1:90, 2:180, 3:270
 */
void
lcd_setRotation(LCD_Handler_t *hlcd, uint8_t m)
{
	SVC_PERROR(lcd_writecommand(hlcd, ILI9341_MADCTL));
	hlcd->rotation = m % 4;	// can't be higher than 3
	switch(hlcd->rotation){
	case 1:
		SVC_PERROR(lcd_writedata(hlcd, (MADCTL_MV | MADCTL_BGR)));
		hlcd->_width  = ILI9341_TFTHEIGHT;
		hlcd->_height = ILI9341_TFTWIDTH;
		break;
	case 2:
		SVC_PERROR(lcd_writedata(hlcd, (MADCTL_MY | MADCTL_BGR)));
		hlcd->_width  = ILI9341_TFTWIDTH;
		hlcd->_height = ILI9341_TFTHEIGHT;
		break;
	case 3:
		SVC_PERROR(lcd_writedata(hlcd, (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR)));
		hlcd->_width  = ILI9341_TFTHEIGHT;
		hlcd->_height = ILI9341_TFTWIDTH;
		break;
	case 0:
	default:
		SVC_PERROR(lcd_writedata(hlcd, (MADCTL_MX | MADCTL_BGR)));
		hlcd->_width  = ILI9341_TFTWIDTH;
		hlcd->_height = ILI9341_TFTHEIGHT;
		break;
	}
}

/*
 *  INVERT DISPLAY
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  i: invert value
 */
void
lcd_invertDisplay(LCD_Handler_t *hlcd, bool_t i)
{
	SVC_PERROR(lcd_writecommand(hlcd, i ? ILI9341_INVON : ILI9341_INVOFF));
}


/*
 *  スクリーンフィル
 *  param1  pDrawProp: Pointer to Draw Prop
 */
void
lcd_fillScreen(LCD_DrawProp_t *pDrawProp)
{
	lcd_fillRect(pDrawProp->hlcd, 0, 0,  pDrawProp->hlcd->_width, pDrawProp->hlcd->_height, pDrawProp->BackColor);
}

/*
 *  RECTANGLE描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x: left X position
 *  param3  y: top Y position
 *  param4  w: width
 *  param5  h: height
 */
void
lcd_drawRect(LCD_DrawProp_t *pDrawProp, int16_t x, int16_t y, int16_t w, int16_t h)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	color_t color;

	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x + w - 1) >= hlcd->_width)  w = hlcd->_width  - x;
	if((y + h - 1) >= hlcd->_height) h = hlcd->_height - y;

	color = pDrawProp->TextColor;
	lcd_drawFastVLine(hlcd, x, y, h, color);
	lcd_drawFastHLine(hlcd, x, y+h-1, w, color);
	lcd_drawFastVLine(hlcd, x+w-1, y, h, color);
	lcd_drawFastHLine(hlcd, x, y, w, color);
}

/*
 *  円描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: X position
 *  param3  y0: Y position
 *  param4  Radius: Circle radius
 */
void
lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, uint16_t Radius)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
 	int32_t   decision;    /* Decision Variable */ 
	uint32_t  current_x;   /* Current X Value */
	uint32_t  current_y;   /* Current Y Value */

	decision = 3 - (Radius << 1);
	current_x = 0;
	current_y = Radius;

	while(current_x <= current_y){
		lcd_drawPixel(hlcd, (x0 + current_x), (y0 - current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_x), (y0 - current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 + current_y), (y0 - current_x), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_y), (y0 - current_x), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 + current_x), (y0 + current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_x), (y0 + current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 + current_y), (y0 + current_x), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_y), (y0 + current_x), pDrawProp->TextColor);

		if (decision < 0){
			decision += (current_x << 2) + 6;
		}
		else{
			decision += ((current_x - current_y) << 2) + 10;
			current_y--;
		}
		current_x++;
	}
}

/*
 *  線描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x1: Point 1 X position
 *  param3  y1: Point 1 Y position
 *  param4  x2: Point 2 X position
 *  param5  y2: Point 2 Y position
 */
void
lcd_drawLine(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, num_add = 0, num_pixels = 0, 
	curpixel = 0;

	deltax = ABS(x2 - x1);		/* The difference between the x's */
	deltay = ABS(y2 - y1);		/* The difference between the y's */
	x = x1;						/* Start x off at the first pixel */
	y = y1;						/* Start y off at the first pixel */

	if(x2 >= x1){				/* The x-values are increasing */
		xinc1 = 1;
		xinc2 = 1;
	}
	else{						/* The x-values are decreasing */
		xinc1 = -1;
		xinc2 = -1;
	}

	if(y2 >= y1){				/* The y-values are increasing */
		yinc1 = 1;
		yinc2 = 1;
	}
	else{						/* The y-values are decreasing */
		yinc1 = -1;
		yinc2 = -1;
	}

	if(deltax >= deltay){		/* There is at least one x-value for every y-value */
		xinc1 = 0;				/* Don't change the x when numerator >= denominator */
		yinc2 = 0;				/* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		num_add = deltay;
		num_pixels = deltax;	/* There are more x-values than y-values */
	}
	else{						/* There is at least one y-value for every x-value */
		xinc2 = 0;				/* Don't change the x for every iteration */
		yinc1 = 0;				/* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		num_add = deltax;
		num_pixels = deltay;	/* There are more y-values than x-values */
	}

	for (curpixel = 0; curpixel <= num_pixels; curpixel++){
		lcd_drawPixel(hlcd, x, y, pDrawProp->TextColor);	/* Draw the current pixel */
		num += num_add;			/* Increase the numerator by the top of the fraction */
		if(num >= den){			/* Check if numerator >= denominator */
			num -= den;			/* Calculate the new numerator value */
			x += xinc1;			/* Change the x as appropriate */
			y += yinc1;			/* Change the y as appropriate */
		}
		x += xinc2;				/* Change the x as appropriate */
		y += yinc2;				/* Change the y as appropriate */
	}
}

/*
 *  PLOY-LINE描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  Points: Pointer to the points array
 *  param3  PointCount: Number of points
 */
void
lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount)
{
	int16_t x = 0, y = 0;

	if(PointCount < 2){
		return;
	}

	lcd_drawLine(pDrawProp, Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
	while(--PointCount){
		x = Points->X;
		y = Points->Y;
		Points++;
		lcd_drawLine(pDrawProp, x, y, Points->X, Points->Y);
	}
}

