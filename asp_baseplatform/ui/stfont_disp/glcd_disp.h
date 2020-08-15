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

#ifndef _GLCD_DISP_H_
#define _GLCD_DISP_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include "glcd_core.h"
#include "fonts.h"

/*
 *  テキスト表示ラインモード定義
 */
typedef enum
{
	CENTER_MODE = 0x01,	/* Center mode */
	RIGHT_MODE,			/* Right mode  */
	LEFT_MODE			/* Left mode   */
}Line_ModeTypdef;


/*
 *  BSP表示関数変換定義
 */
#define BSP_LCD_Init()                  lcd_init(pHLCD, LCD_ORIENTATION_LANDSCAPE)
#define BSP_LCD_DeInit()                lcd_deinit(pHLCD)
#define BSP_LCD_LayerDefaultInit(a, b)  lcd_layerdefaultinit(pHLCD, pDRAWPROP, (a), (b))
#define BSP_LCD_SetLayerWindow(i, x, y, w, h) lcd_setAddrWindow(pHLCD, (i), (x), (y), (w), (h))

#define BSP_LCD_Clear(c)                lcd_clear(pHLCD, (c))
#define BSP_LCD_ReadPixel(x, y)         lcd_readPixel(pHLCD, (x), (y))
#define BSP_LCD_DrawPixel(x, y, c)      lcd_drawPixel(pHLCD, (x), (y), (c))
#define BSP_LCD_DrawHLine(x, y, w)      lcd_drawFastHLine(pHLCD, (x), (y), (w), pDRAWPROP->TextColor)
#define BSP_LCD_DrawVLine(x, y, h)      lcd_drawFastVLine(pHLCD, (x), (y), (h), pDRAWPROP->TextColor)
#define BSP_LCD_DrawBitmap(x, y, p)     lcd_drawBitmap(pHLCD, (x), (y), (p))
#define BSP_LCD_DrawRGBImage(x, y, xs, ys, p) lcd_drawRGBImage(pHLCD, (x), (y), (xs), (xy), (p))
#define BSP_LCD_DisplayOn()             lcd_displayOn(pHLCD)
#define BSP_LCD_DisplayOff()            lcd_displayOff(pHLCD)
#define BSP_LCD_FillRect(x, y, w,h)     lcd_fillRect(pHLCD, (x), (y), (w), (h), pDRAWPROP->TextColor)

#define BSP_LCD_DrawLine(x, y, x2, y2)  lcd_drawLine(pDRAWPROP, (x), (y), (x2), (y2))
#define BSP_LCD_DrawRect(x, y, w, h)    lcd_drawRect(pDRAWPROP, (x), (y), (w), (h))
#define BSP_LCD_DrawCircle(x, y, r)     lcd_DrawCircle(pDRAWPROP, (x), (y), (r))
#define BSP_LCD_DrawEllipse(x, y, xr, yr) lcd_DrawEllipse(pDRAWPROP, (x), (y), (xr), (yr))
#define BSP_LCD_DrawPolygon(a, b)       lcd_drawPolygon(pDRAWPROP, (a), (b))
#define BSP_LCD_FillCircle(x, y, r)     lcd_fillCircle(pDRAWPROP, (x), (y), (r))
#define BSP_LCD_FillEllipse(x, y, xr, yr) lcd_fillEllipse(pDRAWPROP, (x), (y), (xr), (yr))
#define BSP_LCD_FillPolygon(a, b)       lcd_fillPolygon(pDRAWPROP, (a), (b))

#define BSP_LCD_ClearStringLine(l)      lcd_clearStringLine(pDRAWPROP, (l))
#define BSP_LCD_DisplayChar(x, y, a)    lcd_DisplayChar(pDRAWPROP, (x), (y), (a))
#define BSP_LCD_DisplayStringAt(x, y, t, m) lcd_DisplayStringAt(pDRAWPROP, (x), (y), (t), (m))
#define BSP_LCD_DisplayStringAtLine(l, p) lcd_DisplayStringAtLine(pDRAWPROP, (l), (p))

#define BSP_LCD_SelectLayer(a)          pHLCD->layer = (a)
#define BSP_LCD_GetXSize()              pHLCD->_width
#define BSP_LCD_GetYSize()              pHLCD->_height
#define BSP_LCD_SetTextColor(c)         pDRAWPROP->TextColor = (c)
#define BSP_LCD_GetTextColor()          pDRAWPROP->TextColor
#define BSP_LCD_SetBackColor(c)         pDRAWPROP->BackColor = (c)
#define BSP_LCD_GetBackColor()          pDRAWPROP->BackColor
#define BSP_LCD_SetFont(f)              pDRAWPROP->pFont = (f)
#define BSP_LCD_GetFont()               ((sFONT *)pDRAWPROP->pFont)


extern void lcd_clearStringLine(LCD_DrawProp_t *pDrawProp, uint16_t Line);
extern void lcd_DisplayChar(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);
extern void lcd_DisplayStringAt(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Line_ModeTypdef Mode);
extern void lcd_DisplayStringAtLine(LCD_DrawProp_t *pDrawProp, uint16_t Line, uint8_t *ptr);

#ifdef __cplusplus
}
#endif

#endif /* _GLCD_DISP_H_ */

