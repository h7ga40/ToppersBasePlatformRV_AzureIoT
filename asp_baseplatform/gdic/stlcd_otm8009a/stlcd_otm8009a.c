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

#include "stlcd_otm8009a.h"
#include "kernel_cfg.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))



/** @defgroup STM32F769I-DISCOVERY_LCD_Private_Macros LCD Private Macros
  * @{
  */
#define ABS(X)                 ((X) > 0 ? (X) : -(X))

#define POLY_X(Z)              ((int32_t)((Points + (Z))->X))
#define POLY_Y(Z)              ((int32_t)((Points + (Z))->Y))

/*
 * @brief Constant tables of register settings used to transmit DSI
 * command packets as power up initialization sequence of the KoD LCD (OTM8009A LCD Driver)
 */
static const uint8_t lcdRegData1[]  = {0x80,0x09,0x01,0xFF};
static const uint8_t lcdRegData2[]  = {0x80,0x09,0xFF};
static const uint8_t lcdRegData3[]  = {0x00,0x09,0x0F,0x0E,0x07,0x10,0x0B,0x0A,0x04,0x07,0x0B,0x08,0x0F,0x10,0x0A,0x01,0xE1};
static const uint8_t lcdRegData4[]  = {0x00,0x09,0x0F,0x0E,0x07,0x10,0x0B,0x0A,0x04,0x07,0x0B,0x08,0x0F,0x10,0x0A,0x01,0xE2};
static const uint8_t lcdRegData5[]  = {0x79,0x79,0xD8};
static const uint8_t lcdRegData6[]  = {0x00,0x01,0xB3};
static const uint8_t lcdRegData7[]  = {0x85,0x01,0x00,0x84,0x01,0x00,0xCE};
static const uint8_t lcdRegData8[]  = {0x18,0x04,0x03,0x39,0x00,0x00,0x00,0x18,0x03,0x03,0x3A,0x00,0x00,0x00,0xCE};
static const uint8_t lcdRegData9[]  = {0x18,0x02,0x03,0x3B,0x00,0x00,0x00,0x18,0x01,0x03,0x3C,0x00,0x00,0x00,0xCE};
static const uint8_t lcdRegData10[] = {0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x02,0x00,0x00,0xCF};
static const uint8_t lcdRegData11[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData12[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData13[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData14[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData15[] = {0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData16[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData17[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCB};
static const uint8_t lcdRegData18[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xCB};
static const uint8_t lcdRegData19[] = {0x00,0x26,0x09,0x0B,0x01,0x25,0x00,0x00,0x00,0x00,0xCC};
static const uint8_t lcdRegData20[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x0A,0x0C,0x02,0xCC};
static const uint8_t lcdRegData21[] = {0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC};
static const uint8_t lcdRegData22[] = {0x00,0x25,0x0C,0x0A,0x02,0x26,0x00,0x00,0x00,0x00,0xCC};
static const uint8_t lcdRegData23[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x0B,0x09,0x01,0xCC};
static const uint8_t lcdRegData24[] = {0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC};
static const uint8_t lcdRegData25[] = {0xFF,0xFF,0xFF,0xFF};
/*
  * CASET value (Column Address Set) : X direction LCD GRAM boundaries
  * depending on LCD orientation mode and PASET value (Page Address Set) : Y direction
  * LCD GRAM boundaries depending on LCD orientation mode
  * XS[15:0] = 0x000 = 0, XE[15:0] = 0x31F = 799 for landscape mode : apply to CASET
  * YS[15:0] = 0x000 = 0, YE[15:0] = 0x31F = 799 for portrait mode : : apply to PASET
  */
static const uint8_t lcdRegData27[] = {0x00, 0x00, 0x03, 0x1F, OTM8009A_CMD_CASET};
/*
  * XS[15:0] = 0x000 = 0, XE[15:0] = 0x1DF = 479 for portrait mode : apply to CASET
  * YS[15:0] = 0x000 = 0, YE[15:0] = 0x1DF = 479 for landscape mode : apply to PASET
 */
static const uint8_t lcdRegData28[] = {0x00, 0x00, 0x01, 0xDF, OTM8009A_CMD_PASET};


static const uint8_t ShortRegData1[]  = {OTM8009A_CMD_NOP, 0x00};
static const uint8_t ShortRegData2[]  = {OTM8009A_CMD_NOP, 0x80};
static const uint8_t ShortRegData3[]  = {0xC4, 0x30};
static const uint8_t ShortRegData4[]  = {OTM8009A_CMD_NOP, 0x8A};
static const uint8_t ShortRegData5[]  = {0xC4, 0x40};
static const uint8_t ShortRegData6[]  = {OTM8009A_CMD_NOP, 0xB1};
static const uint8_t ShortRegData7[]  = {0xC5, 0xA9};
static const uint8_t ShortRegData8[]  = {OTM8009A_CMD_NOP, 0x91};
static const uint8_t ShortRegData9[]  = {0xC5, 0x34};
static const uint8_t ShortRegData10[] = {OTM8009A_CMD_NOP, 0xB4};
static const uint8_t ShortRegData11[] = {0xC0, 0x50};
static const uint8_t ShortRegData12[] = {0xD9, 0x4E};
static const uint8_t ShortRegData13[] = {OTM8009A_CMD_NOP, 0x81};
static const uint8_t ShortRegData14[] = {0xC1, 0x66};
static const uint8_t ShortRegData15[] = {OTM8009A_CMD_NOP, 0xA1};
static const uint8_t ShortRegData16[] = {0xC1, 0x08};
static const uint8_t ShortRegData17[] = {OTM8009A_CMD_NOP, 0x92};
static const uint8_t ShortRegData18[] = {0xC5, 0x01};
static const uint8_t ShortRegData19[] = {OTM8009A_CMD_NOP, 0x95};
static const uint8_t ShortRegData20[] = {OTM8009A_CMD_NOP, 0x94};
static const uint8_t ShortRegData21[] = {0xC5, 0x33};
static const uint8_t ShortRegData22[] = {OTM8009A_CMD_NOP, 0xA3};
static const uint8_t ShortRegData23[] = {0xC0, 0x1B};
static const uint8_t ShortRegData24[] = {OTM8009A_CMD_NOP, 0x82};
static const uint8_t ShortRegData25[] = {0xC5, 0x83};
static const uint8_t ShortRegData26[] = {0xC4, 0x83};
static const uint8_t ShortRegData27[] = {0xC1, 0x0E};
static const uint8_t ShortRegData28[] = {OTM8009A_CMD_NOP, 0xA6};
static const uint8_t ShortRegData29[] = {OTM8009A_CMD_NOP, 0xA0};
static const uint8_t ShortRegData30[] = {OTM8009A_CMD_NOP, 0xB0};
static const uint8_t ShortRegData31[] = {OTM8009A_CMD_NOP, 0xC0};
static const uint8_t ShortRegData32[] = {OTM8009A_CMD_NOP, 0xD0};
static const uint8_t ShortRegData33[] = {OTM8009A_CMD_NOP, 0x90};
static const uint8_t ShortRegData34[] = {OTM8009A_CMD_NOP, 0xE0};
static const uint8_t ShortRegData35[] = {OTM8009A_CMD_NOP, 0xF0};
static const uint8_t ShortRegData36[] = {OTM8009A_CMD_SLPOUT, 0x00};
static const uint8_t ShortRegData37[] = {OTM8009A_CMD_COLMOD, OTM8009A_COLMOD_RGB565};
static const uint8_t ShortRegData38[] = {OTM8009A_CMD_COLMOD, OTM8009A_COLMOD_RGB888};
static const uint8_t ShortRegData39[] = {OTM8009A_CMD_MADCTR, OTM8009A_MADCTR_MODE_LANDSCAPE};
static const uint8_t ShortRegData40[] = {OTM8009A_CMD_WRDISBV, 0x7F};
static const uint8_t ShortRegData41[] = {OTM8009A_CMD_WRCTRLD, 0x2C};
static const uint8_t ShortRegData42[] = {OTM8009A_CMD_WRCABC, 0x02};
static const uint8_t ShortRegData43[] = {OTM8009A_CMD_WRCABCMB, 0xFF};
static const uint8_t ShortRegData44[] = {OTM8009A_CMD_DISPON, 0x00};
static const uint8_t ShortRegData45[] = {OTM8009A_CMD_RAMWR, 0x00};
static const uint8_t ShortRegData46[] = {0xCF, 0x00};
static const uint8_t ShortRegData47[] = {0xC5, 0x66};
static const uint8_t ShortRegData48[] = {OTM8009A_CMD_NOP, 0xB6};
static const uint8_t ShortRegData49[] = {0xF5, 0x06};


/** @defgroup STM32F769I-DISCOVERY_LCD_Exported_Variables STM32F769I DISCOVERY LCD Exported Variables
  * @{
  */
static DMA2D_Handle_t hdma2d_discovery;
static LTDC_Handle_t  hltdc_discovery;
static DSI_Handle_t   hdsi_discovery;
/**
  * @}
  */

/**
  * @}
  */

/** @defgroup STM32F769I-DISCOVERY_LCD_Private_FunctionPrototypes LCD Private FunctionPrototypes
  * @{
  */
static void FillTriangle(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);
static void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
static void LL_ConvertLineToARGB8888(void * pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode);
/**
  * @}
  */
/**
  * @brief  DCS or Generic short/long write command
  * @param  NbParams: Number of parameters. It indicates the write command mode:
  *                 If inferior to 2, a long write command is performed else short.
  * @param  pParams: Pointer to parameter values table.
  * @retval HAL status
  */
static void
DSI_IO_WriteCmd(DSI_Handle_t *hdsi, uint32_t NbrParams, uint8_t *pParams)
{
	if(NbrParams <= 1){
		dsi_swrite(hdsi, LCD_OTM8009A_ID, DSI_DCS_SHORT_PKT_WRITE_P1, pParams[0], pParams[1]);
	}
	else{
		dsi_lwrite(hdsi,  LCD_OTM8009A_ID, DSI_DCS_LONG_PKT_WRITE, NbrParams, pParams[NbrParams], pParams);
	}
}

/**
  * @brief  Initializes the LCD KoD display part by communication in DSI mode in Video Mode
  *         with IC Display Driver OTM8009A (see IC Driver specification for more information).
  * @param  hdsi_eval : pointer on DSI configuration structure
  * @param  hdsivideo_handle : pointer on DSI video mode configuration structure
  * @retval Status
  */
static uint8_t
OTM8009A_Init(DSI_Handle_t *hdsi, uint32_t ColorCoding, uint32_t orientation)
{
	/* Enable CMD2 to access vendor specific commands                               */
	/* Enter in command 2 mode and set EXTC to enable address shift function (0x00) */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);
	DSI_IO_WriteCmd(hdsi,  3, (uint8_t *)lcdRegData1);

	/* Enter ORISE Command 2 */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData2); /* Shift address to 0x80 */
	DSI_IO_WriteCmd(hdsi,  2, (uint8_t *)lcdRegData2);

	/////////////////////////////////////////////////////////////////////
	/* SD_PCH_CTRL - 0xC480h - 129th parameter - Default 0x00          */
	/* Set SD_PT                                                       */
	/* -> Source output level during porch and non-display area to GND */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData2);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData3);
	dly_tsk(10);
	/* Not documented */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData4);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData5);
	dly_tsk(10);
	/////////////////////////////////////////////////////////////////////

	/* PWR_CTRL4 - 0xC4B0h - 178th parameter - Default 0xA8 */
	/* Set gvdd_en_test                                     */
	/* -> enable GVDD test mode !!!                         */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData6);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData7);

	/* PWR_CTRL2 - 0xC590h - 146th parameter - Default 0x79      */
	/* Set pump 4 vgh voltage                                    */
	/* -> from 15.0v down to 13.0v                               */
	/* Set pump 5 vgh voltage                                    */
	/* -> from -12.0v downto -9.0v                               */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData8);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData9);

	/* P_DRV_M - 0xC0B4h - 181th parameter - Default 0x00 */
	/* -> Column inversion                                */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData10);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData11);

	/* VCOMDC - 0xD900h - 1st parameter - Default 0x39h */
	/* VCOM Voltage settings                            */
	/* -> from -1.0000v downto -1.2625v                 */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData12);

	/* Oscillator adjustment for Idle/Normal mode (LPDT only) set to 65Hz (default is 60Hz) */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData13);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData14);

	/* Video mode internal */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData15);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData16);

	/* PWR_CTRL2 - 0xC590h - 147h parameter - Default 0x00 */
	/* Set pump 4&5 x6                                     */
	/* -> ONLY VALID when PUMP4_EN_ASDM_HV = "0"           */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData17);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData18);

	/* PWR_CTRL2 - 0xC590h - 150th parameter - Default 0x33h */
	/* Change pump4 clock ratio                              */
	/* -> from 1 line to 1/2 line                            */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData19);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData9);

	/* GVDD/NGVDD settings */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);
	DSI_IO_WriteCmd(hdsi,  2, (uint8_t *)lcdRegData5);

	/* PWR_CTRL2 - 0xC590h - 149th parameter - Default 0x33h */
	/* Rewrite the default value !                           */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData20);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData21);

	/* Panel display timing Setting 3 */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData22);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData23);

	/* Power control 1 */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData24);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData25);

	/* Source driver precharge */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData13);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData26);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData15);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData27);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData28);
	DSI_IO_WriteCmd(hdsi,  2, (uint8_t *)lcdRegData6);

	/* GOAVST */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData2);
	DSI_IO_WriteCmd(hdsi,  6, (uint8_t *)lcdRegData7);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData29);
	DSI_IO_WriteCmd(hdsi,  14, (uint8_t *)lcdRegData8);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData30);
	DSI_IO_WriteCmd(hdsi,  14, (uint8_t *)lcdRegData9);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData31);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData10);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData32);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData46);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData2);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData11);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData33);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData12);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData29);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData13);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData30);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData14);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData31);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData15);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData32);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData16);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData34);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData17);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData35);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData18);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData2);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData19);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData33);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData20);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData29);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData21);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData30);
	DSI_IO_WriteCmd(hdsi,  10, (uint8_t *)lcdRegData22);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData31);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData23);

	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData32);
	DSI_IO_WriteCmd(hdsi,  15, (uint8_t *)lcdRegData24);

	/////////////////////////////////////////////////////////////////////////////
	/* PWR_CTRL1 - 0xc580h - 130th parameter - default 0x00 */
	/* Pump 1 min and max DM                                */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData13);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData47);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData48);
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData49);
	/////////////////////////////////////////////////////////////////////////////

	/* Exit CMD2 mode */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);
	DSI_IO_WriteCmd(hdsi,  3, (uint8_t *)lcdRegData25);

	/*************************************************************************** */
	/* Standard DCS Initialization TO KEEP CAN BE DONE IN HSDT                   */
	/*************************************************************************** */

	/* NOP - goes back to DCS std command ? */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);

	/* Gamma correction 2.2+ table (HSDT possible) */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);
	DSI_IO_WriteCmd(hdsi,  16, (uint8_t *)lcdRegData3);

	/* Gamma correction 2.2- table (HSDT possible) */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);
	DSI_IO_WriteCmd(hdsi,  16, (uint8_t *)lcdRegData4);

	/* Send Sleep Out command to display : no parameter */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData36);

	/* Wait for sleep out exit */
	dly_tsk(120);

	switch(ColorCoding){
	case OTM8009A_FORMAT_RBG565:
		/* Set Pixel color format to RGB565 */
		DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData37);
		break;
	case OTM8009A_FORMAT_RGB888:
		/* Set Pixel color format to RGB888 */
		DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData38);
		break;
	default:
		break;
	}

	/* Send command to configure display in landscape orientation mode. By default
      the orientation mode is portrait  */
	if(orientation == OTM8009A_ORIENTATION_LANDSCAPE){
		DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData39);
		DSI_IO_WriteCmd(hdsi,  4, (uint8_t *)lcdRegData27);
		DSI_IO_WriteCmd(hdsi,  4, (uint8_t *)lcdRegData28);
	}

	/** CABC : Content Adaptive Backlight Control section start >> */
	/* Note : defaut is 0 (lowest Brightness), 0xFF is highest Brightness, try 0x7F : intermediate value */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData40);

	/* defaut is 0, try 0x2C - Brightness Control Block, Display Dimming & BackLight on */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData41);

	/* defaut is 0, try 0x02 - image Content based Adaptive Brightness [Still Picture] */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData42);

	/* defaut is 0 (lowest Brightness), 0xFF is highest Brightness */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData43);

	/** CABC : Content Adaptive Backlight Control section end << */

	/* Send Command Display On */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData44);

	/* NOP command */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData1);

	/* Send Command GRAM memory write (no parameters) : this initiates frame write via other DSI commands sent by */
	/* DSI host from LTDC incoming pixels in video mode */
	DSI_IO_WriteCmd(hdsi, 0, (uint8_t *)ShortRegData45);
	return 0;
}


