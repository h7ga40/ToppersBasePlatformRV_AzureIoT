/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2010 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2019 by TOPPERS PROJECT Educational Working Group.
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
 *  ADAFRUIT ST7789 LCD制御プログラムのヘッダファイル
 */

#ifndef _ADAFRUIT_ST7789_H_
#define _ADAFRUIT_ST7789_H_

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

#define ST7789_NOP          0x00
#define ST7789_SWRESET      0x01
#define ST7789_RDDID        0x04
#define ST7789_RDDST        0x09

#define ST7789_SLPIN        0x10
#define ST7789_SLPOUT       0x11
#define ST7789_PTLON        0x12
#define ST7789_NORON        0x13

#define ST7789_INVOFF       0x20
#define ST7789_INVON        0x21
#define ST7789_DISPOFF      0x28
#define ST7789_DISPON       0x29
#define ST7789_CASET        0x2A
#define ST7789_RASET        0x2B
#define ST7789_RAMWR        0x2C
#define ST7789_RAMRD        0x2E

#define ST7789_PTLAR        0x30
#define ST7789_TEOFF        0x34
#define ST7789_TEON         0x35
#define ST7789_COLMOD       0x3A
#define ST7789_MADCTL       0x36

#define ST7789_MADCTL_MY    0x80
#define ST7789_MADCTL_MX    0x40
#define ST7789_MADCTL_MV    0x20
#define ST7789_MADCTL_ML    0x10
#define ST7789_MADCTL_RGB   0x00

#define ST7789_RDID1        0xDA
#define ST7789_RDID2        0xDB
#define ST7789_RDID3        0xDC
#define ST7789_RDID4        0xDD

// Color definitions
#define	ST7789_BLACK        0x0000
#define	ST7789_BLUE         0x001F
#define	ST7789_RED          0xF800
#define	ST7789_GREEN        0x07E0
#define ST7789_CYAN         0x07FF
#define ST7789_MAGENTA      0xF81F
#define ST7789_YELLOW       0xFFE0
#define ST7789_WHITE        0xFFFF


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
	uint16_t                colstart;
	uint16_t                rowstart;
	uint16_t                lcd_width;
	uint16_t                lcd_height;
	uint16_t                lcd_colstart;
	uint16_t                lcd_rowstart;
	uint8_t                 rotation;
	uint8_t                 dummy[3];
	ID                      spi_lock;
	int8_t                  cs_pin;
	int8_t                  rs_pin;
	int8_t                  rst_pin;
	int8_t                  cs2_pin;
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
extern ER lcd_writedata(LCD_Handler_t *hlcd, uint8_t *p, uint32_t len);
extern ER lcd_writedata2(LCD_Handler_t *hlcd, uint16_t c, int cnt);
extern ER lcd_writedata3(LCD_Handler_t *hlcd, uint8_t *pbmp, uint16_t width, uint16_t height);

extern void lcd_init(LCD_Handler_t *hlcd, uint16_t width, uint16_t height);
extern void lcd_setAddrWindow(LCD_Handler_t *hlcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
extern void lcd_setRotation(LCD_Handler_t *hlcd, uint8_t m);
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

#endif	/* _ADAFRUIT_ST7789_H_ */

