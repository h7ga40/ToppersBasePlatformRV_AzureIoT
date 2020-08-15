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
 *  AQM1248 ST7565 LCD制御プログラムのヘッダファイル
 */

#ifndef _AQM1248_ST7565_H_
#define _AQM1248_ST7565_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "device.h"
#include "pinmode.h"
#include "spi.h"

/*
 *  LCDカラーモード定義
 */
#define CM_ARGB8888         0x00000000			/* ARGB8888 color mode */
#define CM_RGB888           0x00000001			/* RGB888 color mode */
#define CM_RGB565           0x00000002			/* RGB565 color mode */

#define AQM1248_WIDTH  128
#define AQM1248_HEIGHT 48
#define AQM1248_PLINE  8

#define ST7565_COLADR  0x10
#define ST7565_RATSEL  0x20
#define ST7565_PWRSEL  0x28
#define ST7565_STRLINE 0x40
#define ST7565_SETEVOL 0x81

#define ST7565_ADCSELN 0xA0
#define ST7565_ADCSELR 0xA1
#define ST7565_BIAS19  0xA2
#define ST7565_BIAS17  0xA3
#define ST7565_ALLPNL  0xA4
#define ST7565_ALLPON  0xA5
#define ST7565_DISPNOR 0xA6
#define ST7565_DISPREV 0xA7
#define ST7565_DISPOFF 0xAE
#define ST7565_DISPON  0xAF
#define ST7565_PAGEADR 0xB0

#define ST7565_COMOUTN 0xC0
#define ST7565_COMOUTR 0xC8
#define ST7565_NOP     0xE3

// Color definitions
#define	ST7565_BLACK   0x00
#define	ST7565_BLUE    0x01
#define	ST7565_GREEN   0x02
#define ST7565_CYAN    0x03
#define	ST7565_RED     0x04
#define ST7565_MAGENTA 0x05
#define ST7565_YELLOW  0x06
#define ST7565_WHITE   0x07

#ifndef TOPPERS_MACRO_ONLY

/*
 *  カラーの属性を定義
 */
typedef uint16_t   color_t;


/*
 *  LCDハンドラ構造体定義
 */
typedef struct
{
	SPI_Handle_t            *hspi;		/* spiハンドラ */
	uint16_t                _width;		/* 幅ピクセル数 */
	uint16_t				_height;	/* 高さピクセル数 */
	uint16_t                _winx1;
	uint16_t                _winy1;
	uint16_t                _winx2;
	uint16_t                _winy2;
	ID                      spi_lock;
	uint8_t                 cs_pin;
	uint8_t                 rs_pin;
	uint8_t                 rst_pin;
	uint8_t                 lmask;
	uint8_t                 rmask;
	uint8_t                 stride;
	uint8_t                 dummy[2];
	uint8_t                 buffer[AQM1248_HEIGHT][AQM1248_WIDTH/8];
}LCD_Handler_t;

/*
 *  描画構造体
 */
typedef struct
{
	LCD_Handler_t           *hlcd;
	color_t                 TextColor;
	color_t                 BackColor;
	void                    *pFont;
}LCD_DrawProp_t;

typedef struct
{
	int16_t X;
	int16_t Y;
}Point, * pPoint;

/*
 *  関数のプロトタイプ宣言
 */
extern ER lcd_writecommand(LCD_Handler_t *hlcd, uint8_t c);
extern ER lcd_writedata(LCD_Handler_t *hlcd, uint8_t c);

extern void lcd_init(LCD_Handler_t *hlcd);
extern void lcd_setAddrWindow(LCD_Handler_t *hlcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
extern void lcd_fillRect(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, int16_t h, color_t color);
extern void lcd_pushColor(LCD_Handler_t *hlcd, color_t color);
extern void lcd_drawPixel(LCD_Handler_t *hlcd, int16_t x, int16_t y, color_t color);
extern void lcd_drawFastVLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t h, color_t color);
extern void lcd_drawFastHLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, color_t color);
extern void lcd_drawImageHLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, uint16_t w, color_t *pcolor);
extern void lcd_drawBitmap(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, uint8_t *pbmp);
extern void lcd_invertDisplay(LCD_Handler_t *hlcd, bool_t i);

extern void lcd_fillScreen(LCD_DrawProp_t *pDrawProp);
extern void lcd_drawRect(LCD_DrawProp_t *pDrawProp, int16_t x, int16_t y, int16_t w, int16_t h);
extern void lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, uint16_t Radius);
extern void lcd_drawLine(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount);

#endif /* TOPPERS_MACRO_ONLY */

#ifdef __cplusplus
}
#endif

#endif	/* _AQM1248_ST7565_H_ */

