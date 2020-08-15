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

#ifndef _STTS_FT5336_H_
#define _STTS_FT5336_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include "i2c.h"

/* Set Multi-touch as supported */
#if !defined(TS_MONO_TOUCH_SUPPORTED)
#define TS_MULTI_TOUCH_SUPPORTED        1
#endif /* TS_MONO_TOUCH_SUPPORTED */


  /* I2C Slave address of touchscreen FocalTech FT5336 */
#define FT5336_I2C_SLAVE_ADDRESS        0x70

  /* Maximum border values of the touchscreen pad */
#define FT5336_MAX_WIDTH                480	/* Touchscreen pad max width   */
#define FT5336_MAX_HEIGHT               272	/* Touchscreen pad max height  */

  /* Max detectable simultaneous touches */
#define FT5336_MAX_DETECTABLE_TOUCH     0x05

  /**
   * @brief : Definitions for FT5336 I2C register addresses on 8 bit
   **/

  /* Current mode register of the FT5336 (R/W) */
#define FT5336_DEV_MODE_REG             0x00

  /* Possible values of FT5336_DEV_MODE_REG */
#define FT5336_DEV_MODE_WORKING         0x00
#define FT5336_DEV_MODE_FACTORY         0x04

#define FT5336_DEV_MODE_MASK            0x07
#define FT5336_DEV_MODE_SHIFT           0x04

  /* Gesture ID register */
#define FT5336_GEST_ID_REG              0x01

  /* Possible values of FT5336_GEST_ID_REG */
#define FT5336_GEST_ID_NO_GESTURE       0x00
#define FT5336_GEST_ID_MOVE_UP          0x10
#define FT5336_GEST_ID_MOVE_RIGHT       0x14
#define FT5336_GEST_ID_MOVE_DOWN        0x18
#define FT5336_GEST_ID_MOVE_LEFT        0x1C
#define FT5336_GEST_ID_SINGLE_CLICK     0x20
#define FT5336_GEST_ID_DOUBLE_CLICK     0x22
#define FT5336_GEST_ID_ROTATE_CLOCKWISE 0x28
#define FT5336_GEST_ID_ROTATE_C_CLOCKWISE 0x29
#define FT5336_GEST_ID_ZOOM_IN          0x40
#define FT5336_GEST_ID_ZOOM_OUT         0x49

  /* Touch Data Status register : gives number of active touch points (0..5) */
#define FT5336_TD_STAT_REG              0x02

  /* Values related to FT5336_TD_STAT_REG */
#define FT5336_TD_STAT_MASK             0x0F
#define FT5336_TD_STAT_SHIFT            0x00

  /* Values Pn_XH and Pn_YH related */
#define FT5336_TOUCH_EVT_FLAG_PRESS_DOWN 0x00
#define FT5336_TOUCH_EVT_FLAG_LIFT_UP   0x01
#define FT5336_TOUCH_EVT_FLAG_CONTACT   0x02
#define FT5336_TOUCH_EVT_FLAG_NO_EVENT  0x03

#define FT5336_TOUCH_EVT_FLAG_SHIFT     0x06
#define FT5336_TOUCH_EVT_FLAG_MASK      (3 << FT5336_TOUCH_EVT_FLAG_SHIFT)

#define FT5336_TOUCH_POS_MSB_MASK       0x0F
#define FT5336_TOUCH_POS_MSB_SHIFT      0x00

  /* Values Pn_XL and Pn_YL related */
#define FT5336_TOUCH_POS_LSB_MASK       0xFF
#define FT5336_TOUCH_POS_LSB_SHIFT      0x00

#define FT5336_P1_XH_REG                0x03
#define FT5336_P1_XL_REG                0x04
#define FT5336_P1_YH_REG                0x05
#define FT5336_P1_YL_REG                0x06

/* Touch Pressure register value (R) */
#define FT5336_P1_WEIGHT_REG            0x07