/*
 *  LTDC/DSI割込みハンドラ
 */
void ltdc_handler(void)
{
	ltdc_irqhandler(&hltdc_discovery);
}

void dsi_handler(void)
{
	dsi_irqhandler(&hdsi_discovery);
}

/*
 *  DSI/OTM8009Aスクリーンの初期化関数
 */
ER
lcd_init(LCD_Handler_t *hlcd, LCD_OrientationTypeDef orientation)
{
	LTDC_Handle_t *hldtc = &hltdc_discovery;
	DSI_Handle_t  *hdsi  = &hdsi_discovery;
	uint32_t LcdClock  = 27429; /*!< LcdClk = 27429 kHz */

	uint32_t laneByteClk_kHz = 0;
	uint32_t VSA;	/*!< Vertical start active time in units of lines */
	uint32_t VBP;	/*!< Vertical Back Porch time in units of lines */
	uint32_t VFP;	/*!< Vertical Front Porch time in units of lines */
	uint32_t VACT;	/*!< Vertical Active time in units of lines = imageSize Y in pixels to display */
	uint32_t HSA;	/*!< Horizontal start active time in units of lcdClk */
	uint32_t HBP;	/*!< Horizontal Back Porch time in units of lcdClk */
	uint32_t HFP;	/*!< Horizontal Front Porch time in units of lcdClk */
	uint32_t HACT;	/*!< Horizontal Active time in units of lcdClk = imageSize X in pixels to display */

	/* Toggle Hardware Reset of the DSI LCD using
	 * its XRES signal (active low) */
	lcd_reset(hlcd);
	hlcd->layer = LTDC_ACTIVE_LAYER_BACKGROUND;

  /* Call first MSP Initialize only in case of first initialization
  * This will set IP blocks LTDC, DSI and DMA2D
  * - out of reset
  * - clocked
  * - NVIC IRQ related to IP blocks enabled
  */
	/** @brief Enable the LTDC clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_LTDCEN);


	/** @brief Toggle Sw reset of LTDC IP */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2RSTR), RCC_APB2RSTR_LTDCRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2RSTR), RCC_APB2RSTR_LTDCRST);

	/** @brief Enable the DMA2D clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2DEN);

	/** @brief Toggle Sw reset of DMA2D IP */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1RSTR), RCC_AHB1RSTR_DMA2DRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1RSTR), RCC_AHB1RSTR_DMA2DRST);

	/** @brief Enable DSI Host and wrapper clocks */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_DSIEN);

	/** @brief Soft Reset the DSI Host and wrapper */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2RSTR), RCC_APB2RSTR_DSIRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2RSTR), RCC_APB2RSTR_DSIRST);

