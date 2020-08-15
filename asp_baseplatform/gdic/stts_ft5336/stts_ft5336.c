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

#include <string.h>
#include "stts_ft5336.h"

/**
  * @brief  Reads a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @retval Data to be read
  */
static uint8_t
ft5336_IO_Read(TouchScreen_Handle_t *hts, uint8_t Reg)
{
	I2C_Init_t  Init;
	uint8_t read_value = 0;
	ER ercd = E_OK;

	ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &read_value, 1, 1000);

	/* Check the communication status */
	if(ercd != E_OK){
		/* De-initialize the I2C communication bus */
		i2c_deinit(hts->hi2c);

		/* Re-Initialize the I2C communication bus */
		memcpy(&Init, &hts->hi2c->Init, sizeof(I2C_Init_t));
		i2c_init(hts->hi2c->i2cid, &Init);
	}
	return read_value;
}

/**
  * @brief  Read the ft5336 device ID, pre initialize I2C in case of need to be
  *         able to read the FT5336 device ID, and verify this is a FT5336.
  * @param  DeviceAddr: I2C FT5336 Slave address.
  * @retval The Device ID (two bytes).
  */
static uint16_t
ft5336_ReadID(TouchScreen_Handle_t *hts, uint16_t DeviceAddr)
{
	volatile uint8_t ucReadId = 0;
	uint8_t nbReadAttempts = 0;
	uint8_t bFoundDevice = 0; /* Device not found by default */

	hts->I2C_Address = DeviceAddr;
	/* At maximum 4 attempts to read ID : exit at first finding of the searched device ID */
	for(nbReadAttempts = 0; ((nbReadAttempts < 3) && !(bFoundDevice)); nbReadAttempts++){
		/* Read register FT5336_CHIP_ID_REG as DeviceID detection */
		ucReadId = ft5336_IO_Read(hts, FT5336_CHIP_ID_REG);

		/* Found the searched device ID ? */
		if(ucReadId == FT5336_ID_VALUE){
			/* Set device as found */
			bFoundDevice = 1;
		}
	}
	return (ucReadId);
}

/**
  * @brief  Configure the FT5336 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  DeviceAddr: Device address on communication Bus (Slave I2C address of FT5336).
  * @retval None
  */
static void
ft5336_TS_DisableIT(TouchScreen_Handle_t *hts)
{
	ER ercd = E_OK;
	uint8_t regValue = 0;
	regValue = (FT5336_G_MODE_INTERRUPT_POLLING & (FT5336_G_MODE_INTERRUPT_MASK >> FT5336_G_MODE_INTERRUPT_SHIFT)) << FT5336_G_MODE_INTERRUPT_SHIFT;

	/* Set interrupt polling mode in FT5336_GMODE_REG */
	ercd = i2c_memwrite(hts->hi2c, hts->I2C_Address, FT5336_GMODE_REG, I2C_MEMADD_SIZE_8BIT, &regValue, 1, 1000);
	if(ercd != E_OK)
		syslog_1(LOG_ERROR, "ft5336_TS_DisableIT I2C write error(%d) !", ercd);
}

/**
  * @brief  Return if there is touches detected or not.
  *         Try to detect new touches and forget the old ones (reset internal global
  *         variables).
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval : Number of active touches detected (can be 0, 1 or 2).
  */
static uint8_t
ft5336_TS_DetectTouch(TouchScreen_Handle_t *hts)
{
	volatile uint8_t nbTouch = 0;

	/* Read register FT5336_TD_STAT_REG to check number of touches detection */
	nbTouch = ft5336_IO_Read(hts, FT5336_TD_STAT_REG);
	nbTouch &= FT5336_TD_STAT_MASK;

	if(nbTouch > FT5336_MAX_DETECTABLE_TOUCH){
		/* If invalid number of touch detected, set it to zero */
		nbTouch = 0;
	}

	/* Update ft5336 driver internal global : current number of active touches */
	hts->currActiveTouchNb = nbTouch;

	/* Reset current active touch index on which to work on */
	hts->currActiveTouchIdx = 0;
	return(nbTouch);
}

