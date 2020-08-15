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

#ifndef _STLCD_RK043FN48H_H_
#define _STLCD_RK043FN48H_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include "ltdc.h"



/* Include LCD component Driver */
/* LCD RK043FN48H-CT672B 4,3" 480x272 pixels */
/**
  * @brief  RK043FN48H Size
  */
#define  RK043FN48H_WIDTH    ((uint16_t)480)          /* LCD PIXEL WIDTH            */
#define  RK043FN48H_HEIGHT   ((uint16_t)272)          /* LCD PIXEL HEIGHT           */

/**
  * @brief  RK043FN48H Timing
  */
#define  RK043FN48H_HSYNC            ((uint16_t)41)   /* Horizontal synchronization */
#define  RK043FN48H_HBP              ((uint16_t)13)   /* Horizontal back porch      */
#define  RK043FN48H_HFP              ((uint16_t)32)   /* Horizontal front porch     */
#define  RK043FN48H_VSYNC            ((uint16_t)10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP              ((uint16_t)2)    /* Vertical back porch        */
#define  RK043FN48H_VFP              ((uint16_t)2)    /* Vertical front porch       */

/**
  * @brief  RK043FN48H frequency divider
  */
#define  RK043FN48H_FREQUENCY_DIVIDER    5            /* LCD Frequency divider      */

#define MAX_LAYER_NUMBER       2

#define LTDC_ACTIVE_LAYER	     ((uint32_t)1) /* Layer 1 */

/**
  * @brief  LCD FB_StartAddress
  */
#define LCD_FB_START_ADDRESS       ((uint32_t)0xC0000000)

/**
  * @brief  LCD color
  */
#define LCD_COLOR_BLUE          ((uint32_t)0xFF0000FF)
#define LCD_COLOR_GREEN         ((uint32_t)0xFF00FF00)
#define LCD_COLOR_RED           ((uint32_t)0xFFFF0000)
#define LCD_COLOR_CYAN          ((uint32_t)0xFF00FFFF)
#define LCD_COLOR_MAGENTA       ((uint32_t)0xFFFF00FF)
#define LCD_COLOR_YELLOW        ((uint32_t)0xFFFFFF00)
#define LCD_COLOR_LIGHTBLUE     ((uint32_t)0xFF8080FF)
#define LCD_COLOR_LIGHTGREEN    ((uint32_t)0xFF80FF80)
#define LCD_COLOR_LIGHTRED      ((uint32_t)0xFFFF8080)
#define LCD_COLOR_LIGHTCYAN     ((uint32_t)0xFF80FFFF)
#define LCD_COLOR_LIGHTMAGENTA  ((uint32_t)0xFFFF80FF)
#define LCD_COLOR_LIGHTYELLOW   ((uint32_t)0xFFFFFF80)
#define LCD_COLOR_DARKBLUE      ((uint32_t)0xFF000080)
#define LCD_COLOR_DARKGREEN     ((uint32_t)0xFF008000)
#define LCD_COLOR_DARKRED       ((uint32_t)0xFF800000)
#define LCD_COLOR_DARKCYAN      ((uint32_t)0xFF008080)
#define LCD_COLOR_DARKMAGENTA   ((uint32_t)0xFF800080)
#define LCD_COLOR_DARKYELLOW    ((uint32_t)0xFF808000)
#define LCD_COLOR_WHITE         ((uint32_t)0xFFFFFFFF)
#define LCD_COLOR_LIGHTGRAY     ((uint32_t)0xFFD3D3D3)
#define LCD_COLOR_GRAY          ((uint32_t)0xFF808080)
#define LCD_COLOR_DARKGRAY      ((uint32_t)0xFF404040)
#define LCD_COLOR_BLACK         ((uint32_t)0xFF000000)
#define LCD_COLOR_BROWN         ((uint32_t)0xFFA52A2A)
#define LCD_COLOR_ORANGE        ((uint32_t)0xFFFFA500)
#define LCD_COLOR_TRANSPARENT   ((uint32_t)0xFF000000)


/*
 *  LCDハンドラ構造体定義
 */
typedef struct
{
	LTDC_Handle_t           *hltdc;		/* LTDCハンドラ */
	uint32_t                layer;
	uint16_t                _width;		/* 幅ピクセル数 */
	uint16_t				_height;	/* 高さピクセル数 */
} LCD_Handler_t;

/** @defgroup STM32746G_DISCOVERY_LCD_Exported_Types STM32746G_DISCOVERY_LCD Exported Types
  * @{
  */
typedef struct
{
	LCD_Handler_t           *hlcd;
	uint32_t TextColor;		/* Specifies the color of text */
	uint32_t BackColor;		/* Specifies the background color below the text */
	void     *pFont;		/* Specifies the font used for the text */

} LCD_DrawProp_t;

/**
  * @brief  LCD Drawing point (pixel) geometric definition
  */
typedef struct
{
  int16_t X; /*!< geometric X position of drawing */
  int16_t Y; /*!< geometric Y position of drawing */

} Point;

/**
  * @brief  Pointer on LCD Drawing point (pixel) geometric definition
  */
typedef Point * pPoint;


/**
 *  @brief LCD_OrientationTypeDef
 *  Possible values of Display Orientation
 */
typedef enum
{
	LCD_ORIENTATION_PORTRAIT  = 0x00, /*!< Portrait orientation choice of LCD screen  */
	LCD_ORIENTATION_LANDSCAPE = 0x01, /*!< Landscape orientation choice of LCD screen */
	LCD_ORIENTATION_INVALID   = 0x02  /*!< Invalid orientation choice of LCD screen   */
} LCD_OrientationTypeDef;

/** @addtogroup STM32746G_DISCOVERY_LCD_Exported_Functions
  * @{
  */
extern ER lcd_init(LCD_Handler_t *hlcd, LCD_OrientationTypeDef orientation);
extern ER lcd_layerdefaultinit(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address);
extern ER lcd_layerRgb565init(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address);
extern void lcd_setlayervisible(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint8_t State);
extern void lcd_settransparency(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint8_t Transparency);
extern void lcd_setlayeraddress(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint32_t Address);
extern void lcd_setcolorkeying(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint32_t RGBValue);
extern void lcd_resetcolorkeying(LCD_Handler_t *hlcd, uint32_t LayerIndex);

extern void lcd_setAddrWindow(LCD_Handler_t *hlcd, uint16_t LayerIndex, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
extern void lcd_clear(LCD_Handler_t *hlcd, uint32_t Color);
extern uint32_t lcd_readPixel(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos);
extern void lcd_drawPixel(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code);
extern void lcd_drawFastHLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint32_t color);
extern void lcd_drawFastVLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint32_t color);
extern void lcd_drawBitmap(LCD_Handler_t *hlcd, uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);
extern void lcd_fillRect(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint32_t color);

extern void lcd_drawLine(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void lcd_drawRect(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
extern void lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
extern void lcd_DrawEllipse(LCD_DrawProp_t *pDrawProp, int Xpos, int Ypos, int XRadius, int YRadius);
extern void lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount);
extern void lcd_fillCircle(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
extern void lcd_fillEllipse(LCD_DrawProp_t *pDrawProp, int Xpos, int Ypos, int XRadius, int YRadius);
extern void lcd_fillPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount);


#ifdef __cplusplus
}
#endif

#endif /* _STLCD_RK043FN48H_H_ */