/*************************DSI Initialization***********************************/  

	/* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
	hlcd->hdsi = hdsi;
	hdsi->base = TADR_DSI_BASE;

	dsi_deinit(hlcd->hdsi);

	hdsi->Init.pllndiv = 100;
	hdsi->Init.pllidf  = DSI_PLL_IN_DIV5;
	hdsi->Init.pllodf  = DSI_PLL_OUT_DIV1;
	laneByteClk_kHz = 62500; /* 500 MHz / 8 = 62.5 MHz = 62500 kHz */

	/* Set number of Lanes */
	hdsi->Init.NumberOfLanes = DSI_TWO_DATA_LANES;

	/* TXEscapeCkdiv = f(LaneByteClk)/15.62 = 4 */
	hdsi->Init.TXEscapeCkdiv = laneByteClk_kHz/15620;

	dsi_init(hlcd->hdsi);

	/* Timing parameters for all Video modes
	 * Set Timing parameters of LTDC depending on its chosen orientation
	 */
	if(orientation == LCD_ORIENTATION_PORTRAIT){
		hlcd->_width  = OTM8009A_480X800_WIDTH;  /* 480 */
		hlcd->_height = OTM8009A_480X800_HEIGHT; /* 800 */                                
	}
	else{
		/* lcd_orientation == LCD_ORIENTATION_LANDSCAPE */
		hlcd->_width  = OTM8009A_800X480_WIDTH;  /* 800 */
		hlcd->_height = OTM8009A_800X480_HEIGHT; /* 480 */                                
	}

	HACT = hlcd->_width;
	VACT = hlcd->_height;

	/* The following values are same for portrait and landscape orientations */
	VSA  = OTM8009A_480X800_VSYNC;        /* 12  */
	VBP  = OTM8009A_480X800_VBP;          /* 12  */
	VFP  = OTM8009A_480X800_VFP;          /* 12  */
	HSA  = OTM8009A_480X800_HSYNC;        /* 63  */
	HBP  = OTM8009A_480X800_HBP;          /* 120 */
	HFP  = OTM8009A_480X800_HFP;          /* 120 */   

	hlcd->dsivc.VirtualChannelID = LCD_OTM8009A_ID;
	hlcd->dsivc.ColorCoding = LCD_DSI_PIXEL_DATA_FMT_RBG888;
	hlcd->dsivc.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
	hlcd->dsivc.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
	hlcd->dsivc.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;  
	hlcd->dsivc.Mode = DSI_VID_MODE_BURST; /* Mode Video burst ie : one LgP per line */
	hlcd->dsivc.NullPacketSize = 0xFFF;
	hlcd->dsivc.NumberOfChunks = 0;
	hlcd->dsivc.PacketSize                = HACT; /* Value depending on display orientation choice portrait/landscape */ 
	hlcd->dsivc.HorizontalSyncActive      = (HSA * laneByteClk_kHz)/LcdClock;
	hlcd->dsivc.HorizontalBackPorch       = (HBP * laneByteClk_kHz)/LcdClock;
	hlcd->dsivc.HorizontalLine            = ((HACT + HSA + HBP + HFP) * laneByteClk_kHz)/LcdClock; /* Value depending on display orientation choice portrait/landscape */
	hlcd->dsivc.VerticalSyncActive        = VSA;
	hlcd->dsivc.VerticalBackPorch         = VBP;
	hlcd->dsivc.VerticalFrontPorch        = VFP;
	hlcd->dsivc.VerticalActive            = VACT; /* Value depending on display orientation choice portrait/landscape */

	/* Enable or disable sending LP command while streaming is active in video mode */
	hlcd->dsivc.LPCommandEnable = DSI_LP_COMMAND_ENABLE; /* Enable sending commands in mode LP (Low Power) */

	/* Largest packet size possible to transmit in LP mode in VSA, VBP, VFP regions */
	/* Only useful when sending LP packets is allowed while streaming is active in video mode */
	hlcd->dsivc.LPLargestPacketSize = 16;

	/* Largest packet size possible to transmit in LP mode in HFP region during VACT period */
	/* Only useful when sending LP packets is allowed while streaming is active in video mode */
	hlcd->dsivc.LPVACTLargestPacketSize = 0;

	/* Specify for each region of the video frame, if the transmission of command in LP mode is allowed in this region */
	/* while streaming is active in video mode                                                                         */
	hlcd->dsivc.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;   /* Allow sending LP commands during HFP period */
	hlcd->dsivc.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;   /* Allow sending LP commands during HBP period */
	hlcd->dsivc.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;  /* Allow sending LP commands during VACT period */
	hlcd->dsivc.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;   /* Allow sending LP commands during VFP period */
	hlcd->dsivc.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;   /* Allow sending LP commands during VBP period */
	hlcd->dsivc.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE; /* Allow sending LP commands during VSync = VSA period */

	/* Configure DSI Video mode timings with settings set above */
	dsi_configvideo(hlcd->hdsi, &hlcd->dsivc);

