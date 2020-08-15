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
/*
 *  USB Host Middleware BLUETOOTH部定義
 */

#ifndef _HUSBH_BLUETOOTH_H_
#define _HUSBH_BLUETOOTH_H_

#include "tusbh_base.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define BLUETOOTH_DATA_TYPE             (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_DEVICE)


#define HCI_COMMAND_DATA_PACKET         0x01
#define HCI_ACL_DATA_PACKET             0x02
#define HCI_SCO_DATA_PACKET             0x03
#define HCI_EVENT_PACKET                0x04

/*
 *  BLUETOOTHクラスの状態定義
 */
#define BT_INTERRUPT_REQUEST            (1<<0)
#define BT_CTLWRITE_REQUEST             (1<<1)
#define BT_BLKREAD_REQUEST              (1<<2)
#define BT_BLKWRITE_REQUEST             (1<<3)

#define BT_INTERRUPT_ORDER              (1<<8)
#define BT_CTLWRITE_ORDER               (1<<9)
#define BT_BLKREAD_ORDER                (1<<10)
#define BT_BLKWRITE_ORDER               (1<<11)
#define BT_INTERRUPT_STOP               (1<<12)

#ifndef BT_MIN_POLL
#define BT_MIN_POLL                     20
#endif

#define BT_CONTROL_SIZE                 64

/*
 *  BLUETOOTHプロセスの状態定義
 */
enum
{
	BLUETOOTH_PROCESS_INIT = 0,
	BLUETOOTH_PROCESS_IDLE,
	BLUETOOTH_PROCESS_SEND,
	BLUETOOTH_PROCESS_RECEIVE,
	BLUETOOTH_PROCESS_ERROR
};

/*
 *  BLUTOOTHハンドラ定義
 */
typedef struct
{
	TUSBH_Device_t            *pDev;

	uint8_t                   state;
	uint8_t                   poll;
	uint16_t                  ifstate;

	uint8_t                   CommInPipe;
	uint8_t                   CommInEp;
	uint16_t                  CommInEpSize;
	uint8_t                   InPipe;
	uint8_t                   OutPipe;
	uint8_t                   OutEp;
	uint8_t                   InEp;
	uint16_t                  OutEpSize;
	uint16_t                  InEpSize;
	uint16_t                  CTrnSize;
	uint16_t                  BTrnSize;
	uint16_t                  BTrnLen;
	uint16_t                  BTrnCLen;
	uint16_t                  BRecvSize;
	uint16_t                  dummy;

	uint8_t                   CommInBuf[BT_CONTROL_SIZE];
	uint8_t                   *pBRecvBuff;
	uint8_t                   *pBTrnBuff;
	uint8_t                   *pCTrnBuff;
	uint32_t                  timer;
} BLUETOOTH_Handle_t;


void         tusbhLinkBLUETOOTH(TUSBH_Handle_t *phost);
bool_t       tusbhBluetoothTransfarReady(TUSBH_Handle_t *phost, uint8_t unit, uint8_t mode);
TUSBH_ERCODE tusbhBluetoothInterruptPollTiming(TUSBH_Handle_t *phost, uint8_t unit, uint32_t timing);
TUSBH_ERCODE tusbhBluetoothSend(TUSBH_Handle_t *phost, uint8_t unit, uint8_t mode, uint8_t *pbuf, uint16_t length);
TUSBH_ERCODE tusbhBluetoothReceiveBulk(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *pbuf, uint16_t length);
void tusbhSetBluetoothCallBack(TUSBH_Handle_t *phost, void (*func)(TUSBH_Device_t *p, uint8_t, int16_t, uint8_t *));


#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_BLUETOOTH_H_ */

