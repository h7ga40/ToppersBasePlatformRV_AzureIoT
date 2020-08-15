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

#include "stlcd_rk043fn48h.h"
#include <sil.h>
#include <target_syssvc.h>

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

#define POLY_X(Z)              ((int32_t)((Points + Z)->X))
#define POLY_Y(Z)              ((int32_t)((Points + Z)->Y))
#define ABS(X)                 ((X) > 0 ? (X) : -(X))

#ifndef ENABLE
#define ENABLE                 1
#endif

static LTDC_Handle_t  hLtdcHandler;
static DMA2D_Handle_t hDma2dHandler;

static void FillTriangle(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);
static void LL_FillBuffer(LCD_Handler_t *hlcd, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
static void LL_ConvertLineToARGB8888(void * pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode);

/*
 *  LTDC/RK043FN48Hスクリーンの初期化関数
 */
ER
lcd_init(LCD_Handler_t *hlcd, LCD_OrientationTypeDef orientation)
{
	/* The RK043FN48H LCD 480x272 is selected */
	/* Timing Configuration */
	hLtdcHandler.Init.HorizontalSync = (RK043FN48H_HSYNC - 1);
	hLtdcHandler.Init.VerticalSync = (RK043FN48H_VSYNC - 1);
	hLtdcHandler.Init.AccumulatedHBP = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	hLtdcHandler.Init.AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hLtdcHandler.Init.AccumulatedActiveH = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hLtdcHandler.Init.AccumulatedActiveW = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	hLtdcHandler.Init.TotalHeigh = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
	hLtdcHandler.Init.TotalWidth = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);
	hLtdcHandler.Init.pllsain = 192;
	hLtdcHandler.Init.pllsair = 5;
	hLtdcHandler.Init.saidivr = RCC_DCKCFGR1_PLLSAIDIVR_0;	/* DIV 4 */

	/* Initialize the LCD pixel width and pixel height */
	hLtdcHandler.LayerCfg[0].ImageWidth  = RK043FN48H_WIDTH;
	hLtdcHandler.LayerCfg[0].ImageHeight = RK043FN48H_HEIGHT;
#if MAX_LAYER_NUMBER >= 2
	hLtdcHandler.LayerCfg[1].ImageWidth  = RK043FN48H_WIDTH;
	hLtdcHandler.LayerCfg[1].ImageHeight = RK043FN48H_HEIGHT;
#endif

	/* Background value */
	hLtdcHandler.Init.Backcolor.Blue = 0;
	hLtdcHandler.Init.Backcolor.Green = 0;
	hLtdcHandler.Init.Backcolor.Red = 0;

	/* Polarity */
	hLtdcHandler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hLtdcHandler.Init.VSPolarity = LTDC_VSPOLARITY_AL; 
	hLtdcHandler.Init.DEPolarity = LTDC_DEPOLARITY_AL;  
	hLtdcHandler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hLtdcHandler.base = TADR_LTDC_BASE;
	hlcd->_width  = RK043FN48H_WIDTH;
	hlcd->_height = RK043FN48H_HEIGHT;
	ltdc_init(&hLtdcHandler);
	hlcd->hltdc = &hLtdcHandler;
	return E_OK;
}

/*
 *  LTDC/RK043FN48Hスクリーンレイヤ初期化関数
 */
ER
lcd_layerdefaultinit(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address)
{
	LTDC_LayerCfg_t  layer_cfg;

	if(LayerIndex >= MAX_LAYER_NUMBER)
		return E_PAR;
	/* Layer Init */
	layer_cfg.WindowX0 = 0;
	layer_cfg.WindowX1 = hlcd->hltdc->LayerCfg[LayerIndex].ImageWidth;
	layer_cfg.WindowY0 = 0;
	layer_cfg.WindowY1 = hlcd->hltdc->LayerCfg[LayerIndex].ImageHeight;
	layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	layer_cfg.FBStartAdress = FB_Address;
	layer_cfg.Alpha = 255;
	layer_cfg.Alpha0 = 0;
	layer_cfg.Backcolor.Blue = 0;
	layer_cfg.Backcolor.Green = 0;
	layer_cfg.Backcolor.Red = 0;
	layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	layer_cfg.ImageWidth = hlcd->hltdc->LayerCfg[LayerIndex].ImageWidth;
	layer_cfg.ImageHeight = hlcd->hltdc->LayerCfg[LayerIndex].ImageHeight;

	ltdc_configlayer(&hLtdcHandler, &layer_cfg, LayerIndex);

	hlcd->layer = LayerIndex;
	pDrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;
	pDrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK;
	pDrawProp[LayerIndex].hlcd      = hlcd;
	return E_OK;
}

