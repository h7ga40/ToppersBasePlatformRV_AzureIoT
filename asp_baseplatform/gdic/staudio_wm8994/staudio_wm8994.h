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

#ifndef _STAUDIO_WM8994_H_
#define _STAUDIO_WM8994_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "device.h"
#include "i2c.h"
#include "sai.h"
#ifdef TOPPERS_STM32F769_DISCOVERY
#include "dfsdm.h"
#endif

/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER                 ((uint16_t)0x0001)
#define OUTPUT_DEVICE_HEADPHONE               ((uint16_t)0x0002)
#define OUTPUT_DEVICE_BOTH                    ((uint16_t)0x0003)
#define OUTPUT_DEVICE_AUTO                    ((uint16_t)0x0004)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_1     ((uint16_t)0x0100)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_2     ((uint16_t)0x0200)
#define INPUT_DEVICE_INPUT_LINE_1             ((uint16_t)0x0300)
#define INPUT_DEVICE_INPUT_LINE_2             ((uint16_t)0x0400)
#define INPUT_DEVICE_DIGITAL_MIC1_MIC2        ((uint16_t)0x0800)

/* MP34DT01TR digital microphone on PCB top side */
#define INPUT_DEVICE_DIGITAL_MIC       ((uint16_t)0)

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0


#define VOLUME_CONVERT(Volume)        (((Volume) > 100)? 100:((uint8_t)(((Volume) * 63) / 100)))
#define VOLUME_IN_CONVERT(Volume)     (((Volume) >= 100)? 239:((uint8_t)(((Volume) * 240) / 100)))

/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
/** 
  * @brief  WM8994 ID  
  */  
#define  WM8994_ID    0x08994

/**
  * @brief Device ID Register: Reading from this register will indicate device 
  *                            family ID 8994h
  */
#define WM8994_CHIPID_ADDR                  0x00


/*------------------------------------------------------------------------------
                           Audio Codec functions 
------------------------------------------------------------------------------*/
/**
  * @brief Audio I2C Slave address
  */
#define AUDIO_I2C_ADDRESS                ((uint16_t)0x34)


/** @defgroup BSP_Audio_Out_Option BSP Audio Out Option
  * @{
  */
#define BSP_AUDIO_OUT_CIRCULARMODE      ((uint32_t)0x00000001) /* BUFFER CIRCULAR MODE */
#define BSP_AUDIO_OUT_NORMALMODE        ((uint32_t)0x00000002) /* BUFFER NORMAL MODE   */
#define BSP_AUDIO_OUT_STEREOMODE        ((uint32_t)0x00000004) /* STEREO MODE          */
#define BSP_AUDIO_OUT_MONOMODE          ((uint32_t)0x00000008) /* MONO MODE            */
/**
  * @}
  */
/** @defgroup BSP_Audio_Sample_Rate BSP Audio Sample Rate
  * @{
  */
#define BSP_AUDIO_FREQUENCY_96K         ((uint32_t)192000U)
#define BSP_AUDIO_FREQUENCY_48K         ((uint32_t)48000U)
#define BSP_AUDIO_FREQUENCY_44K         ((uint32_t)44100U)
#define BSP_AUDIO_FREQUENCY_32K         ((uint32_t)32000U)
#define BSP_AUDIO_FREQUENCY_22K         ((uint32_t)22050U)
#define BSP_AUDIO_FREQUENCY_16K         ((uint32_t)16000U)
#define BSP_AUDIO_FREQUENCY_11K         ((uint32_t)11025U)
#define BSP_AUDIO_FREQUENCY_8K          ((uint32_t)8000U)

/*------------------------------------------------------------------------------
                          USER SAI defines parameters
 -----------------------------------------------------------------------------*/
/** CODEC_AudioFrame_SLOT_TDMMode In W8994 codec the Audio frame contains 4 slots : TDM Mode
  * TDM format :
  * +------------------|------------------|--------------------|-------------------+ 
  * | CODEC_SLOT0 Left | CODEC_SLOT1 Left | CODEC_SLOT0 Right  | CODEC_SLOT1 Right |
  * +------------------------------------------------------------------------------+
  */
/* To have 2 separate audio stream in Both headphone and speaker the 4 slot must be activated */
#define CODEC_AUDIOFRAME_SLOT_0123          (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1 | SAI_SLOTACTIVE_2 | SAI_SLOTACTIVE_3)

/* To have an audio stream in headphone only SAI Slot 0 and Slot 2 must be activated */ 
#define CODEC_AUDIOFRAME_SLOT_02            SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_2 
/* To have an audio stream in speaker only SAI Slot 1 and Slot 3 must be activated */ 
#define CODEC_AUDIOFRAME_SLOT_13            SAI_SLOTACTIVE_1 | SAI_SLOTACTIVE_3


