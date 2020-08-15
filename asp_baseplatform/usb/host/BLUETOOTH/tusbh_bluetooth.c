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
 *  USB Host Middleware BLUETOOTH CLASS部
 */

#include "tusbh_bluetooth.h"

#define BLUETOOTH_SENDDATA(d, b, l)   tusbhControlRequest((d), (b), BLUETOOTH_DATA_TYPE, 0, 0, 0, (l))
#define ORDER_MASK  (BT_INTERRUPT_ORDER | BT_CTLWRITE_ORDER | BT_BLKREAD_ORDER | BT_BLKWRITE_ORDER)

static TUSBH_ERCODE tusbhBLUETOOTHInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhBLUETOOTHeInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhBLUETOOTHProcess(TUSBH_Device_t *pdevice, uint8_t *mes);

static TUSBH_Class_t BLUETOOTH_Class = {
	NULL,
	"BLUETOOTH",
	BLUETOOTH_CLASS,
	tusbhBLUETOOTHInit,
	tusbhBLUETOOTHeInit,
	tusbhBLUETOOTHProcess,
	NULL
};

/*
 *  SERIALクラスセットアップ関数
 */
void tusbhLinkBLUETOOTH(TUSBH_Handle_t *phost)
{
	tusbhLinkClass(phost, &BLUETOOTH_Class);
}

/*
 *  BLUETOOTHクラス初期設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhBLUETOOTHInit(TUSBH_Device_t *pdevice)
{
	uint8_t interface = 0; 
	BLUETOOTH_Handle_t *hblue;
	TUSBH_Handle_t *phost = pdevice->pHost;
	uint8_t max_ep, i;

	interface = tusbhFindInterface(pdevice, pdevice->pClass->classCode, 0x01, 0x01);

	if(interface == NO_INTERFACE){
		syslog_1(LOG_ERROR, "[BLUETOOTH] Cannot Find the interface for %s class.", pdevice->pClass->Name);
		return TUSBH_E_ERR;
	}
	tusbhSelectInterface(pdevice, interface);

	hblue = (BLUETOOTH_Handle_t *)tusbmalloc(sizeof(BLUETOOTH_Handle_t));
	pdevice->pData = hblue;
	memset(hblue, 0, sizeof(BLUETOOTH_Handle_t));
	max_ep = pdevice->ItfDesc[interface].bNumEndpoints;
	if(max_ep < 3 || (pdevice->EpDesc[interface][0].bEndpointAddress & 0x80) == 0)
		return TUSBH_E_ERR;

	hblue->CommInEp     = pdevice->EpDesc[interface][0].bEndpointAddress;
    hblue->CommInEpSize = pdevice->EpDesc[interface][0].wMaxPacketSize;
	hblue->CommInPipe   = tusbhAllocPipe(phost, hblue->CommInEp);
	hblue->poll         = pdevice->EpDesc[interface][0].bInterval;

	if(hblue->poll < BT_MIN_POLL)
		hblue->poll = BT_MIN_POLL;

	for(i = 1 ; i < max_ep; i++){
		if(pdevice->EpDesc[interface][i].bEndpointAddress & 0x80){
			hblue->InEp     = (pdevice->EpDesc[interface][i].bEndpointAddress);
			hblue->InEpSize = pdevice->EpDesc[interface][i].wMaxPacketSize;
			hblue->InPipe = tusbhAllocPipe(phost, hblue->InEp);
		}
		else{
			hblue->OutEp = (pdevice->EpDesc[interface][i].bEndpointAddress);
			hblue->OutEpSize = pdevice->EpDesc[interface][i].wMaxPacketSize;
			hblue->OutPipe = tusbhAllocPipe(phost, hblue->OutEp);
		}
	}

	if(hblue->OutPipe == 0 || hblue->InPipe == 0)
		return TUSBH_E_ERR;

	hblue->state = BLUETOOTH_PROCESS_INIT;
	hblue->timer = 0;
	hblue->pDev = pdevice;

	/*
	 *  NSCチャネルオープン
	 */
	tusbhOpenPipe(pdevice, hblue->CommInPipe, hblue->CommInEp, USB_EP_TYPE_INTR, 64);
	tusbhHDSetToggle(phost, hblue->CommInPipe, 0);
	tusbhOpenPipe(pdevice, hblue->OutPipe, hblue->OutEp, USB_EP_TYPE_BULK, hblue->OutEpSize);
	tusbhHDSetToggle(phost, hblue->OutPipe, 0);
	tusbhOpenPipe(pdevice, hblue->InPipe, hblue->InEp, USB_EP_TYPE_BULK, hblue->InEpSize);
	tusbhHDSetToggle(phost, hblue->InPipe, 0);

	/* タイマースタート */
	pdevice->timeid = pdevice->timecount = hblue->poll;
	return TUSBH_E_OK;
}

