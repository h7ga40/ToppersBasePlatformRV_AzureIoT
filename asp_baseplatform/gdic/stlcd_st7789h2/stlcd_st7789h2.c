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

#include "stlcd_st7789h2.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))


#define POLY_X(Z)              ((int32_t)((Points + Z)->X))
#define POLY_Y(Z)              ((int32_t)((Points + Z)->Y))
#define ABS(X)  ((X) > 0 ? (X) : -(X))      

#define FMC_BANK2_BASE  ((uint32_t)(0x60000000 | 0x04000000))
#define GPIO_AF12_FMC           ((uint8_t)0xCU)  /* FMC Alternate Function mapping                      */

#define GIOD_PINMAP     (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5  | GPIO_PIN_7 | GPIO_PIN_8 |\
						 GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15)
#define GIOE_PINMAP     (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |GPIO_PIN_10 |\
						 GPIO_PIN_11 | GPIO_PIN_12 |GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)
#define GIOF_PINMAP     (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |\
						 GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)
#define GIOG_PINMAP     (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_9)


typedef struct  {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} ST7789H2_Rgb888;

#define LCD_IO_WriteData(b, r) do { \
								  sil_wrh_mem((uint16_t *)((b)->fmc_base+2), (r)); \
								  asm ("dsb 0xF":::"memory"); \
							   } while(0)

#define LCD_IO_WriteReg(b, r)  do { \
								  sil_wrh_mem((uint16_t *)((b)->fmc_base), (r)); \
								  asm ("dsb 0xF":::"memory"); \
							   } while(0)

#define LCD_IO_ReadData(b)     (uint16_t)sil_reh_mem((uint16_t *)((b)->fmc_base+2))


/** @defgroup STM32F723E_DISCOVERY_LCD_Private_FunctionPrototypes STM32F723E Discovery Lcd Private Prototypes
  * @{
  */

static void FillTriangle(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);


/**
  * @brief  Initializes LCD low level.
  * @retval None
  */
