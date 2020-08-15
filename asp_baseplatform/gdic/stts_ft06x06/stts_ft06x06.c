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


/* Includes ------------------------------------------------------------------*/
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"
#include "i2c.h"
#include "stts_ft06x06.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))

/**
  * @brief TouchScreen FT6206 Slave I2C address
  */
#define TS_I2C_ADDRESS                   ((uint16_t)0x70)


/* Table for touchscreen event information display on LCD : table indexed on enum @ref TS_TouchEventTypeDef information */
char * ts_event_string_tab[TOUCH_EVENT_NB_MAX] = { "None",
                                                   "Press down",
                                                   "Lift up",
                                                   "Contact"
                                                  };

/* Table for touchscreen gesture Id information display on LCD : table indexed on enum @ref TS_GestureIdTypeDef information */
char * ts_gesture_id_string_tab[GEST_ID_NB_MAX] = { "None",
                                                    "Move Up",
                                                    "Move Right",
                                                    "Move Down",
                                                    "Move Left",
                                                    "Zoom In",
                                                    "Zoom Out"
                                                  };




/* Private functions prototypes-----------------------------------------------*/
/**
  * @}
  */

/** @defgroup ft6x06_Private_Functions ft6x06 Private Functions
  * @{
  */

/**
  * @brief  Read the ft6x06 device ID, pre initialize I2C in case of need to be
  *         able to read the FT6206 device ID, and verify this is a FT6206.
  * @param  DeviceAddr: I2C FT6x06 Slave address.
  * @retval The Device ID (two bytes).
  */
static ER
ft6x06_ReadID(TouchScreen_Handle_t *hts, uint8_t *ts_id)
{
	uint8_t read_value = 0;
	ER ercd = E_OK;

	ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_CHIP_ID_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&read_value, 1, 1000);
	*ts_id = read_value;
	return ercd;
}

/**
  * @brief  Return if there is touches detected or not.
  *         Try to detect new touches and forget the old ones (reset internal global
  *         variables).
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval : Number of active touches detected (can be 0, 1 or 2).
  */
static ER_UINT
ft6x06_TS_DetectTouch(TouchScreen_Handle_t *hts)
{
	ER ercd = E_OK;
	volatile uint8_t nbTouch = 0;

	/* Read register FT6206_TD_STAT_REG to check number of touches detection */
	ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_TD_STAT_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&nbTouch, 1, 1000);
	/* Check the communication status */
	if(ercd != E_OK)
		return ercd;

	nbTouch &= FT6206_TD_STAT_MASK;

	if(nbTouch > FT6206_MAX_DETECTABLE_TOUCH){
		/* If invalid number of touch detected, set it to zero */
		nbTouch = 0;
	}

	/* Update ft6x06 driver internal global : current number of active touches */
	hts->currActiveTouchNb = nbTouch;

	/* Reset current active touch index on which to work on */
	hts->currActiveTouchIdx = 0;
	return nbTouch;
}

/**
  * @brief  Get the touch screen X and Y positions values
  *         Manage multi touch thanks to touch Index global
  *         variable 'ft6x06_handle.currActiveTouchIdx'.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  X: Pointer to X position value
  * @param  Y: Pointer to Y position value
  * @retval None.
  */
static ER
ft6x06_TS_GetXY(TouchScreen_Handle_t *hts, uint16_t *X, uint16_t *Y)
{
	uint8_t regAddress = 0;
	uint8_t  dataxy[4];
	ER ercd = E_OK;

	if(hts->currActiveTouchIdx < hts->currActiveTouchNb){
		switch(hts->currActiveTouchIdx){
		case 0 :
			regAddress = FT6206_P1_XH_REG;
			break;
		case 1 :
			regAddress = FT6206_P2_XH_REG; 
			break;

		default :
			break;
 		}

		/* Read X and Y positions */
		ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)regAddress, I2C_MEMADD_SIZE_8BIT, dataxy, sizeof(dataxy), 1000);
		if(ercd != E_OK)
			return ercd;
		/* Send back ready X position to caller */
		*X = ((dataxy[0] & FT6206_MSB_MASK) << 8) | (dataxy[1] & FT6206_LSB_MASK);

		/* Send back ready Y position to caller */
		*Y = ((dataxy[2] & FT6206_MSB_MASK) << 8) | (dataxy[3] & FT6206_LSB_MASK);
		hts->currActiveTouchIdx++;
	}
	return ercd;
}