/* Values Pn_WEIGHT related  */
#define FT5336_TOUCH_WEIGHT_MASK        0xFF
#define FT5336_TOUCH_WEIGHT_SHIFT       0x00

/* Touch area register */
#define FT5336_P1_MISC_REG              0x08

/* Values related to FT5336_Pn_MISC_REG */
#define FT5336_TOUCH_AREA_MASK          (0x04 << 4)
#define FT5336_TOUCH_AREA_SHIFT         0x04

#define FT5336_P2_XH_REG                0x09
#define FT5336_P2_XL_REG                0x0A
#define FT5336_P2_YH_REG                0x0B
#define FT5336_P2_YL_REG                0x0C
#define FT5336_P2_WEIGHT_REG            0x0D
#define FT5336_P2_MISC_REG              0x0E

#define FT5336_P3_XH_REG                0x0F
#define FT5336_P3_XL_REG                0x10
#define FT5336_P3_YH_REG                0x11
#define FT5336_P3_YL_REG                0x12
#define FT5336_P3_WEIGHT_REG            0x13
#define FT5336_P3_MISC_REG              0x14

#define FT5336_P4_XH_REG                0x15
#define FT5336_P4_XL_REG                0x16
#define FT5336_P4_YH_REG                0x17
#define FT5336_P4_YL_REG                0x18
#define FT5336_P4_WEIGHT_REG            0x19
#define FT5336_P4_MISC_REG              0x1A

#define FT5336_P5_XH_REG                0x1B
#define FT5336_P5_XL_REG                0x1C
#define FT5336_P5_YH_REG                0x1D
#define FT5336_P5_YL_REG                0x1E
#define FT5336_P5_WEIGHT_REG            0x1F
#define FT5336_P5_MISC_REG              0x20

#define FT5336_P6_XH_REG                0x21
#define FT5336_P6_XL_REG                0x22
#define FT5336_P6_YH_REG                0x23
#define FT5336_P6_YL_REG                0x24
#define FT5336_P6_WEIGHT_REG            0x25
#define FT5336_P6_MISC_REG              0x26

#define FT5336_P7_XH_REG                0x27
#define FT5336_P7_XL_REG                0x28
#define FT5336_P7_YH_REG                0x29
#define FT5336_P7_YL_REG                0x2A
#define FT5336_P7_WEIGHT_REG            0x2B
#define FT5336_P7_MISC_REG              0x2C

#define FT5336_P8_XH_REG                0x2D
#define FT5336_P8_XL_REG                0x2E
#define FT5336_P8_YH_REG                0x2F
#define FT5336_P8_YL_REG                0x30
#define FT5336_P8_WEIGHT_REG            0x31
#define FT5336_P8_MISC_REG              0x32

#define FT5336_P9_XH_REG                0x33
#define FT5336_P9_XL_REG                0x34
#define FT5336_P9_YH_REG                0x35
#define FT5336_P9_YL_REG                0x36
#define FT5336_P9_WEIGHT_REG            0x37
#define FT5336_P9_MISC_REG              0x38

#define FT5336_P10_XH_REG               0x39
#define FT5336_P10_XL_REG               0x3A
#define FT5336_P10_YH_REG               0x3B
#define FT5336_P10_YL_REG               0x3C
#define FT5336_P10_WEIGHT_REG           0x3D
#define FT5336_P10_MISC_REG             0x3E

  /* Threshold for touch detection */
#define FT5336_TH_GROUP_REG             0x80

  /* Values FT5336_TH_GROUP_REG : threshold related  */
#define FT5336_THRESHOLD_MASK           0xFF
#define FT5336_THRESHOLD_SHIFT          0x00

  /* Filter function coefficients */
#define FT5336_TH_DIFF_REG              0x85

  /* Control register */
#define FT5336_CTRL_REG                 0x86

  /* Values related to FT5336_CTRL_REG */

  /* Will keep the Active mode when there is no touching */