static void LCD_IO_Init(void) 
{
	GPIO_Init_t GPIO_Init_Data;
	uint32_t pin, i;
	uint32_t tmpr = 0;

	/* Enable FMC clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR), RCC_AHB3ENR_FMCEN);

	/* Enable FSMC clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR), RCC_AHB3ENR_FMCEN);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_FMCRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_FMCRST);

	/* Enable GPIOs clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIODEN);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOEEN);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOFEN);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOGEN);

	GPIO_Init_Data.mode      = GPIO_MODE_AF;
	GPIO_Init_Data.pull      = GPIO_PULLUP;
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
	GPIO_Init_Data.speed     = GPIO_SPEED_HIGH;
	GPIO_Init_Data.alternate = GPIO_AF12_FMC;

	/* GPIOD configuration */ 
	/* LCD_PSRAM_D2, LCD_PSRAM_D3, LCD_PSRAM_NOE, LCD_PSRAM_NWE, PSRAM_NE1, LCD_PSRAM_D13, 
	   LCD_PSRAM_D14, LCD_PSRAM_D15, PSRAM_A16, PSRAM_A17, LCD_PSRAM_D0, LCD_PSRAM_D1 */
	pin = GIOD_PINMAP;
	for(i = 0 ; i < 16 ; i++){
		if((pin & (1<<i)) != 0)
			gpio_setup(TADR_GPIOD_BASE, &GPIO_Init_Data, i);
	}

	/* GPIOE configuration */
	/* PSRAM_NBL0, PSRAM_NBL1, LCD_PSRAM_D4, LCD_PSRAM_D5, LCD_PSRAM_D6, LCD_PSRAM_D7, 
	   LCD_PSRAM_D8, LCD_PSRAM_D9, LCD_PSRAM_D10, LCD_PSRAM_D11, LCD_PSRAM_D12 */
	pin = GIOE_PINMAP;
	for(i = 0 ; i < 16 ; i++){
		if((pin & (1<<i)) != 0)
			gpio_setup(TADR_GPIOE_BASE, &GPIO_Init_Data, i);
	}

	/* GPIOF configuration */
	/* PSRAM_A0, PSRAM_A1, PSRAM_A2, PSRAM_A3, PSRAM_A4, PSRAM_A5, 
	   PSRAM_A6, PSRAM_A7, PSRAM_A8, PSRAM_A9 */
	pin = GIOF_PINMAP;
	for(i = 0 ; i < 16 ; i++){
		if((pin & (1<<i)) != 0)
			gpio_setup(TADR_GPIOF_BASE, &GPIO_Init_Data, i);
	}

	/* GPIOG configuration */
	/* PSRAM_A10, PSRAM_A11, PSRAM_A12, PSRAM_A13, PSRAM_A14, PSRAM_A15, LCD_NE */
	pin = GIOG_PINMAP;
	for(i = 0 ; i < 16 ; i++){
		if((pin & (1<<i)) != 0)
			gpio_setup(TADR_GPIOG_BASE, &GPIO_Init_Data, i);
	}

	/* Initialize SRAM control Interface */
	tmpr = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR2));

	/* Clear MBKEN, MUXEN, MTYP, MWID, FACCEN, BURSTEN, WAITPOL, WAITCFG, WREN,
           WAITEN, EXTMOD, ASYNCWAIT, CBURSTRW and CCLKEN bits */
	tmpr &= ((uint32_t)~(FMC_BCR1_MBKEN     | FMC_BCR1_MUXEN    | FMC_BCR1_MTYP    | \
                       FMC_BCR1_MWID      | FMC_BCR1_FACCEN   | FMC_BCR1_BURSTEN | \
                       FMC_BCR1_WAITPOL   | FMC_BCR1_CPSIZE   | FMC_BCR1_WAITCFG | \
                       FMC_BCR1_WREN      | FMC_BCR1_WAITEN   | FMC_BCR1_EXTMOD  | \
                       FMC_BCR1_ASYNCWAIT | FMC_BCR1_CBURSTRW | FMC_BCR1_CCLKEN  | FMC_BCR1_WFDIS));

	/* Set NORSRAM device control parameters */
	tmpr |= (uint32_t)(FMC_BCR1_MWID_0 | FMC_BCR1_WREN | FMC_BCR1_WFDIS);

	sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR2), tmpr);

	/* Configure synchronous mode when Continuous clock is enabled for bank2..4 */
	sil_orw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR0), FMC_BCR1_WFDIS);

	/* Get the BTCR register value */
	tmpr = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR3));

	/* Clear ADDSET, ADDHLD, DATAST, BUSTURN, CLKDIV, DATLAT and ACCMOD bits */
	tmpr &= ((uint32_t)~(FMC_BTR1_ADDSET  | FMC_BTR1_ADDHLD | FMC_BTR1_DATAST | \
                       FMC_BTR1_BUSTURN | FMC_BTR1_CLKDIV | FMC_BTR1_DATLAT | \
                       FMC_BTR1_ACCMOD));

	/* Set FMC_NORSRAM device timing parameters */  
	tmpr |= (uint32_t)(9 /* AddressSetupTime */                 |\
                   ((2 /* AddressHoldTime */ ) << 4)          |\
                   ((6 /* DataSetupTime */ ) << 8)            |\
                   ((1 /* BusTurnAroundDuration */ ) << 16)   |\
                   (((2 /* CLKDivision */ )-1) << 20)         |\
                   (((2 /* DataLatency */)-2) << 24));

	sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR3), tmpr);

	/* Configure Clock division value (in NORSRAM bank 1) when continuous clock is enabled */
	if((sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR1)) & FMC_BCR1_CCLKEN) != 0){
		tmpr = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR1));
		tmpr &= ~(((uint32_t)0x0F) << 20);
		tmpr |= (uint32_t)(((2 /* CLKDivision */ )-1) << 20);
		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR1), tmpr);
	}

	/* Initialize SRAM extended mode timing Interface */
	sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BWTR2), 0xFFFFFFFF);

	/* Enable the NORSRAM device */
	sil_orw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_BTCR2), FMC_BCR1_MBKEN);
}


/**
  * @brief  Writes to the selected LCD register.
  * @param  Command: command value (or register address as named in st7789h2 doc).
  * @param  Parameters: pointer on parameters value (if command uses one or several parameters).
  * @param  NbParameters: number of command parameters (0 if no parameter)
  * @retval None
  */
static void
ST7789H2_WriteReg(LCD_Handler_t *hlcd, uint8_t Command, uint8_t *Parameters, uint8_t NbParameters)
{
	uint8_t   i;

	/* Send command */
	LCD_IO_WriteReg(hlcd, Command);

	/* Send command's parameters if any */
	for(i = 0 ; i<NbParameters; i++){
		LCD_IO_WriteData(hlcd, Parameters[i]);
	}
}

/**
  * @brief  Reads the selected LCD Register.
  * @param  Command: command value (or register address as named in st7789h2 doc).
  * @retval Register Value.
  */
static uint8_t
ST7789H2_ReadReg(LCD_Handler_t *hlcd, uint8_t Command)
{
	/* Send command */
	LCD_IO_WriteReg(hlcd, Command);

	/* Read dummy data */
	LCD_IO_ReadData(hlcd);

	/* Read register value */
	return (LCD_IO_ReadData(hlcd));
}

/**
  * @brief  Initialize the st7789h2 LCD Component.
  * @param  None
  * @retval None
  */