/*
 *  LTDC/RK043FN48HスクリーンRGB565レイヤ初期化関数
 */
ER
lcd_layerRgb565init(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address)
{
	LTDC_LayerCfg_t  layer_cfg;

	if(LayerIndex >= MAX_LAYER_NUMBER)
		return E_PAR;
	/* Layer Init */
	layer_cfg.WindowX0 = 0;
	layer_cfg.WindowX1 = hlcd->hltdc->LayerCfg[LayerIndex].ImageWidth;
	layer_cfg.WindowY0 = 0;
	layer_cfg.WindowY1 = hlcd->hltdc->LayerCfg[LayerIndex].ImageHeight;
	layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	layer_cfg.FBStartAdress = FB_Address;
	layer_cfg.Alpha = 255;
	layer_cfg.Alpha0 = 0;
	layer_cfg.Backcolor.Blue = 0;
	layer_cfg.Backcolor.Green = 0;
	layer_cfg.Backcolor.Red = 0;
	layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	layer_cfg.ImageWidth = hlcd->hltdc->LayerCfg[LayerIndex].ImageWidth;
	layer_cfg.ImageHeight = hlcd->hltdc->LayerCfg[LayerIndex].ImageHeight;

	ltdc_configlayer(&hLtdcHandler, &layer_cfg, LayerIndex);

	hlcd->layer = LayerIndex;
	pDrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;
	pDrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK;
	pDrawProp[LayerIndex].hlcd      = hlcd;
	return E_OK;
}

/*
 *  LTDC/RK043FN48Hスクリーンレイヤ表示設定関数
 */