#define FT5336_CTRL_KEEP_ACTIVE_MODE    0x00

  /* Switching from Active mode to Monitor mode automatically when there is no touching */
#define FT5336_CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE  0x01

  /* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT5336_TIMEENTERMONITOR_REG     0x87

  /* Report rate in Active mode */
#define FT5336_PERIODACTIVE_REG         0x88

  /* Report rate in Monitor mode */
#define FT5336_PERIODMONITOR_REG        0x89

  /* The value of the minimum allowed angle while Rotating gesture mode */
#define FT5336_RADIAN_VALUE_REG         0x91

  /* Maximum offset while Moving Left and Moving Right gesture */
#define FT5336_OFFSET_LEFT_RIGHT_REG    0x92

  /* Maximum offset while Moving Up and Moving Down gesture */
#define FT5336_OFFSET_UP_DOWN_REG       0x93

  /* Minimum distance while Moving Left and Moving Right gesture */
#define FT5336_DISTANCE_LEFT_RIGHT_REG  0x94

  /* Minimum distance while Moving Up and Moving Down gesture */
#define FT5336_DISTANCE_UP_DOWN_REG     0x95

  /* Maximum distance while Zoom In and Zoom Out gesture */
#define FT5336_DISTANCE_ZOOM_REG        0x96

  /* High 8-bit of LIB Version info */
#define FT5336_LIB_VER_H_REG            0xA1

  /* Low 8-bit of LIB Version info */
#define FT5336_LIB_VER_L_REG            0xA2

  /* Chip Selecting */
#define FT5336_CIPHER_REG               0xA3

  /* Interrupt mode register (used when in interrupt mode) */
#define FT5336_GMODE_REG                0xA4

#define FT5336_G_MODE_INTERRUPT_MASK    0x03
#define FT5336_G_MODE_INTERRUPT_SHIFT   0x00

  /* Possible values of FT5336_GMODE_REG */
#define FT5336_G_MODE_INTERRUPT_POLLING 0x00
#define FT5336_G_MODE_INTERRUPT_TRIGGER 0x01

  /* Current power mode the FT5336 system is in (R) */
#define FT5336_PWR_MODE_REG             0xA5

  /* FT5336 firmware version */
#define FT5336_FIRMID_REG               0xA6

  /* FT5336 Chip identification register */
#define FT5336_CHIP_ID_REG              0xA8

  /*  Possible values of FT5336_CHIP_ID_REG */
#define FT5336_ID_VALUE                 0x51

  /* Release code version */
#define FT5336_RELEASE_CODE_ID_REG      0xAF

  /* Current operating mode the FT5336 system is in (R) */
#define FT5336_STATE_REG                0xBC


/** @typedef TouchScreen_Handle_t
 *  TouchScreen Handle definition.
 */
typedef struct
{
	I2C_Handle_t    *hi2c;
	uint16_t        tsXBoundary;
	uint16_t        tsYBoundary;
	uint8_t         tsOrientation;
	uint8_t         I2C_Address;

	/* field holding the current number of simultaneous active touches */
	uint8_t         currActiveTouchNb;

	/* field holding the touch index currently managed */
	uint8_t         currActiveTouchIdx;

} TouchScreen_Handle_t;

/** @brief With FT5336 : maximum 5 touches detected simultaneously
  */
#define TS_MAX_NB_TOUCH                 ((uint32_t) FT5336_MAX_DETECTABLE_TOUCH)

#define TS_NO_IRQ_PENDING               ((uint8_t) 0)
#define TS_IRQ_PENDING                  ((uint8_t) 1)

#define TS_SWAP_NONE                    ((uint8_t) 0x01)
#define TS_SWAP_X                       ((uint8_t) 0x02)
#define TS_SWAP_Y                       ((uint8_t) 0x04)
#define TS_SWAP_XY                      ((uint8_t) 0x08)