static void
ST7789H2_Init(LCD_Handler_t *hlcd)
{
	uint8_t   parameter[14];

	/* Initialize st7789h2 low level bus layer ----------------------------------*/
	/* Sleep In Command */ 
	ST7789H2_WriteReg(hlcd, ST7789H2_SLEEP_IN, (uint8_t*)NULL, 0); 
	/* Wait for 10ms */
	dly_tsk(10);

	/* SW Reset Command */
	ST7789H2_WriteReg(hlcd, 0x01, (uint8_t*)NULL, 0); 
	/* Wait for 200ms */
	dly_tsk(200);

	/* Sleep Out Command */
	ST7789H2_WriteReg(hlcd, ST7789H2_SLEEP_OUT, (uint8_t*)NULL, 0); 
	/* Wait for 120ms */
	dly_tsk(120);

	/* Normal display for Driver Down side */
	parameter[0] = 0x00;
	ST7789H2_WriteReg(hlcd, ST7789H2_NORMAL_DISPLAY, parameter, 1);

	/* Color mode 16bits/pixel */
	parameter[0] = 0x05;
	ST7789H2_WriteReg(hlcd, ST7789H2_COLOR_MODE, parameter, 1);

	/* Display inversion On */
	ST7789H2_WriteReg(hlcd, ST7789H2_DISPLAY_INVERSION, (uint8_t*)NULL, 0);

	/* Set Column address CASET */
	parameter[0] = 0x00;
	parameter[1] = 0x00;
	parameter[2] = 0x00;
	parameter[3] = 0xEF;
	ST7789H2_WriteReg(hlcd, ST7789H2_CASET, parameter, 4);
	/* Set Row address RASET */
	parameter[0] = 0x00;
	parameter[1] = 0x00;
	parameter[2] = 0x00;
	parameter[3] = 0xEF;
	ST7789H2_WriteReg(hlcd, ST7789H2_RASET, parameter, 4);

	/*--------------- ST7789H2 Frame rate setting -------------------------------*/
	/* PORCH control setting */      
	parameter[0] = 0x0C;
	parameter[1] = 0x0C;
	parameter[2] = 0x00;
	parameter[3] = 0x33;
	parameter[4] = 0x33; 
	ST7789H2_WriteReg(hlcd, ST7789H2_PORCH_CTRL, parameter, 5);

	/* GATE control setting */
	parameter[0] = 0x35; 
	ST7789H2_WriteReg(hlcd, ST7789H2_GATE_CTRL, parameter, 1);

	/*--------------- ST7789H2 Power setting ------------------------------------*/
	/* VCOM setting */ 
	parameter[0] = 0x1F;
	ST7789H2_WriteReg(hlcd, ST7789H2_VCOM_SET, parameter, 1); 

	/* LCM Control setting */
	parameter[0] = 0x2C; 
	ST7789H2_WriteReg(hlcd, ST7789H2_LCM_CTRL, parameter, 1);

	/* VDV and VRH Command Enable */
	parameter[0] = 0x01;
	parameter[1] = 0xC3;
	ST7789H2_WriteReg(hlcd, ST7789H2_VDV_VRH_EN, parameter, 2);

	/* VDV Set */ 
	parameter[0] = 0x20;
	ST7789H2_WriteReg(hlcd, ST7789H2_VDV_SET, parameter, 1);

	/* Frame Rate Control in normal mode */
	parameter[0] = 0x0F; 
	ST7789H2_WriteReg(hlcd, ST7789H2_FR_CTRL, parameter, 1);

	/* Power Control */
	parameter[0] = 0xA4;
	parameter[1] = 0xA1;
	ST7789H2_WriteReg(hlcd, ST7789H2_POWER_CTRL, parameter, 1); 

	/*--------------- ST7789H2 Gamma setting ------------------------------------*/
	/* Positive Voltage Gamma Control */ 
	parameter[0] = 0xD0;
	parameter[1] = 0x08;
	parameter[2] = 0x11;
	parameter[3] = 0x08;
	parameter[4] = 0x0C;
	parameter[5] = 0x15;
	parameter[6] = 0x39;
	parameter[7] = 0x33;
	parameter[8] = 0x50;
	parameter[9] = 0x36;
	parameter[10] = 0x13;
	parameter[11] = 0x14;
	parameter[12] = 0x29;
	parameter[13] = 0x2D;
	ST7789H2_WriteReg(hlcd, ST7789H2_PV_GAMMA_CTRL, parameter, 14); 

	/* Negative Voltage Gamma Control */
	parameter[0] = 0xD0;
	parameter[1] = 0x08;
	parameter[2] = 0x10;
	parameter[3] = 0x08;
	parameter[4] = 0x06;
	parameter[5] = 0x06;
	parameter[6] = 0x39;
	parameter[7] = 0x44;
	parameter[8] = 0x51;
	parameter[9] = 0x0B;
	parameter[10] = 0x16;
	parameter[11] = 0x14;
	parameter[12] = 0x2F;
	parameter[13] = 0x31;
	ST7789H2_WriteReg(hlcd, ST7789H2_NV_GAMMA_CTRL, parameter, 14); 

	/* Display ON command */
	lcd_displayOn(hlcd);

	/* Tearing Effect Line On: Option (00h:VSYNC Interface OFF, 01h:VSYNC Interface ON) */
	parameter[0] = 0x00;
	ST7789H2_WriteReg(hlcd, ST7789H2_TEARING_EFFECT, parameter, 1);
}

