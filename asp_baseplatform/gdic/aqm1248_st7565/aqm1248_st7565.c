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
 *  AQM1248 ST7565 LCD制御プログラムの本体
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
#include "aqm1248_st7565.h"

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

#define PORT_HIGH           1
#define PORT_LOW            0

#define cs_set(sw)  digitalWrite(hlcd->cs_pin, sw)
#define rs_set(sw)  digitalWrite(hlcd->rs_pin, sw)
#define rst_set(sw) digitalWrite(hlcd->rst_pin, sw)

static const uint8_t left_mask_table[8] = {
	0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
};

static const uint8_t right_mask_table[8] = {
	0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF
};

static const uint8_t dither_table[8][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	/* BLACK */
	{0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},	/* BLUE */
	{0xFA, 0x5F, 0xF5, 0xAF, 0xFA, 0x5F, 0xF5, 0xAF},	/* GREEN */
	{0xFD, 0xDF, 0xFB, 0xBF, 0xFD, 0xDF, 0xFB, 0xBF},	/* CYAN */
	{0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA},	/* RED */
	{0xAF, 0xF5, 0x5F, 0xFA, 0xAF, 0xF5, 0x5F, 0xFA},	/* MAGENTA */
	{0xDF, 0xFD, 0xBF, 0xFB, 0xDF, 0xFD, 0xBF, 0xFB},	/* YELLOW */
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}	/* WHITE */
};

/* Buffer used for transmission */
static uint8_t aTxBuffer[4];


/*
 *  バッファデータをLCDに転送
 */
static void
lcd_flush_buffer(LCD_Handler_t *hlcd)
{
	uint32_t l, col, i;
	uint8_t page, val1, val2;

	for(l = hlcd->_winy1 ; l <= hlcd->_winy2 ; l += AQM1248_PLINE){
		page = l / AQM1248_PLINE;
		col  = hlcd->_winx1;
		SVC_PERROR(lcd_writecommand(hlcd, ST7565_PAGEADR + page));
		SVC_PERROR(lcd_writecommand(hlcd, ST7565_COLADR + (col >> 4)));
		SVC_PERROR(lcd_writecommand(hlcd, (col & 0x0f)));
		for(; col <= hlcd->_winx2 ; col++){
			for(i = val1 = 0 ; i < 8 ; i++){
				val2  = hlcd->buffer[(page * AQM1248_PLINE) + i][col/8] << (col & 7);
				val1 |= (val2 & 0x80) >> (7 - i);
			}
			SVC_PERROR(lcd_writedata(hlcd, val1));
		}
	}
}

/*
 *  バッファ上にイメージを作る
 */
static void
lcd_fill_buffer(LCD_Handler_t *hlcd, color_t color)
{
	uint32_t x, y;
	uint8_t  *p;

	color &= 7;
	for(y = hlcd->_winy1 ; y <= hlcd->_winy2 ; y++){
		p = &hlcd->buffer[y][hlcd->_winx1/8];
		for(x = 0 ; x < hlcd->stride ; x++, p++){
			if(x == 0){
				*p &= ~hlcd->lmask;
				*p |= dither_table[color][y & 7] & hlcd->lmask;
			}
			else if(x == (hlcd->stride-1)){
				*p &= ~hlcd->rmask;
				*p |= dither_table[color][y & 7] & hlcd->rmask;
			}
			else
				*p = dither_table[color][y & 7];
		}
	}
}

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
	rs_set(PORT_LOW);
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
	rs_set(PORT_HIGH);
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
 *  ST7565 LCDの初期化関数
 */