void
lcd_setlayervisible(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint8_t State)
{
	uint32_t layer_base = ((uint32_t)TADR_LTDC_BASE + 0x84 + (0x80 * LayerIndex));
	if(State == ENABLE){
		sil_orw_mem((uint32_t *)(layer_base+TOFF_LTDCW_CR), LTDC_CR_LEN);
	}
	else{
		sil_andw_mem((uint32_t *)(layer_base+TOFF_LTDCW_CR), LTDC_CR_LEN);
	}
	sil_orw_mem((uint32_t *)(TADR_LTDC_BASE+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
}

/*
 *  LTDC/RK043FN48Hスクリーンレイヤ透過設定関数
*
  * @brief  Configures the transparency.
  * @param  LayerIndex: Layer foreground or background.
  * @param  Transparency: Transparency
  *           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF 
  * @retval None
  */
void
lcd_settransparency(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint8_t Transparency)
{
	ltdc_setalpha(hlcd->hltdc, Transparency, LayerIndex);
}

/*
 *  LTDC/RK043FN48HスクリーンレイヤRAMアドレス設定関数
 */
void
lcd_setlayeraddress(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint32_t Address)
{
	ltdc_setaddress(hlcd->hltdc, Address, LayerIndex);
}

/*
 *  LTDC/RK043FN48H表示ウィンドウ設定
 */
void
lcd_setAddrWindow(LCD_Handler_t *hlcd, uint16_t LayerIndex, uint16_t x0, uint16_t y0, uint16_t Width, uint16_t Height)
{
	/* Reconfigure the layer size */
	ltdc_setwindowsize(hlcd->hltdc, Width, Height, LayerIndex);

	/* Reconfigure the layer position */
	ltdc_setwindowposition(hlcd->hltdc, x0, y0, LayerIndex);
}

/**
  * @brief  Configures and sets the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @param  RGBValue: Color reference
  * @retval None
  */
void
lcd_setcolorkeying(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint32_t RGBValue)
{
	/* Configure and Enable the color Keying for LCD Layer */
	ltdc_configcolorkeying(hlcd->hltdc, RGBValue, LayerIndex);
	ltdc_enablecolorkeying(hlcd->hltdc, LayerIndex);
}

/**
  * @brief  Disables the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @retval None
  */
void
lcd_resetcolorkeying(LCD_Handler_t *hlcd, uint32_t LayerIndex)
{
	/* Disable the color Keying for LCD Layer */
	ltdc_disablecolorkeying(hlcd->hltdc, LayerIndex);
}

/*
 *  LTDC/RK043FN48Hスクリーンクリア
 */
void
lcd_clear(LCD_Handler_t *hlcd, uint32_t Color)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t layer = hlcd->layer;
	uint32_t width, height;

	width  = hltdc->LayerCfg[layer].ImageWidth;
	height = hltdc->LayerCfg[layer].ImageHeight;
 	/* Clear the LCD */
 	LL_FillBuffer(hlcd, (uint32_t *)(hltdc->LayerCfg[layer].FBStartAdress), width, height, 0, Color);
}

/*
 *  PIXEL読み出し
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 */
uint32_t
lcd_readPixel(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t layer = hlcd->layer;
	uint32_t ret = 0;
	uint32_t width;

	width = hltdc->LayerCfg[layer].ImageWidth;
	if(hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888){
		/* Read data value from SDRAM memory */
		ret = *(volatile uint32_t*)(hltdc->LayerCfg[layer].FBStartAdress + (2*(Ypos * width + Xpos)));
	}
	else if(hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB888){
		/* Read data value from SDRAM memory */
		ret = (*(volatile uint32_t*)(hltdc->LayerCfg[layer].FBStartAdress + (2*(Ypos * width + Xpos))) & 0x00FFFFFF);
	}
	else if((hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
			|| (hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444)
			|| (hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_AL88)){
		/* Read data value from SDRAM memory */
		ret = *(volatile uint16_t*)(hltdc->LayerCfg[layer].FBStartAdress + (2*(Ypos * width + Xpos)));
	}
	else{
		/* Read data value from SDRAM memory */
		ret = *(volatile uint8_t*)(hltdc->LayerCfg[layer].FBStartAdress + (2*(Ypos * width + Xpos)));
	}
	return ret;
}

/*
 *  PIXEL描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 *  param4  color: color value
 */
void
lcd_drawPixel(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t layer = hlcd->layer;

	/* Write data value to all SDRAM memory */
	if(hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565){ /* RGB565 format */
		*(volatile uint16_t*)(hltdc->LayerCfg[layer].FBStartAdress + (2*(Ypos*hltdc->LayerCfg[layer].ImageWidth + Xpos))) = (uint16_t)RGB_Code;
	}
	else{	/* ARGB8888 format */
		*(volatile uint32_t*)(hltdc->LayerCfg[layer].FBStartAdress + (4*(Ypos*hltdc->LayerCfg[layer].ImageWidth + Xpos))) = RGB_Code;
	}
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
lcd_drawFastHLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint32_t color)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t layer = hlcd->layer;
	uint32_t Xaddress = 0, width;

	width = hltdc->LayerCfg[layer].ImageWidth;
	/* Get the line address */
	if(hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565){	/* RGB565 format */
		Xaddress = (hltdc->LayerCfg[layer].FBStartAdress) + 2*(width * Ypos + Xpos);
	}
	else{	/* ARGB8888 format */
		Xaddress = (hltdc->LayerCfg[layer].FBStartAdress) + 4*(width * Ypos + Xpos);
	}
	/* Write line */
	LL_FillBuffer(hlcd, (uint32_t *)Xaddress, Length, 1, 0, color);
}

/*
 *  水平方向LINEの高速描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  w: width
 *  param5  color: color value
 */
void
lcd_drawFastVLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint32_t color)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t layer = hlcd->layer;
	uint32_t  Xaddress = 0, width;

	width = hltdc->LayerCfg[layer].ImageWidth;
	/* Get the line address */
	if(hltdc->LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565){	/* RGB565 format */
		Xaddress = (hltdc->LayerCfg[layer].FBStartAdress) + 2*(width * Ypos + Xpos);
	}
	else{	/* ARGB8888 format */
		Xaddress = (hltdc->LayerCfg[layer].FBStartAdress) + 4*(width*Ypos + Xpos);
	}

	/* Write line */
	LL_FillBuffer(hlcd, (uint32_t *)Xaddress, 1, Length, (width - 1), color);
}