/**
  * @brief  Set the Display Orientation.
  * @param  orientation: ST7789H2_ORIENTATION_PORTRAIT, ST7789H2_ORIENTATION_LANDSCAPE
  *                      or ST7789H2_ORIENTATION_LANDSCAPE_ROT180  
  * @retval None
  */
static void
ST7789H2_SetOrientation(LCD_Handler_t *hlcd, uint32_t orientation)
{
	uint8_t   parameter[6];

	if(orientation == ST7789H2_ORIENTATION_LANDSCAPE){
		parameter[0] = 0x00;
	}
	else if(orientation == ST7789H2_ORIENTATION_LANDSCAPE_ROT180){
		/* Vertical Scrolling Definition */
		/* TFA describes the Top Fixed Area */
		parameter[0] = 0x00;
		parameter[1] = 0x00;
		/* VSA describes the height of the Vertical Scrolling Area */
		parameter[2] = 0x01;
		parameter[3] = 0xF0;
		/* BFA describes the Bottom Fixed Area */
		parameter[4] = 0x00;
		parameter[5] = 0x00; 
		ST7789H2_WriteReg(hlcd, ST7789H2_VSCRDEF, parameter, 6);

		/* Vertical Scroll Start Address of RAM */
		/* GRAM row nbr (320) - Display row nbr (240) = 80 = 0x50 */
		parameter[0] = 0x00;
		parameter[1] = 0x50;
		ST7789H2_WriteReg(hlcd, ST7789H2_VSCSAD, parameter, 2);

		parameter[0] = 0xC0;
	}
	else{
		parameter[0] = 0x60;
	}
	ST7789H2_WriteReg(hlcd, ST7789H2_NORMAL_DISPLAY, parameter, 1);
}

/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
static void
ST7789H2_SetCursor(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos)
{
  uint8_t   parameter[4];
  /* CASET: Comumn Addrses Set */
  parameter[0] = 0x00;     
  parameter[1] = 0x00 + Xpos;
  parameter[2] = 0x00;
  parameter[3] = 0xEF + Xpos;
  ST7789H2_WriteReg(hlcd, ST7789H2_CASET, parameter, 4);
  /* RASET: Row Addrses Set */  
  parameter[0] = 0x00;
  parameter[1] = 0x00 + Ypos;
  parameter[2] = 0x00;
  parameter[3] = 0xEF + Ypos;
  ST7789H2_WriteReg(hlcd, ST7789H2_RASET, parameter, 4);
}

/**
  * @brief  Read pixel from LCD RAM in RGB888 format
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval Each RGB pixel color components in a structure
  */
static ST7789H2_Rgb888
ST7789H2_ReadPixel_rgb888(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos)
{
	ST7789H2_Rgb888 rgb888;
	uint16_t         rgb888_part1, rgb888_part2;

  /* In LCD RAM, pixels are 24 bits packed and read with 16 bits access
   * Here is the pixels components arrangement in memory :
   *       bits:  15 14 13 12 11 10 09 08 | 07 06 05 04 03 02 01 00
   * address 0 :     red pixel 0    X  X  |   green pixel 0   X  X
   * address 1 :    blue pixel 0    X  X  |     red pixel 1   X  X
   * address 2 :   green pixel 1    X  X  |    blue pixel 1   X  X
   */

	/* Set Cursor */
	ST7789H2_SetCursor(hlcd, Xpos, Ypos);
	/* Prepare to read LCD RAM */
	ST7789H2_WriteReg(hlcd, ST7789H2_READ_RAM, (uint8_t*)NULL, 0);   /* RAM read data command */
	/* Dummy read */
	LCD_IO_ReadData(hlcd);
	/* Read first part of the RGB888 data */
	rgb888_part1 = LCD_IO_ReadData(hlcd);
	/* Read first part of the RGB888 data */
	rgb888_part2 = LCD_IO_ReadData(hlcd);

	/* red component */
	rgb888.red   = (rgb888_part1 & 0xFC00) >> 8;
	/* green component */
	rgb888.green = (rgb888_part1 & 0x00FC) >> 0;
	/* blue component */
	rgb888.blue  = (rgb888_part2 & 0xFC00) >> 8;
	return rgb888;
}