/*************************End DSI Initialization*******************************/ 


/************************LTDC Initialization***********************************/  

	/* Timing Configuration */
	hldtc->Init.HorizontalSync = (HSA - 1);
	hldtc->Init.AccumulatedHBP = (HSA + HBP - 1);
	hldtc->Init.AccumulatedActiveW = (hlcd->_width + HSA + HBP - 1);
	hldtc->Init.TotalWidth = (hlcd->_width + HSA + HBP + HFP - 1);

	/* Initialize the LCD pixel width and pixel height */
	hldtc->LayerCfg->ImageWidth  = hlcd->_width;
	hldtc->LayerCfg->ImageHeight = hlcd->_height;

	/** LCD clock configuration
	 * Note: The following values should not be changed as the PLLSAI is also used 
	 *      to clock the USB FS
	 * PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz 
	 * PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 384 Mhz 
	 * PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 384 MHz / 7 = 54.85 MHz 
	 * LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_2 = 54.85 MHz / 2 = 27.429 MHz 
	 */
	/* Background value */
	hldtc->Init.Backcolor.Blue = 0;
	hldtc->Init.Backcolor.Green = 0;
	hldtc->Init.Backcolor.Red = 0;
	hldtc->Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hldtc->Init.pllsain = 384;
	hldtc->Init.pllsair = 7;
	hldtc->Init.saidivr = RCC_PLLSAIDIVR_2;
	hldtc->base = TADR_LTDC_BASE;

	/* Get LTDC Configuration from DSI Configuration */
	dci_configltdc(hldtc, &hlcd->dsivc);

	/* Initialize the LTDC */  
	ltdc_init(hldtc);
	hlcd->hltdc = hldtc;

	/* Enable the DSI host and wrapper after the LTDC initialization
	   To avoid any synchronization issue, the DSI shall be started after enabling the LTDC */
	dsi_start(hlcd->hdsi);

	/* Initialize the font */
