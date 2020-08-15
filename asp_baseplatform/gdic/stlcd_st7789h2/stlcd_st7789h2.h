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

#ifndef _STLCD_ST7789H2_H_
#define _STLCD_ST7789H2_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <target_syssvc.h>
#include "device.h"
#include "ltdc.h"


/**
  * @brief  ST7789H2 ID
  */
#define  ST7789H2_ID    0x85

/**
  * @brief  ST7789H2 Size
  */
#define  ST7789H2_LCD_PIXEL_WIDTH    ((uint16_t)240)
#define  ST7789H2_LCD_PIXEL_HEIGHT   ((uint16_t)240)

/**
 *  @brief LCD_OrientationTypeDef
 *  Possible values of Display Orientation
 */
#define ST7789H2_ORIENTATION_PORTRAIT         ((uint32_t)0x00) /* Portrait orientation choice of LCD screen  */
#define ST7789H2_ORIENTATION_LANDSCAPE        ((uint32_t)0x01) /* Landscape orientation choice of LCD screen */
#define ST7789H2_ORIENTATION_LANDSCAPE_ROT180 ((uint32_t)0x02) /* Landscape rotated 180ｰ orientation choice of LCD screen */

/**
  * @brief  ST7789H2 Registers
  */
#define ST7789H2_LCD_ID             0x04
#define ST7789H2_SLEEP_IN           0x10
#define ST7789H2_SLEEP_OUT          0x11
#define ST7789H2_PARTIAL_DISPLAY    0x12
#define ST7789H2_DISPLAY_INVERSION  0x21
#define ST7789H2_DISPLAY_ON         0x29
#define ST7789H2_WRITE_RAM          0x2C
#define ST7789H2_READ_RAM           0x2E
#define ST7789H2_CASET              0x2A
#define ST7789H2_RASET              0x2B
#define ST7789H2_VSCRDEF            0x33 /* Vertical Scroll Definition */
#define ST7789H2_VSCSAD             0x37 /* Vertical Scroll Start Address of RAM */
#define ST7789H2_TEARING_EFFECT     0x35
#define ST7789H2_NORMAL_DISPLAY     0x36
#define ST7789H2_IDLE_MODE_OFF      0x38
#define ST7789H2_IDLE_MODE_ON       0x39
#define ST7789H2_COLOR_MODE         0x3A
#define ST7789H2_PORCH_CTRL         0xB2
#define ST7789H2_GATE_CTRL          0xB7
#define ST7789H2_VCOM_SET           0xBB
#define ST7789H2_DISPLAY_OFF        0xBD
#define ST7789H2_LCM_CTRL           0xC0
#define ST7789H2_VDV_VRH_EN         0xC2
#define ST7789H2_VDV_SET            0xC4
#define ST7789H2_VCOMH_OFFSET_SET   0xC5
#define ST7789H2_FR_CTRL            0xC6
#define ST7789H2_POWER_CTRL         0xD0
#define ST7789H2_PV_GAMMA_CTRL      0xE0
#define ST7789H2_NV_GAMMA_CTRL      0xE1


/*
 *  ポリゴンポイント定義
 */
typedef struct
{
	int16_t X;
	int16_t Y;
}Point, *pPoint;


#define  LCD_ORIENTATION_PORTRAIT         ((uint8_t)0x00)  /*!< Portrait orientation choice of LCD screen  */
#define  LCD_ORIENTATION_LANDSCAPE        ((uint8_t)0x01)  /*!< Landscape orientation choice of LCD screen */
#define  LCD_ORIENTATION_LANDSCAPE_ROT180 ((uint32_t)0x02) /*!< Landscape rotated 180ｰ orientation choice of LCD screen */


/**
  * @brief  LCD color
  */