/**
  * @brief  Displays a single picture line.
  * @param  pdata: picture address.
  * @param  Xpos: Image X position in the LCD
  * @param  Ypos: Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @retval None
  */
static void
ST7789H2_DrawRGBHLine(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint8_t *pdata)
{
	uint32_t i = 0;
	uint32_t posX;
	uint16_t *rgb565 = (uint16_t*)pdata;

	/* Prepare to write to LCD RAM */
	ST7789H2_WriteReg(hlcd, ST7789H2_WRITE_RAM, (uint8_t*)NULL, 0);   /* RAM write data command */

	for(posX = Xpos ; posX < (Xsize + Xpos) ; posX++){
		if((posX >= hlcd->winXstart) && (Ypos >= hlcd->winYstart) &&     /* Check we are in the defined window */
			(posX <= hlcd->winXend) && (Ypos <= hlcd->winXend)){
			if(posX != (Xsize + Xpos)){	/* When writing last pixel when size is odd, the third part is not written */
				LCD_IO_WriteData(hlcd, rgb565[i]);
			}
			i++;
		}
	}
}

/*
 *  ST7789H2スクリーンの初期化関数
 */
ER
lcd_init(LCD_Handler_t *hlcd, uint32_t orientation)
{ 
	GPIO_Init_t GPIO_Init_Data;
	uint16_t id;

	hlcd->_width  = ST7789H2_LCD_PIXEL_WIDTH;
	hlcd->_height = ST7789H2_LCD_PIXEL_HEIGHT;
	hlcd->winXstart = 0;
	hlcd->winYstart = 0;
	hlcd->winXend = hlcd->_width - 1;
	hlcd->winXend = hlcd->_height - 1;

	/* Initialize LCD special pins GPIOs */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), hlcd->rst_clk);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), hlcd->te_clk);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), hlcd->bl_clk);

	GPIO_Init_Data.mode      = GPIO_MODE_OUTPUT;   /* LCD_RESET pin has to be manually controlled */
	GPIO_Init_Data.speed     = hlcd->gpio_speed;
	GPIO_Init_Data.pull      = hlcd->rst_pull;
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
	gpio_setup(hlcd->rst_base, &GPIO_Init_Data, hlcd->rst_pin);
	sil_wrw_mem((uint32_t *)(hlcd->rst_base+TOFF_GPIO_BSRR), (1<<hlcd->rst_pin) << 16);

	/* LCD_TE GPIO configuration */
	GPIO_Init_Data.mode      = GPIO_MODE_INPUT;        /* LCD_TE pin has to be manually managed */
	gpio_setup(hlcd->te_base, &GPIO_Init_Data, hlcd->te_pin);

	/* LCD_BL_CTRL GPIO configuration */
	GPIO_Init_Data.mode      = GPIO_MODE_OUTPUT;   /* LCD_BL_CTRL pin has to be manually controlled */
	GPIO_Init_Data.pull      = hlcd->bl_pull;
	gpio_setup(hlcd->bl_base, &GPIO_Init_Data, hlcd->bl_pin);

	/* Backlight control signal assertion */
	if(hlcd->bl_active == 0)
		sil_wrw_mem((uint32_t *)(hlcd->bl_base+TOFF_GPIO_BSRR), (1<<hlcd->bl_pin) << 16);
	else
		sil_wrw_mem((uint32_t *)(hlcd->bl_base+TOFF_GPIO_BSRR), (1<<hlcd->bl_pin));

	/* Apply hardware reset according to procedure indicated in FRD154BP2901 documentation */
	dly_tsk(5);   /* Reset signal asserted during 5ms  */
	sil_wrw_mem((uint32_t *)(hlcd->rst_base+TOFF_GPIO_BSRR), (1<<hlcd->rst_pin));
	dly_tsk(10);  /* Reset signal released during 10ms */
	sil_wrw_mem((uint32_t *)(hlcd->rst_base+TOFF_GPIO_BSRR), (1<<hlcd->rst_pin) << 16);
	dly_tsk(20);  /* Reset signal asserted during 20ms */
	sil_wrw_mem((uint32_t *)(hlcd->rst_base+TOFF_GPIO_BSRR), (1<<hlcd->rst_pin));
	dly_tsk(10);  /* Reset signal released during 10ms */

	LCD_IO_Init();
	id = ST7789H2_ReadReg(hlcd, ST7789H2_LCD_ID);
	if(id  == ST7789H2_ID){
		/* LCD Init */
		ST7789H2_Init(hlcd);

		if(orientation == LCD_ORIENTATION_PORTRAIT){
			ST7789H2_SetOrientation(hlcd, LCD_ORIENTATION_PORTRAIT);
		}
		else if(orientation == LCD_ORIENTATION_LANDSCAPE_ROT180){
			ST7789H2_SetOrientation(hlcd, LCD_ORIENTATION_LANDSCAPE_ROT180);
		}
		else{
			/* Default landscape orientation is selected */
		}
		return E_OK;
	}
	else
		return E_SYS;
}