/**** NEW FEATURES enabled when Multi-touch support is enabled ****/

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
/**
  * @brief  Get the touch detailed informations on touch number 'touchIdx' (0..1)
  *         This touch detailed information contains :
  *         - weight that was applied to this touch
  *         - sub-area of the touch in the touch panel
  *         - event of linked to the touch (press down, lift up, ...)
  * @param  DeviceAddr: Device address on communication Bus (I2C slave address of FT6x06).
  * @param  touchIdx : Passed index of the touch (0..1) on which we want to get the
  *                    detailed information.
  * @param  pWeight : Pointer to to get the weight information of 'touchIdx'.
  * @param  pArea   : Pointer to to get the sub-area information of 'touchIdx'.
  * @param  pEvent  : Pointer to to get the event information of 'touchIdx'.

  * @retval None.
  */
static ER
ft6x06_TS_GetTouchInfo(TouchScreen_Handle_t *hts,
                            uint32_t   touchIdx,
                            uint32_t * pWeight,
                            uint32_t * pArea,
                            uint32_t * pEvent)
{
	uint8_t regAddress = 0;
	uint8_t dataxy[3];
	ER ercd = E_OK;

	if(touchIdx < hts->currActiveTouchNb){
		switch(touchIdx){
		case 0 :
			regAddress = FT6206_P1_WEIGHT_REG;
			break;
		case 1 :
			regAddress = FT6206_P2_WEIGHT_REG;
			break;
		default :
			break;
		} /* end switch(touchIdx) */

		/* Read weight, area and Event Id of touch index */
		ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)regAddress, I2C_MEMADD_SIZE_8BIT, dataxy, sizeof(dataxy), 1000);

		/* Return weight of touch index */
		*pWeight = (dataxy[0] & FT6206_TOUCH_WEIGHT_MASK) >> FT6206_TOUCH_WEIGHT_SHIFT;
		/* Return area of touch index */
		*pArea = (dataxy[1] & FT6206_TOUCH_AREA_MASK) >> FT6206_TOUCH_AREA_SHIFT;
		/* Return Event Id  of touch index */
		*pEvent = (dataxy[2] & FT6206_TOUCH_EVT_FLAG_MASK) >> FT6206_TOUCH_EVT_FLAG_SHIFT;
	} /* of if(touchIdx < ft6x06_handle.currActiveTouchNb) */
	return ercd;
}

#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

#if (TS_AUTO_CALIBRATION_SUPPORTED == 1)
/**
  * @brief  Start TouchScreen calibration phase
  * @param  DeviceAddr: FT6206 Device address for communication on I2C Bus.
  * @retval Status FT6206_STATUS_OK or FT6206_STATUS_NOT_OK.
  */
static ER
ft6x06_TS_Calibration(TouchScreen_Handle_t *hts)
{
	uint32_t nbAttempt = 0;
	volatile uint8_t ucReadData;
	volatile uint8_t regValue;
	uint8_t bEndCalibration = 0;
	ER ercd = E_OK;

	/* >> Calibration sequence start */

	/* Switch FT6206 back to factory mode to calibrate */
	regValue = (FT6206_DEV_MODE_FACTORY & FT6206_DEV_MODE_MASK) << FT6206_DEV_MODE_SHIFT;
	ercd = i2c_memwrite(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_DEV_MODE_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&regValue, 1, 1000);
	if(ercd != E_OK)
		return ercd;

	/* Read back the same register FT6206_DEV_MODE_REG */
	ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_GEST_ID_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&ucReadData, 1, 1000);
	if(ercd != E_OK)
		return ercd;

	dly_tsk(300);	/* Wait 300 ms */

	if(((ucReadData & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT) != FT6206_DEV_MODE_FACTORY ){
		/* Return error to caller */
		return E_OBJ;
	}

	/* Start calibration command */
	regValue = 0x04;
	ercd = i2c_memwrite(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_TD_STAT_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&regValue, 1, 1000);
	if(ercd != E_OK)
		return ercd;

	dly_tsk(300);	/* Wait 300 ms */

	/* 100 attempts to wait switch from factory mode (calibration) to working mode */
	for (nbAttempt=0; ((nbAttempt < 100) && (!bEndCalibration)) ; nbAttempt++){
		ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_DEV_MODE_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&ucReadData, 1, 1000);
		if(ercd != E_OK)
			return ercd;

		ucReadData = (ucReadData & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT;
		if(ucReadData == FT6206_DEV_MODE_WORKING){
			/* Auto Switch to FT6206_DEV_MODE_WORKING : means calibration have ended */
			bEndCalibration = 1; /* exit for loop */
		}
		dly_tsk(200);	/* Wait 200 ms */
	}

	/* Calibration sequence end << */
	return ercd;
}
#endif /* TS_AUTO_CALIBRATION_SUPPORTED == 1 */