/*
 *  BITMAP描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x0: Bmp X position in the LCD
 *  param3  y0: Bmp Y position in the LCD
 *  param4  pbmp: Pointer to Bmp picture address in the internal Flash
 */
void
lcd_drawBitmap(LCD_Handler_t *hlcd, uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t layer = hlcd->layer;
	uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
	uint32_t address, iwidth;
	uint32_t input_color_mode = 0;

	iwidth = hltdc->LayerCfg[hlcd->layer].ImageWidth;
	/* Get bitmap data address offset */
	index = *(volatile uint16_t *) (pbmp + 10);
	index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;

	/* Read bitmap width */
	width = *(volatile uint16_t *) (pbmp + 18);
	width |= (*(volatile uint16_t *) (pbmp + 20)) << 16;

	/* Read bitmap height */
	height = *(volatile uint16_t *) (pbmp + 22);
	height |= (*(volatile uint16_t *) (pbmp + 24)) << 16; 

	/* Read bit/pixel */
	bit_pixel = *(uint16_t *) (pbmp + 28);

	/* Set the address */
	address = hltdc->LayerCfg[layer].FBStartAdress + (((hltdc->LayerCfg[layer].ImageWidth*Ypos) + Xpos)*(4));

	/* Get the layer pixel format */
	if((bit_pixel/8) == 4){
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

	/* Convert picture to ARGB8888 pixel format */
	for(index=0; index < height; index++){
		/* Pixel format conversion */
		LL_ConvertLineToARGB8888((uint32_t *)pbmp, (uint32_t *)address, width, input_color_mode);

		/* Increment the source and destination buffers */
		address+=  (iwidth*4);
		pbmp -= width*(bit_pixel/8);
	}
}

/*
 *  RECTANGLE塗りつぶし描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: left X position
 *  param3  y: top Y position
 *  param4  w: width
 *  param5  h: height
 *  param6  color: color value
 */
void
lcd_fillRect(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint32_t color)
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;
	uint32_t  x_address = 0;
	uint32_t width;

	width = hltdc->LayerCfg[hlcd->layer].ImageWidth;

	/* Get the rectangle start address */
	if(hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565){	/* RGB565 format */
		x_address = (hltdc->LayerCfg[hlcd->layer].FBStartAdress) + 2*(width * Ypos + Xpos);
	}
	else{	/* ARGB8888 format */
		x_address = (hltdc->LayerCfg[hlcd->layer].FBStartAdress) + 4*(width * Ypos + Xpos);
	}
	/* Fill the rectangle */
	LL_FillBuffer(hlcd, (uint32_t *)x_address, Width, Height, (width - Width), color);
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

	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

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

	for(curpixel = 0; curpixel <= num_pixels; curpixel++){
		lcd_drawPixel(hlcd, x, y, pDrawProp[hlcd->layer].TextColor);   /* Draw the current pixel */
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
 *  RECTANGLE描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x: left X position
 *  param3  y: top Y position
 *  param4  w: width
 *  param5  h: height
 */
void
lcd_drawRect(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	uint32_t      color = pDrawProp[hlcd->layer].TextColor;
	/* Draw horizontal lines */
	lcd_drawFastHLine(hlcd, Xpos, Ypos, Width, color);
	lcd_drawFastHLine(hlcd, Xpos, (Ypos+ Height), Width, color);

	/* Draw vertical lines */
	lcd_drawFastVLine(hlcd, Xpos, Ypos, Height, color);
	lcd_drawFastVLine(hlcd, (Xpos + Width), Ypos, Height, color);
}

/*
 *  円描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: X position
 *  param3  y0: Y position
 *  param4  Radius: Circle radius
 */
void
lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	uint32_t      color = pDrawProp[hlcd->layer].TextColor;
	int32_t   decision;    /* Decision Variable */ 
	uint32_t  current_x;   /* Current X Value */
	uint32_t  current_y;   /* Current Y Value */

	decision = 3 - (Radius << 1);
	current_x = 0;
	current_y = Radius;

	while (current_x <= current_y){
		lcd_drawPixel(hlcd, (Xpos + current_x), (Ypos - current_y), color);
		lcd_drawPixel(hlcd, (Xpos - current_x), (Ypos - current_y), color);
		lcd_drawPixel(hlcd, (Xpos + current_y), (Ypos - current_x), color);
		lcd_drawPixel(hlcd, (Xpos - current_y), (Ypos - current_x), color);
		lcd_drawPixel(hlcd, (Xpos + current_x), (Ypos + current_y), color);
		lcd_drawPixel(hlcd, (Xpos - current_x), (Ypos + current_y), color);
		lcd_drawPixel(hlcd, (Xpos + current_y), (Ypos + current_x), color);
		lcd_drawPixel(hlcd, (Xpos - current_y), (Ypos + current_x), color);

		if(decision < 0){
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
 *  楕円描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: X position
 *  param3  y0: Y position
 *  param4  XRadius: Ellipse X radius
 *  param5  YRadius: Ellipse Y radius
 */
void
lcd_DrawEllipse(LCD_DrawProp_t *pDrawProp, int Xpos, int Ypos, int XRadius, int YRadius)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	uint32_t      color = pDrawProp[hlcd->layer].TextColor;
	int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
	float k = 0, rad1 = 0, rad2 = 0;

	rad1 = XRadius;
	rad2 = YRadius;
	k = (float)(rad2/rad1);

	do{
		lcd_drawPixel(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos+y), color);
		lcd_drawPixel(hlcd, (Xpos+(uint16_t)(x/k)), (Ypos+y), color);
		lcd_drawPixel(hlcd, (Xpos+(uint16_t)(x/k)), (Ypos-y), color);
		lcd_drawPixel(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos-y), color);

		e2 = err;
		if(e2 <= x){
			err += ++x*2+1;
			if(-y == x && e2 <= y)
				e2 = 0;
		}
		if(e2 > y)
			err += ++y*2+1;
	}
	while (y <= 0);
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

/*
 *  円塗りつぶし描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: X position
 *  param3  y0: Y position
 *  param4  Radius: Circle radius
 */
void
lcd_fillCircle(LCD_DrawProp_t *pDrawProp, uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	uint32_t  color = pDrawProp->TextColor;
	int32_t   decision;     /* Decision Variable */ 
	uint32_t  current_x;   /* Current X Value */
	uint32_t  current_y;   /* Current Y Value */

	decision = 3 - (Radius << 1);
	current_x = 0;
	current_y = Radius;

	while (current_x <= current_y){
		if(current_y > 0){
			lcd_drawFastHLine(hlcd, Xpos - current_y, Ypos + current_x, 2*current_y, color);
			lcd_drawFastHLine(hlcd, Xpos - current_y, Ypos - current_x, 2*current_y, color);
		}
		if(current_x > 0){
			lcd_drawFastHLine(hlcd, Xpos - current_x, Ypos - current_y, 2*current_x, color);
			lcd_drawFastHLine(hlcd, Xpos - current_x, Ypos + current_y, 2*current_x, color);
		}
		if(decision < 0){
			decision += (current_x << 2) + 6;
		}
		else{
			decision += ((current_x - current_y) << 2) + 10;
			current_y--;
		}
		current_x++;
	}
	lcd_DrawCircle(pDrawProp, Xpos, Ypos, Radius);
}

/*
 *  楕円塗りつぶし描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: X position
 *  param3  y0: Y position
 *  param4  XRadius: Ellipse X radius
 *  param5  YRadius: Ellipse Y radius
 */
void
lcd_fillEllipse(LCD_DrawProp_t *pDrawProp, int Xpos, int Ypos, int XRadius, int YRadius)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	uint32_t      color = pDrawProp[hlcd->layer].TextColor;
	int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
	float k = 0, rad1 = 0, rad2 = 0;

	rad1 = XRadius;
	rad2 = YRadius;
	k = (float)(rad2/rad1);

	do{
		lcd_drawFastHLine(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos+y), (2*(uint16_t)(x/k) + 1), color);
		lcd_drawFastHLine(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos-y), (2*(uint16_t)(x/k) + 1), color);

		e2 = err;
		if(e2 <= x){
			err += ++x*2+1;
			if(-y == x && e2 <= y)
				e2 = 0;
		}
		if(e2 > y)
			err += ++y*2+1;
	}while(y <= 0);
}

