/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
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
 *  GRAPHIC-LCDテキスト表示プログラムの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "glcd_disp.h"
#include "font24.c"
#include "font20.c"
#include "font16.c"
#include "font12.c"
#include "font8.c"

/*
 *  文字描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: 文字描画 X座標始点
 *  param3  y0: 文字描画 Y座標始点
 *  param4  c:  文字イメージへのポインタ
 */
#ifndef MAX_FONT_IMAGE
static void
DrawChar(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, const uint8_t *c)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	sFONT    *pFont = pDrawProp->pFont;
	uint32_t h, w, stride;
	uint8_t  *pchar;

	stride = ((pFont->Width + 7)/8);
	for(h = 0 ; h < pFont->Height ; h++){
		pchar = ((uint8_t *)c + stride * h);
		for(w = 0; w < pFont->Width ; w++){
			if(pchar[w/8] & (1 << (7 - (w % 8)))){
				lcd_drawPixel(hlcd, (x0 + w), (y0 + h), pDrawProp->TextColor);
			}
			else{
				lcd_drawPixel(hlcd, (x0 + w), (y0 + h), pDrawProp->BackColor);
			}
		}
	}
}
#else
static void
DrawChar(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, const uint8_t *c)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	sFONT    *pFont = pDrawProp->pFont;
	uint32_t h, w, stride;
	const uint8_t  *pchar;
	color_t  image[MAX_FONT_IMAGE];

	stride = ((pFont->Width + 7)/8);
	for(h = 0 ; h < pFont->Height ; h++){
		pchar = ((uint8_t *)c + stride * h);
		for(w = 0; w < pFont->Width ; w++){
			if(pchar[w/8] & (1 << (7 - (w % 8)))){
				image[w] = pDrawProp->TextColor;
			}
			else{
				image[w] = pDrawProp->BackColor;
			}
		}
		lcd_drawImageHLine(hlcd, x0, (y0 + h), pFont->Width, image);
	}
}
#endif

/*
 *  文字列表示ラインクリア
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  ライン番号
 */
void
lcd_clearStringLine(LCD_DrawProp_t *pDrawProp, uint16_t Line)
{
	sFONT    *pFont = pDrawProp->pFont;
	lcd_fillRect(pDrawProp->hlcd, 0, (Line * pFont->Height), pDrawProp->hlcd->_width, pFont->Height, pDrawProp->BackColor);
}

/*
 *  一文字描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: 文字描画 X座標始点
 *  param3  y0: 文字描画 Y座標始点
 *  param4  描画アスキーコード(範囲は0x20から0x7Eまで)
 */
void
lcd_DisplayChar(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, uint8_t Ascii)
{
	sFONT    *pFont = pDrawProp->pFont;
	DrawChar(pDrawProp, x0, y0, &pFont->table[(Ascii-' ') * pFont->Height * ((pFont->Width + 7) / 8)]);
}

/*
 *  文字列描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param1  x0: 文字列描画 X座標始点
 *  param2  y0: 文字列描画 Y座標始点
 *  param3  Text: 描画文字列へのポインタ
 */
void
lcd_DisplayStringAt(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, uint8_t *Text, Line_ModeTypdef Mode)
{
	sFONT    *pFont = pDrawProp->pFont;
	uint16_t ref_column = 1, i = 0;
	uint32_t size = 0, xsize = 0; 
	uint8_t  *ptr = (uint8_t *)Text;

	/* Get the text size */
	while (*ptr++) size++;

	/* Characters number per line */
	xsize = (pDrawProp->hlcd->_width / pFont->Width);

	switch(Mode){
	case CENTER_MODE:
		ref_column = x0 + ((xsize - size) * pFont->Width) / 2;
		break;
	case LEFT_MODE:
		ref_column = x0;
		break;
	case RIGHT_MODE:
		ref_column =  - x0 + ((xsize - size) * pFont->Width);
		break;
	default:
		ref_column = x0;
		break;
	}

	/*
	 *  開始カラム位置をスクリーン内に収める
	 */
	if((ref_column < 1) || (ref_column >= 0x8000)){
		ref_column = 1;
	}

	/*
	 *  一文字描画を使って文字列を描画する
	 */
	while((*Text != 0) & (((pDrawProp->hlcd->_width - (i*pFont->Width)) & 0xFFFF) >= pFont->Width)){
		lcd_DisplayChar(pDrawProp, ref_column, y0, *Text);
		ref_column += pFont->Width;		/* 次のカラムに移動 */
		Text++;							/* 次の文字に移動 */
		i++;
	}
}

/*
 *  文字列描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: 文字列描画 X座標始点
 *  param3  y0: 文字列描画 Y座標始点
 *  param4  Text: 描画文字列へのポインタ
 */
void
lcd_DisplayStringAtLine(LCD_DrawProp_t *pDrawProp, uint16_t Line, uint8_t *ptr)
{
	sFONT    *pFont = pDrawProp->pFont;

	lcd_DisplayStringAt(pDrawProp, 0, Line * pFont->Height, ptr, LEFT_MODE);
}