/*
 *  ST7789H2スクリーンの終了関数
 */
ER
lcd_deinit(LCD_Handler_t *hlcd)
{
	/* Actually LcdDrv does not provide a DeInit function */
	return E_OK;
}

/*
 *  ST7789H2スクリーンレイヤ初期化関数
 */
ER
lcd_layerdefaultinit(LCD_Handler_t *hlcd, LCD_DrawProp_t *pDrawProp, uint16_t LayerIndex, uint32_t FB_Address)
{
	pDrawProp->BackColor = 0xFFFF;
	pDrawProp->TextColor = 0x0000;
	pDrawProp->hlcd      = hlcd;
	return E_OK;
}

/*
 *  ST7789H2表示ウィンドウ設定
 */
void
lcd_setAddrWindow(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
	if(Xpos < hlcd->_width){
		hlcd->winXstart = Xpos;
	}
	else{
		hlcd->winXstart = 0;
	}

	if(Ypos < hlcd->_height){
		hlcd->winYstart = Ypos;
	}
	else{
		hlcd->winYstart = 0;
	}

	if(Width  + Xpos <= hlcd->_width){
		hlcd->winXend = Width  + Xpos - 1;
	}
	else{
		hlcd->winXend = hlcd->_width - 1;
	}

	if(Height + Ypos <= hlcd->_height){
		hlcd->winXend = Height + Ypos - 1;
	}
	else{
		hlcd->winXend = hlcd->_height-1;
	}
}

/*
 *  ST7789H2スクリーンクリア
 */
void
lcd_clear(LCD_Handler_t *hlcd, uint16_t Color)
{
	uint32_t counter = 0;
	uint32_t y_size = 0;

	y_size =  hlcd->_height;

	for(counter = 0; counter < y_size; counter++){
		lcd_drawFastHLine(hlcd, 0, counter, hlcd->_width, Color);
	}
}

/*
 *  PIXEL読み出し
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 */
uint16_t
lcd_readPixel(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos)
{
	ST7789H2_Rgb888 rgb888;
	uint8_t          r, g, b;
	uint16_t         rgb565;

	/* Set Cursor */
	ST7789H2_SetCursor(hlcd, Xpos, Ypos);

	/* Read RGB888 data from LCD RAM */
	rgb888 = ST7789H2_ReadPixel_rgb888(hlcd, Xpos, Ypos);

	/* Convert RGB888 to RGB565 */
	r = ((rgb888.red & 0xF8) >> 3);    /* Extract the red component 5 most significant bits */
	g = ((rgb888.green & 0xFC) >> 2);  /* Extract the green component 6 most significant bits */
	b = ((rgb888.blue & 0xF8) >> 3);   /* Extract the blue component 5 most significant bits */

	rgb565 = ((uint16_t)(r) << 11) + ((uint16_t)(g) << 5) + ((uint16_t)(b) << 0);
	return (rgb565);
}

/*
 *  PIXEL描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 *  param4  color: color value
 */