/*
 *  PLOY-LINE塗りつぶし描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  Points: Pointer to the points array
 *  param3  PointCount: Number of points
 */
void
lcd_fillPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount)
{
	int16_t X = 0, Y = 0, X2 = 0, Y2 = 0, X_center = 0, Y_center = 0, X_first = 0, Y_first = 0, pixelX = 0, pixelY = 0, counter = 0;
	uint16_t  image_left = 0, image_right = 0, image_top = 0, image_bottom = 0;

	image_left = image_right = Points->X;
	image_top= image_bottom = Points->Y;

	for(counter = 1; counter < PointCount; counter++){
		pixelX = POLY_X(counter);
		if(pixelX < image_left){
			image_left = pixelX;
		}
		if(pixelX > image_right){
			image_right = pixelX;
		}

		pixelY = POLY_Y(counter);
		if(pixelY < image_top){
			image_top = pixelY;
		}
		if(pixelY > image_bottom){
			image_bottom = pixelY;
		}
	}

	if(PointCount < 2){
		return;
	}

	X_center = (image_left + image_right)/2;
	Y_center = (image_bottom + image_top)/2;
	X_first = Points->X;
	Y_first = Points->Y;

	while(--PointCount){
		X = Points->X;
		Y = Points->Y;
		Points++;
		X2 = Points->X;
		Y2 = Points->Y;

		FillTriangle(pDrawProp, X, X2, X_center, Y, Y2, Y_center);
		FillTriangle(pDrawProp, X, X_center, X2, Y, Y_center, Y2);
		FillTriangle(pDrawProp, X_center, X2, X, Y_center, Y2, Y);
	}

	FillTriangle(pDrawProp, X_first, X2, X_center, Y_first, Y2, Y_center);
	FillTriangle(pDrawProp, X_first, X_center, X2, Y_first, Y_center, Y2);
	FillTriangle(pDrawProp, X_center, X2, X_first, Y_center, Y2, Y_first);
}