/**
  * @brief  Get the touch screen X and Y positions values
  *         Manage multi touch thanks to touch Index global
  *         variable 'TouchScreen_Handle.currActiveTouchIdx'.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  X: Pointer to X position value
  * @param  Y: Pointer to Y position value
  * @retval None.
  */
static void
ft5336_TS_GetXY(TouchScreen_Handle_t *hts, uint16_t *X, uint16_t *Y)
{
	volatile uint8_t ucReadData = 0;
	static uint16_t coord;
	uint8_t regAddressXLow = 0;
	uint8_t regAddressXHigh = 0;
	uint8_t regAddressYLow = 0;
	uint8_t regAddressYHigh = 0;

	if(hts->currActiveTouchIdx < hts->currActiveTouchNb){
		switch(hts->currActiveTouchIdx){
		case 0:
			regAddressXLow  = FT5336_P1_XL_REG;
			regAddressXHigh = FT5336_P1_XH_REG;
			regAddressYLow  = FT5336_P1_YL_REG;
			regAddressYHigh = FT5336_P1_YH_REG;
			break;
		case 1:
			regAddressXLow  = FT5336_P2_XL_REG;
			regAddressXHigh = FT5336_P2_XH_REG;
			regAddressYLow  = FT5336_P2_YL_REG;
			regAddressYHigh = FT5336_P2_YH_REG;
			break;
		case 2:
			regAddressXLow  = FT5336_P3_XL_REG;
			regAddressXHigh = FT5336_P3_XH_REG;
			regAddressYLow  = FT5336_P3_YL_REG;
			regAddressYHigh = FT5336_P3_YH_REG;
			break;
		case 3:
			regAddressXLow  = FT5336_P4_XL_REG;
			regAddressXHigh = FT5336_P4_XH_REG;
			regAddressYLow  = FT5336_P4_YL_REG;
			regAddressYHigh = FT5336_P4_YH_REG;
			break;
		case 4:
			regAddressXLow  = FT5336_P5_XL_REG;
			regAddressXHigh = FT5336_P5_XH_REG;
			regAddressYLow  = FT5336_P5_YL_REG;
			regAddressYHigh = FT5336_P5_YH_REG;
			break;
		case 5:
			regAddressXLow  = FT5336_P6_XL_REG;
			regAddressXHigh = FT5336_P6_XH_REG;
			regAddressYLow  = FT5336_P6_YL_REG;
			regAddressYHigh = FT5336_P6_YH_REG;
			break;
		case 6:
			regAddressXLow  = FT5336_P7_XL_REG;
			regAddressXHigh = FT5336_P7_XH_REG;
			regAddressYLow  = FT5336_P7_YL_REG;
			regAddressYHigh = FT5336_P7_YH_REG;
			break;
		case 7:
			regAddressXLow  = FT5336_P8_XL_REG;
			regAddressXHigh = FT5336_P8_XH_REG;
			regAddressYLow  = FT5336_P8_YL_REG;
			regAddressYHigh = FT5336_P8_YH_REG;
			break;
		case 8:
			regAddressXLow  = FT5336_P9_XL_REG;
			regAddressXHigh = FT5336_P9_XH_REG;
			regAddressYLow  = FT5336_P9_YL_REG;
			regAddressYHigh = FT5336_P9_YH_REG;
			break;
		case 9:
			regAddressXLow  = FT5336_P10_XL_REG;
			regAddressXHigh = FT5336_P10_XH_REG;
			regAddressYLow  = FT5336_P10_YL_REG;
			regAddressYHigh = FT5336_P10_YH_REG;
			break;
		default:
			break;

		} /* end switch(TouchScreen_Handle.currActiveTouchIdx) */

		/* Read low part of X position */
		ucReadData = ft5336_IO_Read(hts, regAddressXLow);
		coord = (ucReadData & FT5336_TOUCH_POS_LSB_MASK) >> FT5336_TOUCH_POS_LSB_SHIFT;

		/* Read high part of X position */
		ucReadData = ft5336_IO_Read(hts, regAddressXHigh);
		coord |= ((ucReadData & FT5336_TOUCH_POS_MSB_MASK) >> FT5336_TOUCH_POS_MSB_SHIFT) << 8;

		/* Send back ready X position to caller */
		*X = coord;

		/* Read low part of Y position */
		ucReadData = ft5336_IO_Read(hts, regAddressYLow);
		coord = (ucReadData & FT5336_TOUCH_POS_LSB_MASK) >> FT5336_TOUCH_POS_LSB_SHIFT;

		/* Read high part of Y position */
		ucReadData = ft5336_IO_Read(hts, regAddressYHigh);
		coord |= ((ucReadData & FT5336_TOUCH_POS_MSB_MASK) >> FT5336_TOUCH_POS_MSB_SHIFT) << 8;

		/* Send back ready Y position to caller */
		*Y = coord;

		hts->currActiveTouchIdx++;	/* next call will work on next touch */
	} /* of if(TouchScreen_Handle.currActiveTouchIdx < TouchScreen_Handle.currActiveTouchNb) */
}