void
lcd_init(LCD_Handler_t *hlcd)
{
	uint32_t i;

	rst_set(PORT_LOW);
	dly_tsk(100);
	rst_set(PORT_HIGH);
	dly_tsk(100);

	hlcd->_width  = AQM1248_WIDTH;
	hlcd->_height = AQM1248_HEIGHT;

	SVC_PERROR(lcd_writecommand(hlcd, ST7565_DISPOFF));		/* 表示OFF */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_ADCSELN));		/* ADC select normal */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_COMOUTR));		/* comon output reverse */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_BIAS17));		/* lcd bias 1/7 */

	SVC_PERROR(lcd_writecommand(hlcd, ST7565_PWRSEL+4));	/* power select 1 */
	dly_tsk(2);
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_PWRSEL+6));	/* power select 2 */
	dly_tsk(2);
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_PWRSEL+7));	/* power select 3 */

	SVC_PERROR(lcd_writecommand(hlcd, ST7565_RATSEL+3));	/* voltage resistor ratio 3 */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_SETEVOL));		/* electronic volume set */
	SVC_PERROR(lcd_writecommand(hlcd, 0x1C));				/* volume value */

	SVC_PERROR(lcd_writecommand(hlcd, ST7565_ALLPNL));		/* all point normal */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_STRLINE));		/* start line #0 */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_DISPREV));		/* 表示reverse */
	SVC_PERROR(lcd_writecommand(hlcd, ST7565_DISPON));		/* 表示ON */
	for(i = 0 ; i < AQM1248_HEIGHT ; i++)
		memset(&hlcd->buffer[i][0], 0xff, AQM1248_WIDTH/8);
}

/*
 *  表示ウィンドウ設定
 */
void
lcd_setAddrWindow(LCD_Handler_t *hlcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	uint8_t hx = x0 & 7;

	hlcd->_winx1 = x0;
	hlcd->_winy1 = y0;
	hlcd->_winx2 = x1;
	hlcd->_winy2 = y1;
	hlcd->lmask  = left_mask_table[hx];
	hlcd->rmask  = right_mask_table[x1 & 7];
	if(hx == 0)
		hlcd->stride = (x1 - x0 + 8) / 8;
	else
		hlcd->stride = (x1 - x0 + hx) / 8 + 1;
	if(hlcd->stride == 1)
		hlcd->lmask &= hlcd->rmask;
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
	lcd_fill_buffer(hlcd, color);
	lcd_flush_buffer(hlcd);
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
	lcd_fill_buffer(hlcd, color);
	lcd_flush_buffer(hlcd);
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
	lcd_fill_buffer(hlcd, color);
	lcd_flush_buffer(hlcd);
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
	lcd_fill_buffer(hlcd, color);
	lcd_flush_buffer(hlcd);
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

	for(i = 0 ; i < w ; i++, pcolor++){
		lcd_setAddrWindow(hlcd, x+i, y, x+i, y);
		lcd_fill_buffer(hlcd, *pcolor);
		lcd_flush_buffer(hlcd);
	}
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
	uint16_t val, l, col;

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
	syslog_4(LOG_NOTICE, "## input_color_mode(%d) width(%d) height(%d) bit_pixel(%d) ##", input_color_mode, width, height, bit_pixel);

	for(l = 0 ; l < height ; l++){
		uint8_t *p = pbmp;
		for(col = 0 ; col < width ; col++){
			switch(input_color_mode){
			case CM_RGB565:
				if((*p & 0xC0) == 0xC0)
					val = 0x04;
				else
					val = 0x00;
				if((*p++ & 0x06) == 0x06)
					val |= 0x02;
				if((*p++ & 0x18) == 0x18)
					val |= 0x01;
				break;
			default:
				if((*p++ & 0xE0) == 0xE0)
					val = 0x04;
				else
					val = 0x00;
				if((*p++ & 0xE0) == 0xE0)
					val |= 0x02;
				if((*p++ & 0xE0) == 0xE0)
					val |= 0x01;
				break;
			}
			if((l+y0) < hlcd->_height)
				lcd_drawPixel(hlcd, col+x0, l+y0, val);
		}
		pbmp -= (bit_pixel/8) * width;
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
 	SVC_PERROR(lcd_writecommand(hlcd, i ? ST7565_DISPNOR : ST7565_DISPREV));
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
	if(y < 0) return;
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