/*******************************************************************************
                            Static Functions
*******************************************************************************/

/*
 *  トライアングルフィル
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x1: 点1 X座標始点
 *  param3  y1: 点1 Y座標始点
 *  param4  x2: 点2 X座標始点
 *  param5  y2: 点2 Y座標始点
 *  param6  x3: 点3 X座標始点
 *  param7  y3: 点3 Y座標始点
 */
static void
FillTriangle(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, num_add = 0, num_pixels = 0,
	curpixel = 0;

	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

	if(x2 >= x1){			/* The x-values are increasing */
		xinc1 = 1;
		xinc2 = 1;
	}
	else{					/* The x-values are decreasing */
		xinc1 = -1;
		xinc2 = -1;
	}

	if(y2 >= y1){			/* The y-values are increasing */
		yinc1 = 1;
		yinc2 = 1;
	}
	else{					/* The y-values are decreasing */
		yinc1 = -1;
		yinc2 = -1;
	}

	if(deltax >= deltay){	/* There is at least one x-value for every y-value */
		xinc1 = 0;			/* Don't change the x when numerator >= denominator */
		yinc2 = 0;			/* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		num_add = deltay;
		num_pixels = deltax;	/* There are more x-values than y-values */
	}
	else{					/* There is at least one y-value for every x-value */
		xinc2 = 0;			/* Don't change the x for every iteration */
		yinc1 = 0;			/* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		num_add = deltax;
		num_pixels = deltay;	/* There are more y-values than x-values */
	}

	for(curpixel = 0; curpixel <= num_pixels; curpixel++){
		lcd_drawLine(pDrawProp, x, y, x3, y3);
		num += num_add;		/* Increase the numerator by the top of the fraction */
		if(num >= den){		/* Check if numerator >= denominator */
			num -= den;		/* Calculate the new numerator value */
			x += xinc1;		/* Change the x as appropriate */
			y += yinc1;		/* Change the y as appropriate */
		}
		x += xinc2;			/* Change the x as appropriate */
		y += yinc2;			/* Change the y as appropriate */
	}
}