/*
 *  BLUETOOTHクラス終了設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhBLUETOOTHeInit(TUSBH_Device_t *pdevice)
{
	BLUETOOTH_Handle_t *hblue;
	TUSBH_Handle_t *phost = pdevice->pHost;

	hblue = (BLUETOOTH_Handle_t *)pdevice->pData;
	hblue->ifstate  &= ~ORDER_MASK;
	if(hblue->CommInPipe != 0){
		tusbhClosePipe(pdevice, hblue->CommInPipe);
		tusbFreePipe(phost, hblue->CommInPipe);
		hblue->CommInPipe = 0;
	}

	if(hblue->OutPipe != 0){
		tusbhClosePipe(pdevice, hblue->OutPipe);
		tusbFreePipe(phost, hblue->OutPipe);
		hblue->OutPipe = 0;
	}

	if(hblue->InPipe != 0){
		tusbhClosePipe(pdevice, hblue->InPipe);
		tusbFreePipe(phost, hblue->InPipe);
		hblue->InPipe = 0;
	}

	if(pdevice->pData != NULL){
		tusbfree(pdevice->pData);
		pdevice->pData = NULL;
	}
	return TUSBH_E_OK;
}

/*
 *  BLUETOOTHクラスプロセス実行
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 mes:      通信メッセージへのポインタ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhBLUETOOTHProcess(TUSBH_Device_t *pdevice, uint8_t *mes)
{
	BLUETOOTH_Handle_t *hblue  = (BLUETOOTH_Handle_t *)pdevice->pData;
	TUSBH_Handle_t *phost = pdevice->pHost;
	void (*func)(TUSBH_Device_t *, uint8_t, int16_t, uint8_t *) = pdevice->pClass->subfunc;
	TUSBH_URBState_t URB_Status = mes[3];
	TUSBH_ERCODE ercd = TUSBH_E_BUSY;
	uint8_t mesreq = 0;
	bool_t  timereq = false;

	if(mes[0] == TUSBH_TIME_EVENT){
		hblue->timer += mes[3];
		pdevice->timeid = pdevice->timecount = hblue->poll;
		timereq = true;
	}
	else if(mes[0] == TUSBH_CLASS_EVENT && mes[2] != 0)
		hblue->state = mes[2];
	switch (hblue->state){
	case BLUETOOTH_PROCESS_INIT:
		if(phost->usrcallback != NULL)
			phost->usrcallback(phost, pdevice, HOST_USER_CLASS_ACTIVE);
		hblue->state = BLUETOOTH_PROCESS_IDLE;
	case BLUETOOTH_PROCESS_IDLE:
		if(timereq && (hblue->ifstate & (BT_INTERRUPT_ORDER | BT_INTERRUPT_REQUEST)) == BT_INTERRUPT_ORDER){
			tusbhInterruptRead(pdevice, &hblue->CommInBuf[0], hblue->CommInEpSize, hblue->CommInPipe);
			hblue->ifstate |= BT_INTERRUPT_REQUEST;
		}
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(mes[2] == hblue->CommInPipe){	/* interrupt */
			if(URB_Status == TUSBH_URB_DONE){
				if(func != NULL)
					func(pdevice, HCI_EVENT_PACKET, hblue->CommInEpSize, &hblue->CommInBuf[0]);
				hblue->ifstate &= ~BT_INTERRUPT_REQUEST;
			}
			else if(URB_Status == TUSBH_URB_STALL)
				hblue->state = BLUETOOTH_PROCESS_ERROR;
			else
				hblue->ifstate &= ~BT_INTERRUPT_REQUEST;
		}
		else if(mes[2] == hblue->InPipe){	/* blk read */
			if(URB_Status == TUSBH_URB_DONE){
				if(func != NULL)
					func(pdevice, HCI_ACL_DATA_PACKET, tusbhHDTrasLength(phost, hblue->InPipe), hblue->pBRecvBuff);
			}
			else if(URB_Status == TUSBH_URB_NOTREADY){
				hblue->ifstate |= BT_BLKREAD_ORDER;
				mesreq = BLUETOOTH_PROCESS_RECEIVE;
			}
			else if(URB_Status == TUSBH_URB_STALL)
				hblue->state = BLUETOOTH_PROCESS_ERROR;
			else{
				if(func != NULL)
					func(pdevice, HCI_ACL_DATA_PACKET, -1, hblue->pBRecvBuff);
			}
			hblue->ifstate &= ~BT_BLKREAD_REQUEST;
		}
		else if(mes[2] == hblue->OutPipe){	/* blk write */
			if(URB_Status == TUSBH_URB_DONE){
				hblue->BTrnLen += hblue->BTrnCLen;
				if(hblue->BTrnLen >= hblue->BTrnSize){
					if(func != NULL)
						func(pdevice, HCI_SCO_DATA_PACKET, hblue->BTrnSize, NULL);
					if((hblue->ifstate & (BT_CTLWRITE_ORDER | BT_CTLWRITE_REQUEST)) == BT_CTLWRITE_ORDER)
						mesreq = BLUETOOTH_PROCESS_SEND;
				}
				else{
					hblue->ifstate |= BT_BLKWRITE_ORDER;
					mesreq = BLUETOOTH_PROCESS_SEND;
				}
			}
			else if(URB_Status == TUSBH_URB_NOTREADY){
				hblue->ifstate |= BT_BLKWRITE_ORDER;
				mesreq = BLUETOOTH_PROCESS_SEND;
			}
			else if(URB_Status == TUSBH_URB_STALL)
				hblue->state = BLUETOOTH_PROCESS_ERROR;
			else{
				if(func != NULL)
					func(pdevice, HCI_SCO_DATA_PACKET, 0, NULL);
				if((hblue->ifstate & (BT_CTLWRITE_ORDER | BT_CTLWRITE_REQUEST)) == BT_CTLWRITE_ORDER)
					mesreq = BLUETOOTH_PROCESS_SEND;
			}
			hblue->ifstate &= ~BT_BLKWRITE_REQUEST;
		}
		else{								/* control write */
			TUSBH_ERCODE status = tusbhControlWait(pdevice, mes);
			if(status == TUSBH_E_OK){
				hblue->ifstate &= ~BT_CTLWRITE_REQUEST;
				if(func != NULL)
					func(pdevice, HCI_COMMAND_DATA_PACKET, hblue->CTrnSize, hblue->pCTrnBuff);
				if((hblue->ifstate & BT_INTERRUPT_STOP) == 0)
					hblue->ifstate  |= BT_INTERRUPT_ORDER;
				if((hblue->ifstate & (BT_BLKWRITE_ORDER | BT_BLKWRITE_REQUEST)) == BT_BLKWRITE_ORDER)
					mesreq = BLUETOOTH_PROCESS_SEND;
			}
			else if(status != TUSBH_E_BUSY && status != TUSBH_E_BUSY_URB){
				hblue->state = BLUETOOTH_PROCESS_ERROR;
				hblue->ifstate  &= ~BT_CTLWRITE_REQUEST;
			}
		}
		tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, mesreq, 0);
		break;
	case BLUETOOTH_PROCESS_SEND:
		if((hblue->ifstate & (BT_CTLWRITE_ORDER | BT_CTLWRITE_REQUEST | BT_BLKWRITE_REQUEST)) == BT_CTLWRITE_ORDER){
			BLUETOOTH_SENDDATA(pdevice, hblue->pCTrnBuff, hblue->CTrnSize);
			hblue->ifstate |= BT_CTLWRITE_REQUEST;
			hblue->ifstate &= ~BT_CTLWRITE_ORDER;
		}
		if((hblue->ifstate & (BT_BLKWRITE_ORDER | BT_BLKWRITE_REQUEST | BT_CTLWRITE_REQUEST)) == BT_BLKWRITE_ORDER){
			hblue->BTrnCLen = hblue->BTrnSize - hblue->BTrnLen;
			if(hblue->BTrnCLen > hblue->OutEpSize)
				hblue->BTrnCLen = hblue->OutEpSize;
			tusbhBulkWrite(pdevice, &hblue->pBTrnBuff[hblue->BTrnLen], hblue->BTrnCLen, hblue->OutPipe);
			hblue->ifstate |= BT_BLKWRITE_REQUEST;
			hblue->ifstate &= ~BT_BLKWRITE_ORDER;
		}
		hblue->state = BLUETOOTH_PROCESS_IDLE;
		break;
	case BLUETOOTH_PROCESS_RECEIVE:
		if((hblue->ifstate & (BT_BLKREAD_ORDER | BT_BLKREAD_REQUEST)) == BT_BLKREAD_ORDER){
			tusbhBulkRead(pdevice, hblue->pBRecvBuff, hblue->BRecvSize, hblue->InPipe);
			hblue->ifstate &= ~BT_BLKREAD_ORDER;
			hblue->ifstate |= BT_BLKREAD_REQUEST;
		}
		hblue->state = BLUETOOTH_PROCESS_IDLE;
		break;
	case BLUETOOTH_PROCESS_ERROR:
		break;
	default:
		break;
	}
	return ercd;
}