/**
  * @brief  Initializes and configures the touch screen functionalities and
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..)
  *         with a given orientation
  * @param  ts_SizeX : Maximum X size of the TS area on LCD
  * @param  ts_SizeY : Maximum Y size of the TS area on LCD
  * @param  orientation : TS_ORIENTATION_LANDSCAPE or TS_ORIENTATION_PORTRAIT
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
ER
touchscreen_init(TouchScreen_Handle_t *hts, uint16_t ts_SizeX, uint16_t ts_SizeY, uint8_t orientation)
{
	GPIO_Init_t GPIO_Init_Data;
	ER ercd = E_OK;
	uint8_t ts_id = 0;

	/* Note : I2C_Address is un-initialized here, but is not used at all in init function */
	/* but the prototype of Init() is like that in template and should be respected       */

	/* Initialize the communication channel to sensor (I2C) if necessary */
	/* that is initialization is done only once after a power up         */
	hts->tsOrientation = TS_SWAP_NONE;
	if(hts->I2C_Address == 0)
		hts->I2C_Address   = TS_I2C_ADDRESS;
	hts->initialized   = false;
	if(hts->pcb->reset_gpio_base){
		sil_orw_mem((uint32_t *)(hts->pcb->reset_clk_base), hts->pcb->reset_clk_bit);
		GPIO_Init_Data.mode  = hts->pcb->reset_gpio_mode;
		GPIO_Init_Data.otype = hts->pcb->reset_gpio_otype;
		GPIO_Init_Data.pull  = hts->pcb->reset_gpio_pull;
		GPIO_Init_Data.speed = hts->pcb->reset_gpio_speed;
		gpio_setup(hts->pcb->reset_gpio_base, &GPIO_Init_Data, hts->pcb->reset_gpio_pinno);
		if(hts->pcb->reset_gpio_release == 0)
			sil_wrw_mem((uint32_t *)(hts->pcb->reset_gpio_base+TOFF_GPIO_BSRR), (1<<hts->pcb->reset_gpio_pinno)<<16);
		else
			sil_wrw_mem((uint32_t *)(hts->pcb->reset_gpio_base+TOFF_GPIO_BSRR), (1<<hts->pcb->reset_gpio_pinno));
	}

	/* Scan FT6x36 TouchScreen IC controller ID register by I2C Read */
	/* Verify this is a FT6x36, otherwise this is an error case      */
	ercd = ft6x06_ReadID(hts, &ts_id);
	if(ercd != E_OK)
		return ercd;
	if(ts_id == FT6x36_ID_VALUE){
		/* Found FT6x36 : Initialize the TS driver structure */
		/* Get LCD chosen orientation */
		if(orientation == TS_ORIENTATION_PORTRAIT){
			hts->tsOrientation = TS_SWAP_X | TS_SWAP_Y;               
		}
		else if(orientation == TS_ORIENTATION_LANDSCAPE_ROT180){
			hts->tsOrientation = TS_SWAP_XY;
		}
		else{
			hts->tsOrientation = TS_SWAP_XY | TS_SWAP_Y;                 
		}
		hts->max_width  = FT_6206_MAX_WIDTH_HEIGHT;
		hts->max_height = FT_6206_MAX_WIDTH_HEIGHT;
	}
	/* Scan FT6xx6 TouchScreen IC controller ID register by I2C Read       */
	/* Verify this is a FT6206 or FT6336G, otherwise this is an error case */
	else if(ts_id == FT6206_ID_VALUE){
		/* Found FT6206 : Initialize the TS driver structure */
		/* Get LCD chosen orientation */
		if(ts_SizeX < ts_SizeY){
			hts->tsOrientation = TS_SWAP_NONE;
		}
		else{
			hts->tsOrientation = TS_SWAP_XY | TS_SWAP_Y;
		}
		hts->max_width  = FT_6206_MAX_WIDTH;
		hts->max_height = FT_6206_MAX_HEIGHT;
	}

	if(ercd == E_OK){
		/* Software reset the TouchScreen */
		/* Calibrate, Configure and Start the TouchScreen driver */
#if (TS_AUTO_CALIBRATION_SUPPORTED == 1)
		/* Hw Calibration sequence start : should be done once after each power up */
		/* This is called internal calibration of the touch screen                 */
		ft6x06_TS_Calibration(hts);
#endif

		/* By default set FT6206 IC in Polling mode : no INT generation on FT6206 for new touch available */
		/* Note TS_INT is active low                                                                      */
		ercd = touchscreen_disable_it(hts);
			/* Configure Interrupt mode for TS_INT pin falling edge : when a new touch is available */
			/* TS_INT pin is active on low level on new touch available */
		if(ercd == E_OK && hts->pcb->int_gpio_base != 0){
			exti_func[hts->pcb->int_gpio_pinno] = hts->ifunc;
			sil_orw_mem((uint32_t *)hts->pcb->int_clk_base, hts->pcb->int_clk_bit);
			GPIO_Init_Data.mode      = hts->pcb->int_gpio_mode;
			GPIO_Init_Data.pull      = hts->pcb->int_gpio_pull;
			GPIO_Init_Data.speed     = hts->pcb->int_gpio_speed;
			gpio_setup(hts->pcb->int_gpio_base, &GPIO_Init_Data, hts->pcb->int_gpio_pinno);
			if(hts->enable_it)
				ercd = touchscreen_enable_it(hts);
		}
		if(ercd == E_OK)
			hts->initialized = true;
	}
	return (ercd);
}