//	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

/************************End LTDC Initialization*******************************/
  
  
/***********************OTM8009A Initialization********************************/ 

	/* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver)
	 *  depending on configuration set in 'hdsivideo_handle'.
	 */
	OTM8009A_Init(hlcd->hdsi, OTM8009A_FORMAT_RGB888, orientation);

/***********************End OTM8009A Initialization****************************/ 
	return E_OK;
}

/*
 *  DSI/OTM8009Aスクリーンのリセット関数
 */
ER
lcd_reset(LCD_Handler_t *hlcd)
{
	GPIO_Init_t  gpio_init_structure;
	uint32_t base = TADR_GPIOJ_BASE;

	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOJEN);

    /* Configure the GPIO on PJ15 */
    gpio_init_structure.mode  = GPIO_MODE_OUTPUT;
    gpio_init_structure.pull  = GPIO_PULLUP;
    gpio_init_structure.otype = GPIO_OTYPE_PP;
    gpio_init_structure.speed = GPIO_SPEED_HIGH;
	gpio_setup(base, &gpio_init_structure, PINPOSITION15);

    sil_wrw_mem((uint32_t *)(base+TOFF_GPIO_BSRR), GPIO_PIN_15<<16);
	sil_dly_nse(100);

    dly_tsk(20); /* wait 20 ms */

    /* Desactivate XRES */
    sil_wrw_mem((uint32_t *)(base+TOFF_GPIO_BSRR), GPIO_PIN_15);

    /* Wait for 10ms after releasing XRES before sending commands */
    dly_tsk(10);
	return E_OK;
}


/*
 *  DSI/OTM8009Aスクリーンレイヤ初期化関数
 */