/*
 *  BLUETOOTH TRANSFAR READY
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 unit:    UNIT#
 *  parameter3 mode:    確認モード
 *  return     bool
 */
bool_t
tusbhBluetoothTransfarReady(TUSBH_Handle_t *phost, uint8_t unit, uint8_t mode)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, BLUETOOTH_Class.classCode, &unit);
	BLUETOOTH_Handle_t *hblue;
	bool_t result = false;

	if(pdevice != NULL){
		hblue = pdevice->pData;
		if(pdevice->dstate != DEVICE_CLASS)
			return false;
		if(hblue->state == BLUETOOTH_PROCESS_ERROR)
			return false;
	}
	else
		return false;
	switch(mode){
	case HCI_COMMAND_DATA_PACKET:
		result = (hblue->ifstate & (BT_CTLWRITE_ORDER | BT_CTLWRITE_REQUEST)) == 0;
		break;
	case HCI_ACL_DATA_PACKET:
		result = (hblue->ifstate & (BT_BLKREAD_ORDER | BT_BLKREAD_REQUEST)) == 0;
		break;
	case HCI_SCO_DATA_PACKET:
		result = (hblue->ifstate & (BT_BLKWRITE_ORDER | BT_BLKWRITE_REQUEST)) == 0;
		break;
	case HCI_EVENT_PACKET:
		result = (hblue->ifstate & BT_INTERRUPT_REQUEST) == 0;
		break;
	default:
		break;
	}
	return result;
}