/*
 *  バッファフィル
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  pDst:バッファ先頭ポインタ
 *  param3  xSize: バッファ幅
 *  param3  ySize: バッファ高さ
 *  param4  OffLine: 出力オフセット
 *  param5  ColorIndex: フィルカラー
 */
static void
LL_FillBuffer(LCD_Handler_t *hlcd, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) 
{
	LTDC_Handle_t *hltdc = hlcd->hltdc;

	/* Register to memory mode with ARGB8888 as color Mode */ 
	hDma2dHandler.Init.Mode         = DMA2D_R2M;
	if(hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565){	/* RGB565 format */ 
		hDma2dHandler.Init.ColorMode    = DMA2D_RGB565;
	}
	else{	/* ARGB8888 format */
		hDma2dHandler.Init.ColorMode    = DMA2D_ARGB8888;
	}
	hDma2dHandler.Init.OutputOffset = OffLine;

	hDma2dHandler.base = (uint32_t)TADR_DMA2D_BASE;

	/* DMA2D Initialization */
	if(dma2d_init(&hDma2dHandler) == E_OK){
		if(dma2d_configlayer(&hDma2dHandler, hlcd->layer) == E_OK){
			if(dma2d_start(&hDma2dHandler, ColorIndex, (uint32_t)pDst, xSize, ySize) == E_OK){
				/* Polling For DMA transfer */
				dma2d_waittransfar(&hDma2dHandler, 10);
			}
		}
	}
}

/**
  * @brief  Converts a line to an ARGB8888 pixel format.
  * @param  pSrc: Pointer to source buffer
  * @param  pDst: Output color
  * @param  xSize: Buffer width
  * @param  ColorMode: Input color mode   
  * @retval None
  */
static void
LL_ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
{
	/* Configure the DMA2D Mode, Color Mode and output offset */
	hDma2dHandler.Init.Mode         = DMA2D_M2M_PFC;
	hDma2dHandler.Init.ColorMode    = DMA2D_ARGB8888;
	hDma2dHandler.Init.OutputOffset = 0;

	/* Foreground Configuration */
	hDma2dHandler.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hDma2dHandler.LayerCfg[1].InputAlpha = 0xFF;
	hDma2dHandler.LayerCfg[1].InputColorMode = ColorMode;
	hDma2dHandler.LayerCfg[1].InputOffset = 0;
	hDma2dHandler.base = (uint32_t)TADR_DMA2D_BASE;

	/* DMA2D Initialization */
	if(dma2d_init(&hDma2dHandler) == E_OK){
		if(dma2d_configlayer(&hDma2dHandler, 1) == E_OK){
			if(dma2d_start(&hDma2dHandler, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == E_OK){
				/* Polling For DMA transfer */
				dma2d_waittransfar(&hDma2dHandler, 10);
			}
		}
	}
}