#define LCD_COLOR_BLUE          ((uint16_t)0x001F)
#define LCD_COLOR_GREEN         ((uint16_t)0x07E0)
#define LCD_COLOR_RED           ((uint16_t)0xF800)
#define LCD_COLOR_CYAN          ((uint16_t)0x07FF)
#define LCD_COLOR_MAGENTA       ((uint16_t)0xF81F)
#define LCD_COLOR_YELLOW        ((uint16_t)0xFFE0)
#define LCD_COLOR_LIGHTBLUE     ((uint16_t)0x841F)
#define LCD_COLOR_LIGHTGREEN    ((uint16_t)0x87F0)
#define LCD_COLOR_LIGHTRED      ((uint16_t)0xFC10)
#define LCD_COLOR_LIGHTMAGENTA  ((uint16_t)0xFC1F)
#define LCD_COLOR_LIGHTYELLOW   ((uint16_t)0xFFF0)
#define LCD_COLOR_DARKBLUE      ((uint16_t)0x0010)
#define LCD_COLOR_DARKGREEN     ((uint16_t)0x0400)
#define LCD_COLOR_DARKRED       ((uint16_t)0x8000)
#define LCD_COLOR_DARKCYAN      ((uint16_t)0x0410)
#define LCD_COLOR_DARKMAGENTA   ((uint16_t)0x8010)
#define LCD_COLOR_DARKYELLOW    ((uint16_t)0x8400)
#define LCD_COLOR_WHITE         ((uint16_t)0xFFFF)
#define LCD_COLOR_LIGHTGRAY     ((uint16_t)0xD69A)
#define LCD_COLOR_GRAY          ((uint16_t)0x8410)
#define LCD_COLOR_DARKGRAY      ((uint16_t)0x4208)
#define LCD_COLOR_BLACK         ((uint16_t)0x0000)
#define LCD_COLOR_BROWN         ((uint16_t)0xA145)
#define LCD_COLOR_ORANGE        ((uint16_t)0xFD20)


/*
 *  LCDハンドラ構造体定義
 */
typedef struct
{
	uint32_t                fmc_base;	/* FMCベースアドレス */
	uint16_t                _width;		/* 幅ピクセル数 */
	uint16_t				_height;	/* 高さピクセル数 */
	uint16_t                winXstart;	/* Windows X start */
	uint16_t                winYstart;	/* Windows Y start */
	uint16_t                winXend;	/* Windows X end */
	uint16_t                winYend;	/* Windows Y end */

	uint32_t                rst_clk;	/* RESET CLOCK BIT */
	uint32_t                te_clk;		/* LCD tearing effect CLOCK BIT */
	uint32_t                bl_clk;		/* LCD Backlight control CLOCK BIT */
	uint32_t                rst_base;	/* RESET PORT */
	uint32_t                te_base;	/* LCD tearing effect PORT */
	uint32_t                bl_base;	/* LCD Backlight control PORT */
	uint32_t                gpio_speed;	/* GPIO SPEED */
	uint32_t                rst_pull;	/* RESET PULL TYPE */
	uint32_t                bl_pull;	/* LCD Backlight control PULL TYPE */
	uint8_t                 rst_pin;	/* RESET PIN */
	uint8_t                 te_pin;		/* LCD tearing effect PIN */
	uint8_t                 bl_pin;		/* LCD Backlight control PIN */
	uint8_t                 bl_active;
}LCD_Handler_t;

/**
  * @}
  */
typedef struct 
{
	LCD_Handler_t           *hlcd;
	uint32_t                TextColor;
	uint32_t                BackColor;
	void                    *pFont;
}LCD_DrawProp_t;


extern ER lcd_init(LCD_Handler_t *hlcd, uint32_t orientation);
extern ER lcd_deinit(LCD_Handler_t *hlcd);

extern ER lcd_layerdefaultinit(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address);
extern void lcd_setAddrWindow(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
extern void lcd_clear(LCD_Handler_t *hlcd, uint16_t Color);
extern uint16_t lcd_readPixel(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos);
extern void lcd_drawPixel(LCD_Handler_t *hlcd, int16_t x, int16_t y, uint16_t color);
extern void lcd_drawFastHLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint32_t color);
extern void lcd_drawFastVLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint32_t color);
extern void lcd_drawBitmap(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
extern void lcd_drawRGBImage(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint8_t *pdata);
extern void lcd_displayOn(LCD_Handler_t *hlcd);
extern void lcd_displayOff(LCD_Handler_t *hlcd);
extern void lcd_fillRect(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint32_t color);

extern void lcd_drawLine(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void lcd_drawRect(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
extern void lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
extern void lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount);
extern void lcd_DrawEllipse(LCD_DrawProp_t *pDrawProp, int Xpos, int Ypos, int XRadius, int YRadius);
extern void lcd_fillCircle(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
extern void lcd_fillEllipse(LCD_DrawProp_t *pDrawProp, int Xpos, int Ypos, int XRadius, int YRadius);
extern void lcd_fillPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount);


#ifdef __cplusplus
}
#endif

#endif /* _STLCD_ST7789H2_H_ */