#define TS_ORIENTATION_PORTRAIT         ((uint8_t) 0x00)
#define TS_ORIENTATION_LANDSCAPE        ((uint8_t) 0x01)
#define TS_ORIENTATION_LANDSCAPE_ROT180 ((uint8_t) 0x02)

/**
*  @brief TS_StateTypeDef
*  Define TS State structure
*/
typedef struct
{
	uint8_t  touchDetected;				/* Total number of active touches detected at last scan */
	uint16_t touchX[TS_MAX_NB_TOUCH];	/* Touch X[0], X[1] coordinates on 12 bits */
	uint16_t touchY[TS_MAX_NB_TOUCH];	/* Touch Y[0], Y[1] coordinates on 12 bits */

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
	uint8_t  touchWeight[TS_MAX_NB_TOUCH];	/* Touch_Weight[0], Touch_Weight[1] : weight property of touches */
	uint8_t  touchEventId[TS_MAX_NB_TOUCH];	/* Touch_EventId[0], Touch_EventId[1] : take value of type @ref TS_TouchEventTypeDef */
	uint8_t  touchArea[TS_MAX_NB_TOUCH];	/*!< Touch_Area[0], Touch_Area[1] : touch area of each touch */
	uint32_t gestureId; /*!< type of gesture detected : take value of type @ref TS_GestureIdTypeDef */
#endif  /* TS_MULTI_TOUCH_SUPPORTED == 1 */
} TS_StateTypeDef;


/**
 *  @brief TS_GestureIdTypeDef
 *  Define Possible managed gesture identification values returned by touch screen
 *  driver.
 */
typedef enum
{
	GEST_ID_NO_GESTURE = 0x00, /*!< Gesture not defined / recognized */
	GEST_ID_MOVE_UP    = 0x01, /*!< Gesture Move Up */
	GEST_ID_MOVE_RIGHT = 0x02, /*!< Gesture Move Right */
	GEST_ID_MOVE_DOWN  = 0x03, /*!< Gesture Move Down */
	GEST_ID_MOVE_LEFT  = 0x04, /*!< Gesture Move Left */
	GEST_ID_ZOOM_IN    = 0x05, /*!< Gesture Zoom In */
	GEST_ID_ZOOM_OUT   = 0x06, /*!< Gesture Zoom Out */
	GEST_ID_NB_MAX     = 0x07  /*!< max number of gesture id */
} TS_GestureIdTypeDef;

/**
 *  @brief TS_TouchEventTypeDef
 *  Define Possible touch events kind as returned values
 *  by touch screen IC Driver.
 */
typedef enum
{
	TOUCH_EVENT_NO_EVT        = 0x00, /*!< Touch Event : undetermined */
	TOUCH_EVENT_PRESS_DOWN    = 0x01, /*!< Touch Event Press Down */
	TOUCH_EVENT_LIFT_UP       = 0x02, /*!< Touch Event Lift Up */
	TOUCH_EVENT_CONTACT       = 0x03, /*!< Touch Event Contact */
	TOUCH_EVENT_NB_MAX        = 0x04  /*!< max number of touch events kind */
} TS_TouchEventTypeDef;

/**
 *  @brief Table for touchscreen event information display on LCD :
 *  table indexed on enum @ref TS_TouchEventTypeDef information
 */
extern char * ts_event_string_tab[TOUCH_EVENT_NB_MAX];

/**
 *  @brief Table for touchscreen gesture Id information display on LCD : table indexed
 *  on enum @ref TS_GestureIdTypeDef information
 */
extern char * ts_gesture_id_string_tab[GEST_ID_NB_MAX];

extern ER touchscreen_init(TouchScreen_Handle_t *hts, uint16_t ts_SizeX, uint16_t ts_SizeY, uint8_t orientation);
extern ER touchscreen_getstate(TouchScreen_Handle_t *hts, TS_StateTypeDef *TS_State);
extern ER touchscreen_reset_data(TS_StateTypeDef *TS_State);


#ifdef __cplusplus
}
#endif

#endif /* _STTS_FT5336_H_ */