/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
ER
touchscreen_getstate(TouchScreen_Handle_t *hts, TS_StateTypeDef *TS_State)
{
	static uint32_t _x[TS_MAX_NB_TOUCH] = {0, 0};
	static uint32_t _y[TS_MAX_NB_TOUCH] = {0, 0};
	uint16_t tmp;
	uint16_t Raw_x[TS_MAX_NB_TOUCH];
	uint16_t Raw_y[TS_MAX_NB_TOUCH];
	uint16_t xDiff;
	uint16_t yDiff;
	uint32_t index;
#if (TS_MULTI_TOUCH_SUPPORTED == 1)
	uint32_t weight = 0;
	uint32_t area = 0;
	uint32_t event = 0;
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */
	ER ercd = E_OK;

	/* Check and update the number of touches active detected */
	ercd = ft6x06_TS_DetectTouch(hts);
	if(ercd < 0)
		return ercd;
	if((TS_State->touchDetected = (uint8_t)ercd)){
		for(index=0; index < TS_State->touchDetected; index++){
			/* Get each touch coordinates */
			ft6x06_TS_GetXY(hts, &(Raw_x[index]), &(Raw_y[index]));

			if(hts->tsOrientation & TS_SWAP_XY){
				tmp = Raw_x[index];
				Raw_x[index] = Raw_y[index];
				Raw_y[index] = tmp;
			}
			if(hts->tsOrientation & TS_SWAP_X){
				Raw_x[index] = hts->max_width - 1 - Raw_x[index];
			}
			if(hts->tsOrientation & TS_SWAP_Y){
				Raw_y[index] = hts->max_height - 1 - Raw_y[index];
			}

			xDiff = Raw_x[index] > _x[index]? (Raw_x[index] - _x[index]): (_x[index] - Raw_x[index]);
			yDiff = Raw_y[index] > _y[index]? (Raw_y[index] - _y[index]): (_y[index] - Raw_y[index]);

			if((xDiff + yDiff) > 5){
				_x[index] = Raw_x[index];
				_y[index] = Raw_y[index];
			}

			TS_State->touchX[index] = _x[index];
			TS_State->touchY[index] = _y[index];

#if (TS_MULTI_TOUCH_SUPPORTED == 1)

			/* Get touch info related to the current touch */
			ercd = ft6x06_TS_GetTouchInfo(hts, index, &weight, &area, &event);

			/* Update TS_State structure */
			TS_State->touchWeight[index] = weight;
			TS_State->touchArea[index]   = area;

			/* Remap touch event */
			switch(event){
			case FT6206_TOUCH_EVT_FLAG_PRESS_DOWN  :
				TS_State->touchEventId[index] = TOUCH_EVENT_PRESS_DOWN;
				break;
			case FT6206_TOUCH_EVT_FLAG_LIFT_UP :
				TS_State->touchEventId[index] = TOUCH_EVENT_LIFT_UP;
				break;
			case FT6206_TOUCH_EVT_FLAG_CONTACT :
				TS_State->touchEventId[index] = TOUCH_EVENT_CONTACT;
				break;
			case FT6206_TOUCH_EVT_FLAG_NO_EVENT :
				TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
				break;
			 default :
				ercd = E_SYS;
				break;
			} /* of switch(event) */
			if(ercd < 0)
				return ercd;

#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

		} /* of for(index=0; index < TS_State->touchDetected; index++) */

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
		/* Get gesture Id */
		ercd = touchscreen_get_gestureid(hts, TS_State);
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

	} /* end of if(TS_State->touchDetected != 0) */
	return ercd;
}