/*
 *  BLUETOOTH INTERRUPT POLL TIMING
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 unit:    UNIT#
 *  parameter3 mode:    確認モード
 *  return     bool
 */
TUSBH_ERCODE
tusbhBluetoothInterruptPollTiming(TUSBH_Handle_t *phost, uint8_t unit, uint32_t timing)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, BLUETOOTH_Class.classCode, &unit);
	BLUETOOTH_Handle_t *hblue;

	if(pdevice != NULL){
		hblue = pdevice->pData;
		if(pdevice->dstate != DEVICE_CLASS)
			return TUSBH_E_OBJ;
		if(hblue->state == BLUETOOTH_PROCESS_ERROR)
			return TUSBH_E_ERR;
	}
	else
		return TUSBH_E_OBJ;
	if(timing == 0){
		hblue->ifstate |= BT_INTERRUPT_STOP;
		hblue->ifstate &= ~BT_INTERRUPT_ORDER;
	}
	else{
		if(timing < BT_MIN_POLL)
			hblue->poll = BT_MIN_POLL;
		else
			hblue->poll = timing;
		hblue->ifstate &= ~BT_INTERRUPT_STOP;
		hblue->ifstate |= BT_INTERRUPT_ORDER;
	}
	return TUSBH_E_OK;
}