void
lcd_drawPixel(LCD_Handler_t *hlcd, int16_t Xpos, int16_t Ypos, uint16_t RGB_Code)
{
	ST7789H2_SetCursor(hlcd, Xpos, Ypos);

	/* Prepare to write to LCD RAM */
	ST7789H2_WriteReg(hlcd, ST7789H2_WRITE_RAM, (uint8_t*)NULL, 0);   /* RAM write data command */

	/* Write RAM data */
	LCD_IO_WriteData(hlcd, RGB_Code);
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
	uint16_t counter = 0;

	/* Set Cursor */
	ST7789H2_SetCursor(hlcd, Xpos, Ypos);

	/* Prepare to write to LCD RAM */
	ST7789H2_WriteReg(hlcd, ST7789H2_WRITE_RAM, (uint8_t*)NULL, 0);   /* RAM write data command */

	/* Sent a complete line */
	for(counter = 0; counter < Length; counter++){
		LCD_IO_WriteData(hlcd, color);
	}
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
	uint16_t counter = 0;

	/* Set Cursor */
	ST7789H2_SetCursor(hlcd, Xpos, Ypos);

	/* Prepare to write to LCD RAM */
	ST7789H2_WriteReg(hlcd, ST7789H2_WRITE_RAM, (uint8_t*)NULL, 0);   /* RAM write data command */

	/* Fill a complete vertical line */
	for(counter = 0; counter < Length; counter++){
		lcd_drawPixel(hlcd, Xpos, Ypos + counter, color);
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
lcd_drawBitmap(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
	uint32_t height = 0;
	uint32_t width  = 0;
	uint32_t index = 0, size = 0;
	uint32_t posY;
	uint32_t nb_line = 0;
	uint16_t Xsize = hlcd->winXend - hlcd->winXstart + 1;
	uint16_t Ysize = hlcd->winXend - hlcd->winYstart + 1;

	/* Read bitmap width */
	width = pbmp[18] + (pbmp[19] << 8) + (pbmp[20] << 16)  + (pbmp[21] << 24);

	/* Read bitmap height */
	height = pbmp[22] + (pbmp[23] << 8) + (pbmp[24] << 16)  + (pbmp[25] << 24);

	lcd_setAddrWindow(hlcd, Xpos, Ypos, width, height);

	/* Read bitmap size */
	size = *(volatile uint16_t *) (pbmp + 2);
	size |= (*(volatile uint16_t *) (pbmp + 4)) << 16;
	/* Get bitmap data address offset */
	index = *(volatile uint16_t *) (pbmp + 10);
	index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;
	size = (size - index)/2;
	pbmp += index;

	for (posY = (Ypos + Ysize); posY > Ypos; posY--){	/* In BMP files the line order is inverted */
		/* Set Cursor */
		ST7789H2_SetCursor(hlcd, Xpos, posY - 1);

		/* Draw one line of the picture */
		ST7789H2_DrawRGBHLine(hlcd, Xpos, posY - 1, Xsize, (pbmp + (nb_line * Xsize * 2)));
		nb_line++;
	}
	lcd_setAddrWindow(hlcd, 0, 0, hlcd->_width, hlcd->_height);
}

/*
 *  バッファコピー
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  Xpos: コピー先X座標
 *  param3  Ypos: コピー先Y座標
 *  param4  Xsize: バッファ幅
 *  param5  Ysize: バッファ高さ
 *  param6  pdata: コピー元バッファポインタ
 */
void
lcd_drawRGBImage(LCD_Handler_t *hlcd, uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint8_t *pdata)
{
	uint32_t posY;
	uint32_t nb_line = 0;

	lcd_setAddrWindow(hlcd, Xpos, Ypos, Xsize, Ysize);

	for(posY = Ypos ; posY < (Ypos + Ysize) ; posY++){
		/* Set Cursor */
		ST7789H2_SetCursor(hlcd, Xpos, posY);

		/* Draw one line of the picture */
		ST7789H2_DrawRGBHLine(hlcd, Xpos, posY, Xsize, (pdata + (nb_line * Xsize * 2)));
		nb_line++;
	}
	lcd_setAddrWindow(hlcd, 0, 0, hlcd->_width, hlcd->_height);
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
	do{
		lcd_drawFastHLine(hlcd, Xpos, Ypos++, Width, color);
	}
	while(Height--);
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
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
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
		xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
		yinc2 = 0;                  /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         /* There are more x-values than y-values */
	}
	else{						/* There is at least one y-value for every x-value */
		xinc2 = 0;				/* Don't change the x for every iteration */
		yinc1 = 0;				/* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;		/* There are more y-values than x-values */
	}

	for(curpixel = 0; curpixel <= numpixels; curpixel++){
		lcd_drawPixel(pDrawProp->hlcd, x, y, pDrawProp->TextColor);  /* Draw the current pixel */
		num += numadd;                            /* Increase the numerator by the top of the fraction */
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
	/* Draw horizontal lines */
	lcd_drawFastHLine(pDrawProp->hlcd, Xpos, Ypos, Width, pDrawProp->TextColor);
	lcd_drawFastHLine(pDrawProp->hlcd, Xpos, (Ypos+ Height), Width, pDrawProp->TextColor);

	/* Draw vertical lines */
	lcd_drawFastVLine(pDrawProp->hlcd, Xpos, Ypos, Height, pDrawProp->TextColor);
	lcd_drawFastVLine(pDrawProp->hlcd, (Xpos + Width), Ypos, Height, pDrawProp->TextColor);
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
	int32_t  decision;       /* Decision Variable */ 
	uint32_t  current_x;   /* Current X Value */
	uint32_t  current_y;   /* Current Y Value */ 

	decision = 3 - (Radius << 1);
	current_x = 0;
	current_y = Radius;

	while (current_x <= current_y){
		lcd_drawPixel(pDrawProp->hlcd, (Xpos + current_x), (Ypos - current_y), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos - current_x), (Ypos - current_y), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos + current_y), (Ypos - current_x), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos - current_y), (Ypos - current_x), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos + current_x), (Ypos + current_y), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos - current_x), (Ypos + current_y), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos + current_y), (Ypos + current_x), pDrawProp->TextColor);
		lcd_drawPixel(pDrawProp->hlcd, (Xpos - current_y), (Ypos + current_x), pDrawProp->TextColor);

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
 *  PLOY-LINE描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  Points: Pointer to the points array
 *  param3  PointCount: Number of points
 */