/**** NEW FEATURES enabled when Multi-touch support is enabled ****/

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
/**
  * @brief  Get the touch detailed informations on touch number 'touchIdx' (0..1)
  *         This touch detailed information contains :
  *         - weight that was applied to this touch
  *         - sub-area of the touch in the touch panel
  *         - event of linked to the touch (press down, lift up, ...)
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT5336).
  * @param  touchIdx : Passed index of the touch (0..1) on which we want to get the
  *                    detailed information.
  * @param  pWeight : Pointer to to get the weight information of 'touchIdx'.
  * @param  pArea   : Pointer to to get the sub-area information of 'touchIdx'.
  * @param  pEvent  : Pointer to to get the event information of 'touchIdx'.

  * @retval None.
  */
static void
ft5336_TS_GetTouchInfo(TouchScreen_Handle_t *hts, uint32_t touchIdx, uint32_t *pWeight, uint32_t *pArea, uint32_t *pEvent)
{
	volatile uint8_t ucReadData = 0;
	uint8_t regAddressXHigh = 0;
	uint8_t regAddressPWeight = 0;
	uint8_t regAddressPMisc = 0;

	if(touchIdx < hts->currActiveTouchNb){
		switch(touchIdx){
		case 0:
			regAddressXHigh   = FT5336_P1_XH_REG;
			regAddressPWeight = FT5336_P1_WEIGHT_REG;
			regAddressPMisc   = FT5336_P1_MISC_REG;
			break;
		case 1:
			regAddressXHigh   = FT5336_P2_XH_REG;
			regAddressPWeight = FT5336_P2_WEIGHT_REG;
			regAddressPMisc   = FT5336_P2_MISC_REG;
			break;
		case 2:
			regAddressXHigh   = FT5336_P3_XH_REG;
			regAddressPWeight = FT5336_P3_WEIGHT_REG;
			regAddressPMisc   = FT5336_P3_MISC_REG;
			break;
		case 3:
			regAddressXHigh   = FT5336_P4_XH_REG;
			regAddressPWeight = FT5336_P4_WEIGHT_REG;
			regAddressPMisc   = FT5336_P4_MISC_REG;
			break;
		case 4:
			regAddressXHigh   = FT5336_P5_XH_REG;
			regAddressPWeight = FT5336_P5_WEIGHT_REG;
			regAddressPMisc   = FT5336_P5_MISC_REG;
			break;
		case 5:
			regAddressXHigh   = FT5336_P6_XH_REG;
			regAddressPWeight = FT5336_P6_WEIGHT_REG;
			regAddressPMisc   = FT5336_P6_MISC_REG;
			break;
		case 6:
			regAddressXHigh   = FT5336_P7_XH_REG;
			regAddressPWeight = FT5336_P7_WEIGHT_REG;
			regAddressPMisc   = FT5336_P7_MISC_REG;
			break;
		case 7:
			regAddressXHigh   = FT5336_P8_XH_REG;
			regAddressPWeight = FT5336_P8_WEIGHT_REG;
			regAddressPMisc   = FT5336_P8_MISC_REG;
			break;
		case 8:
			regAddressXHigh   = FT5336_P9_XH_REG;
			regAddressPWeight = FT5336_P9_WEIGHT_REG;
			regAddressPMisc   = FT5336_P9_MISC_REG;
			break;
		case 9:
			regAddressXHigh   = FT5336_P10_XH_REG;
			regAddressPWeight = FT5336_P10_WEIGHT_REG;
			regAddressPMisc   = FT5336_P10_MISC_REG;
			break;
		default:
			break;
		} /* end switch(touchIdx) */

		/* Read Event Id of touch index */
		ucReadData = ft5336_IO_Read(hts, regAddressXHigh);
		*pEvent = (ucReadData & FT5336_TOUCH_EVT_FLAG_MASK) >> FT5336_TOUCH_EVT_FLAG_SHIFT;

		/* Read weight of touch index */
		ucReadData = ft5336_IO_Read(hts, regAddressPWeight);
		*pWeight = (ucReadData & FT5336_TOUCH_WEIGHT_MASK) >> FT5336_TOUCH_WEIGHT_SHIFT;

		/* Read area of touch index */
		ucReadData = ft5336_IO_Read(hts, regAddressPMisc);
		*pArea = (ucReadData & FT5336_TOUCH_AREA_MASK) >> FT5336_TOUCH_AREA_SHIFT;
	} /* of if(touchIdx < TouchScreen_Handle.currActiveTouchNb) */
}
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */


/**
  * @brief  Initializes and configures the touch screen functionalities and 
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..).
  * @param  ts_SizeX: Maximum X size of the TS area on LCD
  * @param  ts_SizeY: Maximum Y size of the TS area on LCD
  * @param  orientation : TS_ORIENTATION_LANDSCAPE or TS_ORIENTATION_PORTRAIT
  * @retval E_OK if all initializations are OK. Other value if error.
  */
ER
touchscreen_init(TouchScreen_Handle_t *hts, uint16_t ts_SizeX, uint16_t ts_SizeY, uint8_t orientation)
{
	ER ercd  = E_OK;
	hts->tsXBoundary = ts_SizeX;
	hts->tsYBoundary = ts_SizeY;

	/* Read ID and verify if the touch screen driver is ready */
	if(ft5336_ReadID(hts, FT5336_I2C_SLAVE_ADDRESS) == FT5336_ID_VALUE){
		/* Initialize the TS driver structure */
		hts->I2C_Address = FT5336_I2C_SLAVE_ADDRESS;
		hts->tsOrientation = TS_SWAP_XY;

		/* Initialize the TS driver */
		ft5336_TS_DisableIT(hts);
	}
	else
		ercd = E_NOSPT;
	return ercd;
}

/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval E_OK if all initializations are OK. Other value if error.
  */