ER
lcd_layerdefaultinit(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address)
{
    LTDC_LayerCfg_t  Layercfg;

	/* Layer Init */
	Layercfg.WindowX0 = 0;
	Layercfg.WindowX1 = hlcd->_width;
	Layercfg.WindowY0 = 0;
	Layercfg.WindowY1 = hlcd->_height;
	Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	Layercfg.FBStartAdress = FB_Address;
	Layercfg.Alpha = 255;
	Layercfg.Alpha0 = 0;
	Layercfg.Backcolor.Blue = 0;
	Layercfg.Backcolor.Green = 0;
	Layercfg.Backcolor.Red = 0;
	Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	Layercfg.ImageWidth = hlcd->_width;
	Layercfg.ImageHeight = hlcd->_height;

	ltdc_configlayer(hlcd->hltdc, &Layercfg, LayerIndex);

	hlcd->layer = LayerIndex;
	pDrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;
	pDrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK;
	pDrawProp[LayerIndex].hlcd      = hlcd;
	return E_OK;
}

/*
 *  DSI/OTM8009Aスクリーンレイヤ表示設定関数
 */
void
lcd_setlayervisible(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint8_t State)
{
	uint32_t layer = (TADR_LTDC_LAYER1 + (LTDC_WINDOW_SIZE * LayerIndex));
	if(State == 1){
		sil_orw_mem((uint32_t *)(layer+TOFF_LTDCW_CR), LTDC_CR_LEN);
	}
	else{
		sil_andw_mem((uint32_t *)(layer+TOFF_LTDCW_CR), LTDC_CR_LEN);
	}
	sil_orw_mem((uint32_t *)(TADR_LTDC_BASE+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
}

/*
 *  DSI/OTM8009Aスクリーンレイヤ透過設定関数
 */
void
lcd_settransparency(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint8_t Transparency)
{
	ltdc_setalpha(hlcd->hltdc, Transparency, LayerIndex);
}

/*
 *  DSI/OTM8009AスクリーンレイヤRAMアドレス設定関数
 */
void
lcd_setlayeraddress(LCD_Handler_t *hlcd, uint32_t LayerIndex, uint32_t Address)
{
	ltdc_setaddress(hlcd->hltdc, Address, LayerIndex);
}

/*
 *  DSI/OTM8009A表示ウィンドウ設定
 */
void
lcd_setAddrWindow(LCD_Handler_t *hlcd, uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
	/* Reconfigure the layer size */
	ltdc_setwindowsize(hlcd->hltdc, Width, Height, LayerIndex);

	/* Reconfigure the layer position */
	ltdc_setwindowposition(hlcd->hltdc, Xpos, Ypos, LayerIndex);
}

/**
  * @brief  Configures and sets the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @param  RGBValue: Color reference
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
  */
void
lcd_resetcolorkeying(LCD_Handler_t *hlcd, uint32_t LayerIndex)
{
	/* Disable the color Keying for LCD Layer */
	ltdc_disablecolorkeying(hlcd->hltdc, LayerIndex);
}

/*
 *  DSI/OTM8009Aスクリーンクリア
 */
void
lcd_clear(LCD_Handler_t *hlcd, uint32_t Color)
{
	/* Clear the LCD */
	LL_FillBuffer(hlcd->layer, (uint32_t *)(hlcd->hltdc->LayerCfg[hlcd->layer].FBStartAdress), hlcd->_width, hlcd->_height, 0, Color);
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
	LTDC_Handle_t  *hltdc = hlcd->hltdc;
	uint32_t ret = 0;

	if(hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888){
		/* Read data value from SDRAM memory */
		ret = *(volatile uint32_t*)(hltdc->LayerCfg[hlcd->layer].FBStartAdress + (2*(Ypos*hlcd->_width + Xpos)));
	}
	else if(hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB888){
		/* Read data value from SDRAM memory */
		ret = (*(volatile uint32_t*) (hltdc->LayerCfg[hlcd->layer].FBStartAdress + (2*(Ypos*hlcd->_width + Xpos))) & 0x00FFFFFF);
	}
	else if((hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) || \
          (hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) || \
          (hltdc->LayerCfg[hlcd->layer].PixelFormat == LTDC_PIXEL_FORMAT_AL88)){
		/* Read data value from SDRAM memory */
		ret = *(volatile uint16_t*) (hltdc->LayerCfg[hlcd->layer].FBStartAdress + (2*(Ypos*hlcd->_width + Xpos)));
	}
	else{
		/* Read data value from SDRAM memory */
		ret = *(volatile uint8_t*) (hltdc->LayerCfg[hlcd->layer].FBStartAdress + (2*(Ypos*hlcd->_width + Xpos)));
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
	/* Write data value to all SDRAM memory */
	*(volatile uint32_t*) (hlcd->hltdc->LayerCfg[hlcd->layer].FBStartAdress + (4*(Ypos*hlcd->_width + Xpos))) = RGB_Code;
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
	uint32_t  Xaddress = 0;

	/* Get the line address */
	Xaddress = (hlcd->hltdc->LayerCfg[hlcd->layer].FBStartAdress) + 4*(hlcd->_width*Ypos + Xpos);

	/* Write line */
	LL_FillBuffer(hlcd->layer, (uint32_t *)Xaddress, Length, 1, 0, color);
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
	uint32_t  Xaddress = 0;

	/* Get the line address */
	Xaddress = (hlcd->hltdc->LayerCfg[hlcd->layer].FBStartAdress) + 4*(hlcd->_width*Ypos + Xpos);

	/* Write line */
	LL_FillBuffer(hlcd->layer, (uint32_t *)Xaddress, 1, Length, (hlcd->_width - 1), color);
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
	uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
	uint32_t Address;
	uint32_t InputColorMode = 0;

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
	Address = hlcd->hltdc->LayerCfg[hlcd->layer].FBStartAdress + (((hlcd->_width*Ypos) + Xpos)*(4));

	/* Get the layer pixel format */
	if((bit_pixel/8) == 4){
		InputColorMode = DMA2D_ARGB8888;
	}
	else if((bit_pixel/8) == 2){
		InputColorMode = DMA2D_RGB565;
	}
	else{
		InputColorMode = DMA2D_RGB888;
	}

	/* Bypass the bitmap header */
	pbmp += (index + (width * (height - 1) * (bit_pixel/8)));

	/* Convert picture to ARGB8888 pixel format */
	for(index=0; index < height; index++){
		/* Pixel format conversion */
		LL_ConvertLineToARGB8888((uint32_t *)pbmp, (uint32_t *)Address, width, InputColorMode);

		/* Increment the source and destination buffers */
		Address+=  (hlcd->_width*4);
		pbmp -= width*(bit_pixel/8);
	}
}

/*
 *  表示オン
 *  param1  hlcd: Pointer to LCD Handler
 */
void
lcd_displayOn(LCD_Handler_t *hlcd)
{
    /* Send Display on DCS command to display */
    dsi_swrite(hlcd->hdsi, hlcd->dsivc.VirtualChannelID,
                       DSI_DCS_SHORT_PKT_WRITE_P1,
                       OTM8009A_CMD_DISPON,
                       0x00);
}

/*
 *  表示オフ
 *  param1  hlcd: Pointer to LCD Handler
 */
void
lcd_displayOff(LCD_Handler_t *hlcd)
{
    /* Send Display off DCS Command to display */
    dsi_swrite(hlcd->hdsi, hlcd->dsivc.VirtualChannelID,
                       DSI_DCS_SHORT_PKT_WRITE_P1,
                       OTM8009A_CMD_DISPOFF,
                       0x00);
}

/**
  * @brief  Set the brightness value 
  * @param  BrightnessValue: [00: Min (black), 100 Max]
  */
void
lcd_setbrightness(LCD_Handler_t *hlcd, uint8_t BrightnessValue)
{
    /* Send Display on DCS command to display */
    dsi_swrite(hlcd->hdsi, LCD_OTM8009A_ID, 
                       DSI_DCS_SHORT_PKT_WRITE_P1, 
                       OTM8009A_CMD_WRDISBV, (uint16_t)(BrightnessValue * 255)/100);
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
	uint32_t  Xaddress = 0;

	/* Get the rectangle start address */
	Xaddress = (hlcd->hltdc->LayerCfg[hlcd->layer].FBStartAdress) + 4*(hlcd->_width*Ypos + Xpos);

	/* Fill the rectangle */
	LL_FillBuffer(hlcd->layer, (uint32_t *)Xaddress, Width, Height, (hlcd->_width - Width), color);
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
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
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
		xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
		yinc2 = 0;                  /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         /* There are more x-values than y-values */
	}
	else{							/* There is at least one y-value for every x-value */
		xinc2 = 0;                  /* Don't change the x for every iteration */
		yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         /* There are more y-values than x-values */
	}

	for(curpixel = 0; curpixel <= numpixels; curpixel++){
		lcd_drawPixel(hlcd, x, y, pDrawProp->TextColor);	/* Draw the current pixel */
		num += numadd;				/* Increase the numerator by the top of the fraction */
		if(num >= den){				/* Check if numerator >= denominator */
			num -= den;				/* Calculate the new numerator value */
			x += xinc1;				/* Change the x as appropriate */
			y += yinc1;				/* Change the y as appropriate */
		}
		x += xinc2;					/* Change the x as appropriate */
		y += yinc2;					/* Change the y as appropriate */
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
	uint32_t      color = pDrawProp->TextColor;

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
	uint32_t      color = pDrawProp->TextColor;
	int32_t   D;    /* Decision Variable */
	uint32_t  CurX; /* Current X Value */
	uint32_t  CurY; /* Current Y Value */

	D = 3 - (Radius << 1);
	CurX = 0;
	CurY = Radius;

	while (CurX <= CurY){
		lcd_drawPixel(hlcd, (Xpos + CurX), (Ypos - CurY), color);
		lcd_drawPixel(hlcd, (Xpos - CurX), (Ypos - CurY), color);
		lcd_drawPixel(hlcd, (Xpos + CurY), (Ypos - CurX), color);
		lcd_drawPixel(hlcd, (Xpos - CurY), (Ypos - CurX), color);
		lcd_drawPixel(hlcd, (Xpos + CurX), (Ypos + CurY), color);
		lcd_drawPixel(hlcd, (Xpos - CurX), (Ypos + CurY), color);
		lcd_drawPixel(hlcd, (Xpos + CurY), (Ypos + CurX), color);
		lcd_drawPixel(hlcd, (Xpos - CurY), (Ypos + CurX), color);
		if(D < 0){
			D += (CurX << 2) + 6;
		}
		else{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
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
	uint32_t      color = pDrawProp->TextColor;
	int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
	float K = 0, rad1 = 0, rad2 = 0;

	rad1 = XRadius;
	rad2 = YRadius;

	K = (float)(rad2/rad1);

	do{
		lcd_drawPixel(hlcd, (Xpos-(uint16_t)(x/K)), (Ypos+y), color);
		lcd_drawPixel(hlcd, (Xpos+(uint16_t)(x/K)), (Ypos+y), color);
		lcd_drawPixel(hlcd, (Xpos+(uint16_t)(x/K)), (Ypos-y), color);
		lcd_drawPixel(hlcd, (Xpos-(uint16_t)(x/K)), (Ypos-y), color);

		e2 = err;
		if(e2 <= x){
			err += ++x*2+1;
			if (-y == x && e2 <= y)
				e2 = 0;
		}
		if(e2 > y)
			err += ++y*2+1;
	}while (y <= 0);
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
	int16_t X = 0, Y = 0;

	if(PointCount < 2){
		return;
	}

	lcd_drawLine(pDrawProp, Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);

	while(--PointCount){
		X = Points->X;
		Y = Points->Y;
		Points++;
		lcd_drawLine(pDrawProp, X, Y, Points->X, Points->Y);
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
	uint32_t      color = pDrawProp->TextColor;
	int32_t  D;     /* Decision Variable */
	uint32_t  CurX; /* Current X Value */
	uint32_t  CurY; /* Current Y Value */

	D = 3 - (Radius << 1);

	CurX = 0;
	CurY = Radius;

	while (CurX <= CurY){
		if(CurY > 0){
			lcd_drawFastHLine(hlcd, Xpos - CurY, Ypos + CurX, 2*CurY, color);
			lcd_drawFastHLine(hlcd, Xpos - CurY, Ypos - CurX, 2*CurY, color);
		}

		if(CurX > 0){
			lcd_drawFastHLine(hlcd, Xpos - CurX, Ypos - CurY, 2*CurX, color);
			lcd_drawFastHLine(hlcd, Xpos - CurX, Ypos + CurY, 2*CurX, color);
		}
		if(D < 0){
			D += (CurX << 2) + 6;
		}
		else{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
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
	int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
	float K = 0, rad1 = 0, rad2 = 0;

	rad1 = XRadius;
	rad2 = YRadius;

	K = (float)(rad2/rad1);

	do{
		lcd_drawFastHLine(hlcd, (Xpos-(uint16_t)(x/K)), (Ypos+y), (2*(uint16_t)(x/K) + 1), pDrawProp->TextColor);
		lcd_drawFastHLine(hlcd, (Xpos-(uint16_t)(x/K)), (Ypos-y), (2*(uint16_t)(x/K) + 1), pDrawProp->TextColor);

		e2 = err;
		if(e2 <= x){
			err += ++x*2+1;
			if (-y == x && e2 <= y)
				e2 = 0;
		}
		if(e2 > y)
			err += ++y*2+1;
	}while (y <= 0);
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
	uint16_t  IMAGE_LEFT = 0, IMAGE_RIGHT = 0, IMAGE_TOP = 0, IMAGE_BOTTOM = 0;

	IMAGE_LEFT = IMAGE_RIGHT = Points->X;
	IMAGE_TOP= IMAGE_BOTTOM = Points->Y;

	for(counter = 1; counter < PointCount; counter++){
		pixelX = POLY_X(counter);
		if(pixelX < IMAGE_LEFT){
			IMAGE_LEFT = pixelX;
		}
		if(pixelX > IMAGE_RIGHT){
			IMAGE_RIGHT = pixelX;
		}

		pixelY = POLY_Y(counter);
		if(pixelY < IMAGE_TOP){
			IMAGE_TOP = pixelY;
		}
		if(pixelY > IMAGE_BOTTOM){
			IMAGE_BOTTOM = pixelY;
		}
	}

	if(PointCount < 2){
		return;
	}

	X_center = (IMAGE_LEFT + IMAGE_RIGHT)/2;
	Y_center = (IMAGE_BOTTOM + IMAGE_TOP)/2;

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
                       LTDC, DMA2D and DSI BSP Routines
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
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
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
		xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
		yinc2 = 0;                  /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         /* There are more x-values than y-values */
	}
	else{						/* There is at least one y-value for every x-value */
		xinc2 = 0;                  /* Don't change the x for every iteration */
		yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         /* There are more y-values than x-values */
	}

	for(curpixel = 0; curpixel <= numpixels; curpixel++){
		lcd_drawLine(pDrawProp, x, y, x3, y3);

		num += numadd;              /* Increase the numerator by the top of the fraction */
		if(num >= den){				/* Check if numerator >= denominator */
			num -= den;				/* Calculate the new numerator value */
			x += xinc1;               /* Change the x as appropriate */
			y += yinc1;               /* Change the y as appropriate */
		}
		x += xinc2;                 /* Change the x as appropriate */
		y += yinc2;                 /* Change the y as appropriate */
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
static
void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex)
{
	/* Register to memory mode with ARGB8888 as color Mode */
	hdma2d_discovery.Init.Mode         = DMA2D_R2M;
	hdma2d_discovery.Init.ColorMode    = CM_ARGB8888;
	hdma2d_discovery.Init.OutputOffset = OffLine;

	hdma2d_discovery.base = TADR_DMA2D_BASE;

	/* DMA2D Initialization */
	if(dma2d_init(&hdma2d_discovery) == E_OK){
		if(dma2d_configlayer(&hdma2d_discovery, LayerIndex) == E_OK){
			if(dma2d_start(&hdma2d_discovery, ColorIndex, (uint32_t)pDst, xSize, ySize) == E_OK){
				/* Polling For DMA transfer */
				dma2d_waittransfar(&hdma2d_discovery, 10);
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
  */
static
void LL_ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
{
	/* Configure the DMA2D Mode, Color Mode and output offset */
	hdma2d_discovery.Init.Mode         = DMA2D_M2M_PFC;
	hdma2d_discovery.Init.ColorMode    = CM_ARGB8888;
	hdma2d_discovery.Init.OutputOffset = 0;

	/* Foreground Configuration */
	hdma2d_discovery.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d_discovery.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d_discovery.LayerCfg[1].InputColorMode = ColorMode;
	hdma2d_discovery.LayerCfg[1].InputOffset = 0;

	hdma2d_discovery.base = TADR_DMA2D_BASE;

	/* DMA2D Initialization */
	if(dma2d_init(&hdma2d_discovery) == E_OK){
		if(dma2d_configlayer(&hdma2d_discovery, 1) == E_OK){
			if(dma2d_start(&hdma2d_discovery, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == E_OK){
				/* Polling For DMA transfer */
				dma2d_waittransfar(&hdma2d_discovery, 10);
			}
		}
	}
}