void
lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount)
{
	int16_t x = 0, y = 0;

	if(PointCount < 2)
		return;

	lcd_drawLine(pDrawProp, Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);

	while(--PointCount){
		x = Points->X;
		y = Points->Y;
		Points++;
		lcd_drawLine(pDrawProp, x, y, Points->X, Points->Y);
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
	int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
	float k = 0, rad1 = 0, rad2 = 0;

	rad1 = XRadius;
	rad2 = YRadius;

	k = (float)(rad2/rad1);

	do{
		lcd_drawPixel(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos+y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (Xpos+(uint16_t)(x/k)), (Ypos+y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (Xpos+(uint16_t)(x/k)), (Ypos-y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos-y), pDrawProp->TextColor);

		e2 = err;
		if(e2 <= x){
			err += ++x*2+1;
			if(-y == x && e2 <= y)
				e2 = 0;
		}
		if(e2 > y)
			err += ++y*2+1;
	}while (y <= 0);
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
	int32_t  decision;        /* Decision Variable */ 
	uint32_t  current_x;    /* Current X Value */
	uint32_t  current_y;    /* Current Y Value */ 

	decision = 3 - (Radius << 1);
	current_x = 0;
	current_y = Radius;

	while(current_x <= current_y){
		if(current_y > 0){
			lcd_drawFastHLine(hlcd, Xpos - current_y, Ypos + current_x, 2*current_y, pDrawProp->TextColor);
			lcd_drawFastHLine(hlcd, Xpos - current_y, Ypos - current_x, 2*current_y, pDrawProp->TextColor);
		}

		if(current_x > 0){
			lcd_drawFastHLine(hlcd, Xpos - current_x, Ypos - current_y, 2*current_x, pDrawProp->TextColor);
			lcd_drawFastHLine(hlcd, Xpos - current_x, Ypos + current_y, 2*current_x, pDrawProp->TextColor);
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
	int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
	float k = 0, rad1 = 0, rad2 = 0;

	rad1 = XRadius;
	rad2 = YRadius;
	k = (float)(rad2/rad1);

	do{
		lcd_drawFastHLine(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos+y), (2*(uint16_t)(x/k) + 1), pDrawProp->TextColor);
		lcd_drawFastHLine(hlcd, (Xpos-(uint16_t)(x/k)), (Ypos-y), (2*(uint16_t)(x/k) + 1), pDrawProp->TextColor);

		e2 = err;
		if(e2 <= x){
			err += ++x*2+1;
			if(-y == x && e2 <= y)
				e2 = 0;
		}
		if(e2 > y)
			err += ++y*2+1;
	}
	while(y <= 0);
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

/*
 *  表示オン
 *  param1  hlcd: Pointer to LCD Handler
 */
void
lcd_displayOn(LCD_Handler_t *hlcd)
{
	/* Display ON command */
	ST7789H2_WriteReg(hlcd, ST7789H2_DISPLAY_ON, (uint8_t*)NULL, 0);

	/* Sleep Out command */
	ST7789H2_WriteReg(hlcd, ST7789H2_SLEEP_OUT, (uint8_t*)NULL, 0);
}

/*
 *  表示オフ
 *  param1  hlcd: Pointer to LCD Handler
 */
void
lcd_displayOff(LCD_Handler_t *hlcd)
{
	uint8_t   parameter[1];
	parameter[0] = 0xFE;
	/* Display OFF command */
	ST7789H2_WriteReg(hlcd, ST7789H2_DISPLAY_OFF, parameter, 1);
	/* Sleep In Command */
	ST7789H2_WriteReg(hlcd, ST7789H2_SLEEP_IN, (uint8_t*)NULL, 0);
	/* Wait for 10ms */
	dly_tsk(10);
}


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

	deltax = ABS(x2 - x1);	/* The difference between the x's */
	deltay = ABS(y2 - y1);	/* The difference between the y's */
	x = x1;					/* Start x off at the first pixel */
	y = y1;					/* Start y off at the first pixel */

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
		numadd = deltay;
		numpixels = deltax;	/* There are more x-values than y-values */
	}
	else{					/* There is at least one y-value for every x-value */
		xinc2 = 0;			/* Don't change the x for every iteration */
		yinc1 = 0;			/* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;	/* There are more y-values than x-values */
	}

	for(curpixel = 0 ; curpixel <= numpixels ; curpixel++){
		lcd_drawLine(pDrawProp, x, y, x3, y3);
		num += numadd;		/* Increase the numerator by the top of the fraction */
		if(num >= den){		/* Check if numerator >= denominator */
			num -= den;		/* Calculate the new numerator value */
			x += xinc1;		/* Change the x as appropriate */
			y += yinc1;		/* Change the y as appropriate */
		}
		x += xinc2;			/* Change the x as appropriate */
		y += yinc2;			/* Change the y as appropriate */
	}
}