/**
  * @brief  Configure the FT6206 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  DeviceAddr: Device address on communication Bus (Slave I2C address of FT6206).
  * @retval None
  */
ER
touchscreen_enable_it(TouchScreen_Handle_t *hts)
{
	ER ercd;
	uint8_t regValue = 0;
	regValue = (FT6206_G_MODE_INTERRUPT_TRIGGER & (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT)) << FT6206_G_MODE_INTERRUPT_SHIFT;

	/* Set interrupt trigger mode in FT6206_GMODE_REG */
	ercd = i2c_memwrite(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_GMODE_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&regValue, 1, 1000);
	return ercd;
}

/**
  * @brief  Configure the FT6206 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  DeviceAddr: Device address on communication Bus (Slave I2C address of FT6206).
  * @retval None
  */
ER touchscreen_disable_it(TouchScreen_Handle_t *hts)
{
	ER  ercd;
	uint8_t regValue = 0;
	regValue = (FT6206_G_MODE_INTERRUPT_POLLING | (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT)) << FT6206_G_MODE_INTERRUPT_SHIFT;

	/* Set interrupt polling mode in FT6206_GMODE_REG */
	ercd = i2c_memwrite(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_GMODE_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&regValue, 1, 1000);
	return ercd;
}

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
/**
  * @brief  Update gesture Id following a touch detected.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
ER
touchscreen_get_gestureid(TouchScreen_Handle_t *hts, TS_StateTypeDef *TS_State)
{
	uint32_t gestureId = 0;
	volatile uint8_t ucReadData = 0;
	ER ercd = E_OK;

	/* Get gesture Id */
	ercd = i2c_memread(hts->hi2c, hts->I2C_Address, (uint16_t)FT6206_GEST_ID_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&ucReadData, 1, 1000);
	if(ercd != E_OK)
		return ercd;

	gestureId = hts->I2C_Address;

	/* Remap gesture Id to a TS_GestureIdTypeDef value */
	switch(gestureId){
	case FT6206_GEST_ID_NO_GESTURE :
		TS_State->gestureId = GEST_ID_NO_GESTURE;
		break;
	case FT6206_GEST_ID_MOVE_UP :
		TS_State->gestureId = GEST_ID_MOVE_UP;
		break;
	case FT6206_GEST_ID_MOVE_RIGHT :
		TS_State->gestureId = GEST_ID_MOVE_RIGHT;
		break;
	case FT6206_GEST_ID_MOVE_DOWN :
		TS_State->gestureId = GEST_ID_MOVE_DOWN;
		break;
	case FT6206_GEST_ID_MOVE_LEFT :
		TS_State->gestureId = GEST_ID_MOVE_LEFT;
		break;
	case FT6206_GEST_ID_ZOOM_IN :
		TS_State->gestureId = GEST_ID_ZOOM_IN;
		break;
	case FT6206_GEST_ID_ZOOM_OUT :
		TS_State->gestureId = GEST_ID_ZOOM_OUT;
		break;
	default :
		ercd = E_OBJ;
		break;
	} /* of switch(gestureId) */
	return ercd;
}
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */


/** @defgroup STM32F723E-DISCOVERY_TS_Private_Functions TS Private Functions
  * @{
  */

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
/**
  * @brief  Function used to reset all touch data before a new acquisition
  *         of touch information.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if OK, TE_ERROR if problem found.
  */
ER
touchscreen_reset_data(TS_StateTypeDef *TS_State)
{
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
		return E_OK;
	}
	else
		return E_PAR;
}
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