ER
touchscreen_getstate(TouchScreen_Handle_t *hts, TS_StateTypeDef *TS_State)
{
	static uint32_t _x[TS_MAX_NB_TOUCH] = {0, 0};
	static uint32_t _y[TS_MAX_NB_TOUCH] = {0, 0};
	ER  ercd = E_OK;
	uint16_t x[TS_MAX_NB_TOUCH];
	uint16_t y[TS_MAX_NB_TOUCH];
	uint16_t brute_x[TS_MAX_NB_TOUCH];
	uint16_t brute_y[TS_MAX_NB_TOUCH];
	uint16_t x_diff;
	uint16_t y_diff;
	uint32_t index;
#if (TS_MULTI_TOUCH_SUPPORTED == 1)
	uint32_t weight = 0;
	uint32_t area = 0;
	uint32_t event = 0;
	uint32_t gestureId = 0;
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

	/* Check and update the number of touches active detected */
	TS_State->touchDetected = ft5336_TS_DetectTouch(hts);

	if(TS_State->touchDetected){
		for(index=0; index < TS_State->touchDetected; index++){
			/* Get each touch coordinates */
			ft5336_TS_GetXY(hts, &(brute_x[index]), &(brute_y[index]));

			if(hts->tsOrientation == TS_SWAP_NONE){
				x[index] = brute_x[index];
				y[index] = brute_y[index];
			}
			if(hts->tsOrientation & TS_SWAP_X){
				x[index] = 4096 - brute_x[index];
			}
			if(hts->tsOrientation & TS_SWAP_Y){
				y[index] = 4096 - brute_y[index];
			}
			if(hts->tsOrientation & TS_SWAP_XY){
				y[index] = brute_x[index];
				x[index] = brute_y[index];
			}

			x_diff = x[index] > _x[index]? (x[index] - _x[index]): (_x[index] - x[index]);
			y_diff = y[index] > _y[index]? (y[index] - _y[index]): (_y[index] - y[index]);

			if((x_diff + y_diff) > 5){
				_x[index] = x[index];
				_y[index] = y[index];
			}
			if(hts->I2C_Address == FT5336_I2C_SLAVE_ADDRESS){
				TS_State->touchX[index] = x[index];
				TS_State->touchY[index] = y[index];
			}
			else{
				/* 2^12 = 4096 : indexes are expressed on a dynamic of 4096 */
				TS_State->touchX[index] = (hts->tsXBoundary * _x[index]) >> 12;
				TS_State->touchY[index] = (hts->tsYBoundary * _y[index]) >> 12;
			}

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
			/* Get touch info related to the current touch */
			ft5336_TS_GetTouchInfo(hts, index, &weight, &area, &event);

			/* Update TS_State structure */
			TS_State->touchWeight[index] = weight;
			TS_State->touchArea[index]   = area;

			/* Remap touch event */
			switch(event){
			case FT5336_TOUCH_EVT_FLAG_PRESS_DOWN	:
				TS_State->touchEventId[index] = TOUCH_EVENT_PRESS_DOWN;
				break;
			case FT5336_TOUCH_EVT_FLAG_LIFT_UP :
				TS_State->touchEventId[index] = TOUCH_EVENT_LIFT_UP;
				break;
			case FT5336_TOUCH_EVT_FLAG_CONTACT :
				TS_State->touchEventId[index] = TOUCH_EVENT_CONTACT;
				break;
			case FT5336_TOUCH_EVT_FLAG_NO_EVENT :
				TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
				break;
			default:
				ercd = E_SYS;
				break;
			} /* of switch(event) */
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

		} /* of for(index=0; index < TS_State->touchDetected; index++) */

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
		/* Get gesture Id */
		gestureId = ft5336_IO_Read(hts, FT5336_GEST_ID_REG);

		/* Remap gesture Id to a TS_GestureIdTypeDef value */
		switch(gestureId){
		case FT5336_GEST_ID_NO_GESTURE :
			TS_State->gestureId = GEST_ID_NO_GESTURE;
			break;
		case FT5336_GEST_ID_MOVE_UP :
			TS_State->gestureId = GEST_ID_MOVE_UP;
			break;
		case FT5336_GEST_ID_MOVE_RIGHT :
			TS_State->gestureId = GEST_ID_MOVE_RIGHT;
			break;
		case FT5336_GEST_ID_MOVE_DOWN :
			TS_State->gestureId = GEST_ID_MOVE_DOWN;
			break;
		case FT5336_GEST_ID_MOVE_LEFT :
			TS_State->gestureId = GEST_ID_MOVE_LEFT;
			break;
		case FT5336_GEST_ID_ZOOM_IN :
			TS_State->gestureId = GEST_ID_ZOOM_IN;
			break;
		case FT5336_GEST_ID_ZOOM_OUT :
			TS_State->gestureId = GEST_ID_ZOOM_OUT;
			break;
		default:
			ercd = E_SYS;
			break;
		} /* of switch(gestureId) */
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

	} /* end of if(TS_State->touchDetected != 0) */
	return ercd;
}

/**
  * @brief  Function used to reset all touch data before a new acquisition
  *         of touch information.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval E_OK if OK, TE_ERROR if problem found.
  */
ER
touchscreen_reset_data(TS_StateTypeDef *TS_State)
{
	ER       ercd = E_PAR;
	uint32_t index;

	if(TS_State != (TS_StateTypeDef *)NULL){
		TS_State->gestureId = GEST_ID_NO_GESTURE;
		TS_State->touchDetected = 0;

		for(index = 0; index < TS_MAX_NB_TOUCH; index++){
			TS_State->touchX[index]       = 0;
			TS_State->touchY[index]       = 0;
			TS_State->touchArea[index]    = 0;
			TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
			TS_State->touchWeight[index]  = 0;
		}
		ercd = E_OK;
	} /* of if (TS_State != (TS_StateTypeDef *)NULL) */
	return ercd;
}