/*
 *  BLUETOOTH DATA SEND
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 unit:    UNIT#
 *  parameter3 mode:    パケットタイプ
 *  parameter4 pbuf:    送信データ領域へのポインタ(cache aline)
 *  parameter5 length:  送信データ長
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhBluetoothSend(TUSBH_Handle_t *phost, uint8_t unit, uint8_t mode, uint8_t *pbuf, uint16_t length)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, BLUETOOTH_Class.classCode, &unit);
	BLUETOOTH_Handle_t *hblue;

	if(pdevice != NULL){
		hblue = pdevice->pData;
		if(pdevice->dstate != DEVICE_CLASS)
			return TUSBH_E_OBJ;
		if(hblue->state == BLUETOOTH_PROCESS_ERROR)
			return TUSBH_E_ERR;
	}
	else
		return TUSBH_E_OBJ;

    /*
	 *  送信データ設定
	 */
	if(mode == HCI_COMMAND_DATA_PACKET){
		if(length > BT_CONTROL_SIZE)
			return TUSBH_E_PAR;
		if((hblue->ifstate & BT_CTLWRITE_ORDER) != 0)
			return TUSBH_E_OBJ;
		hblue->CTrnSize  = length;
		hblue->pCTrnBuff = pbuf;
		hblue->ifstate |= BT_CTLWRITE_ORDER;
	}
	else if(mode == HCI_SCO_DATA_PACKET){
		if((hblue->ifstate & BT_BLKWRITE_ORDER) != 0)
			return TUSBH_E_OBJ;
		hblue->BTrnSize   = length;
		hblue->BTrnLen    = 0;
		hblue->BTrnCLen   = 0;
		hblue->pBTrnBuff  = pbuf;
		hblue->ifstate |= BT_BLKWRITE_ORDER;
	}
	else
		return TUSBH_E_PAR;
	tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, BLUETOOTH_PROCESS_SEND, 0);
	return TUSBH_E_OK;
}

/*
 *  BLUETOOTH RECEIVE BULK IN
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 unit:    UNIT#
 *  parameter3 pbuf:    受信データ領域へのポインタ(cache aline)
 *  parameter4 length:  受信データ長
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhBluetoothReceiveBulk(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *pbuf, uint16_t length)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, BLUETOOTH_Class.classCode, &unit);
	BLUETOOTH_Handle_t *hblue;

	if(pdevice != NULL){
		hblue = pdevice->pData;
		if(pdevice->dstate != DEVICE_CLASS)
			return TUSBH_E_OBJ;
	}
	else
		return TUSBH_E_OBJ;
	if(length < hblue->InEpSize)
		return TUSBH_E_PAR;
	hblue->BRecvSize  = length;
	hblue->pBRecvBuff = pbuf;
	hblue->ifstate |= BT_BLKREAD_ORDER;
	tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, BLUETOOTH_PROCESS_RECEIVE, 0);
	return TUSBH_E_OK;
}

/*
 *  BLUETOOTHコールバック関数
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 func:    関数ポインタ
 */
void
tusbhSetBluetoothCallBack(TUSBH_Handle_t *phost, void (*func)(TUSBH_Device_t *p, uint8_t, int16_t, uint8_t *))
{
	BLUETOOTH_Class.subfunc = func;
}