#define DMA_MAX_SZE                         0xFFFF

/*------------------------------------------------------------------------------
             CONFIGURATION: Audio Driver Configuration parameters
------------------------------------------------------------------------------*/

#define AUDIODATA_SIZE                      2   /* 16-bits audio data size */

/* Audio status definition */
#define AUDIO_OK                            E_OK

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define DEFAULT_AUDIO_IN_FREQ               BSP_AUDIO_FREQUENCY_16K
#define DEFAULT_AUDIO_IN_BIT_RESOLUTION     ((uint8_t)16)
#define DEFAULT_AUDIO_IN_CHANNEL_NBR        ((uint8_t)2) /* Mono = 1, Stereo = 2 */
#define DEFAULT_AUDIO_IN_VOLUME             ((uint16_t)100)
   
/*------------------------------------------------------------------------------
                    OPTIONAL Configuration defines parameters
------------------------------------------------------------------------------*/

/* Delay for the Codec to be correctly reset */
#define CODEC_RESET_DELAY                   ((uint8_t)5)
   

/*------------------------------------------------------------------------------
                            OUTPUT DEVICES definition
------------------------------------------------------------------------------*/
/* Alias on existing output devices to adapt to headphones output */
#define OUTPUT_DEVICE_HEADPHONE1 OUTPUT_DEVICE_HEADPHONE
#define OUTPUT_DEVICE_HEADPHONE2 OUTPUT_DEVICE_SPEAKER 
   
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)

/*
 *  STMAUDIOハンドラ定義
 *
 */
typedef struct STAUDIO_Handle_s {
	I2C_Handle_t    *hi2c;
	AUDIO_Handle_t  *hsai;
	ID              saiid;
	uint8_t         i2caddress;
	uint8_t         outputEnabled;
	uint8_t         inputEnabled;
	uint8_t         ColdStartup;
	uint16_t        InputDevice;
	uint16_t        AudioInVolume;
	void   (*audio_fullout_func)(AUDIO_Handle_t *hsai);
	void   (*audio_halfout_func)(AUDIO_Handle_t *hsai);
	void   (*audio_fullin_func)(void *handle);
	void   (*audio_halfin_func)(void *handle);
	void   (*audio_error_func)(AUDIO_Handle_t *hsai);
} STAUDIO_Handle_t;


extern ER staudio_out_init(STAUDIO_Handle_t *haudio, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq, uint16_t framesize);
extern void staudio_out_deinit(STAUDIO_Handle_t *haudio);
extern ER staudio_out_play(STAUDIO_Handle_t *haudio, uint16_t* pBuffer, uint32_t Size);
extern ER staudio_out_pause(STAUDIO_Handle_t *haudio);
extern ER staudio_out_resume(STAUDIO_Handle_t *haudio);
extern ER staudio_out_stop(STAUDIO_Handle_t *haudio, uint32_t Option);
extern ER staudio_out_setvolume(STAUDIO_Handle_t *haudio, uint8_t Volume);
extern void staudio_out_setfrequency(STAUDIO_Handle_t *haudio, uint32_t AudioFreq);
extern void staudio_out_setframeslot(STAUDIO_Handle_t *haudio, uint32_t AudioFrameSlot);
extern ER staudio_out_setmute(STAUDIO_Handle_t *haudio, uint32_t Cmd);
extern ER staudio_out_setoutputmode(STAUDIO_Handle_t *haudio, uint8_t Output);
extern void staudio_out_changebuffer(STAUDIO_Handle_t *haudio, uint16_t *pData, uint16_t Size);


extern ER staudio_in_init(STAUDIO_Handle_t *haudio, uint16_t InputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
uint8_t BSP_AUDIO_IN_Init(uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
extern ER staudio_in_out_init(STAUDIO_Handle_t *haudio, uint16_t InputDevice, uint16_t OutputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
extern void staudio_in_deinit(STAUDIO_Handle_t *haudio);
extern ER staudio_in_record(STAUDIO_Handle_t *haudio, uint16_t* pbuf, uint32_t size);
extern ER staudio_in_stop(STAUDIO_Handle_t *haudio, uint32_t Option);
extern ER staudio_in_pause(STAUDIO_Handle_t *haudio);
extern ER staudio_in_resume(STAUDIO_Handle_t *haudio);
extern ER staudio_in_setvolume(STAUDIO_Handle_t *haudio, uint8_t Volume);

#ifdef TOPPERS_STM32F769_DISCOVERY
extern ER BSP_AUDIO_IN_AllocScratch(int32_t *pScratch, uint32_t size);
extern uint8_t BSP_AUDIO_IN_GetChannelNumber(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STAUDIO_WM8994_H_ */

