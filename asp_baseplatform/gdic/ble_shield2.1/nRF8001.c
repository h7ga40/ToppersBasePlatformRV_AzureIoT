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
The MIT License (MIT)

Copyright (c) 2014 Sandeep Mistry

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <utility/lib_aci.h>
#include <utility/aci_setup.h>
#include "BLECentral.h"
#include "BLECharacteristic.h"
#include "BLEPeripheral.h"


#define ADVERTISING_INTERVAL 0x050

struct setupMsgData {
	uint8_t length;
	uint8_t cmd;
	uint8_t type;
	uint8_t offset;
	uint8_t data[28];
};

/*
 *  nRF8001セットアップデータ
 */
static hal_aci_data_t baseSetupMsgs[NB_BASE_SETUP_MESSAGES] PROGMEM = {
	{0x00,
		{0x07,0x06,0x00,0x00,0x03,0x02,0x41,0xfe,},
	},
	{0x00,
		{0x1f,0x06,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x06,0x00,0x06,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,},
	},
	{0x00,
		{0x1f,0x06,0x10,0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x90,0x00,0xff,},
	},
	{0x00,
		{0x1f,0x06,0x10,0x38,0xff,0xff,0x02,0x58,0x0a,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00,},
	},
	{0x00,
		{0x05,0x06,0x10,0x54,0x00,0x02,},
	},
	{0x00,
		{0x19,0x06,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,},
	},
	{0x00,
		{0x19,0x06,0x70,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,},
	},
};

static struct pipeInfo _pinfovalue[MAX_ATTRIBUTE];

/*
 *  CRC16計算
 */
static uint16_t
crc_16_ccitt(uint16_t crc, uint8_t * data_in, uint16_t data_len)
{
	uint16_t i;

	for(i = 0 ; i < data_len ; i++){
		crc  = (unsigned char)(crc >> 8) | (crc << 8);
		crc ^= data_in[i];
		crc ^= (unsigned char)(crc & 0xff) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xff) << 4) << 1;
	}
	return crc;
}

/*
 *  nRF8001開始
 */
void
nRF8001_begin(nRF8001* pnRF8001, const uint8_t *advertisementData, uint8_t advertisementDataLength, const uint8_t *scanData,
                      uint8_t scanDataLength, BLEAttribute** attributes, uint8_t numAttributes)
{
	hal_aci_data_t setupMsg;
	struct setupMsgData *setupMsgData;
	uint8_t   numPipedCharacteristics = 0;
	uint8_t   pipeSetupMsgOffet  = 0;
	uint8_t   gattSetupMsgOffset = 0;
	uint16_t  handle             = 1;
	uint8_t   pipe               = 1;
	uint8_t   numPiped           = 0;
	ER        ercd;
	int       i;

	for(i = 0 ; i < numAttributes ; i++){
		BLEAttribute* attribute = attributes[i];

		if(attribute->_type == BLETypeCharacteristic){
			BLECharacteristic *pcharc = (BLECharacteristic *)attribute;

			if(pcharc->_properties){
				numPipedCharacteristics++;
			}
		}
	}

	pnRF8001->_pipeInfo = _pinfovalue;

#if DEBUG == 0
	lib_aci_init(&pnRF8001->_aciState, false);
#else
	lib_aci_init(&pnRF8001->_aciState, true);
#endif

	nRF8001_waitForSetupMode(pnRF8001);

	setupMsgData = (struct setupMsgData*)setupMsg.buffer;
	setupMsg.status_byte = 0;

	for (i = 0; i < NB_BASE_SETUP_MESSAGES; i++) {
		int setupMsgSize = pgm_read_byte_near(&baseSetupMsgs[i].buffer[0]) + 2;

		memcpy(&setupMsg, &baseSetupMsgs[i], setupMsgSize);
		if(i == 1){
			setupMsgData->data[6] = numPipedCharacteristics;
			setupMsgData->data[8] = numPipedCharacteristics;
		}
		else if(i == 2 && advertisementData && advertisementDataLength){
			setupMsgData->data[22] = 0x40;
		}
		else if(i == 3 && scanData && scanDataLength){
			setupMsgData->data[12] = 0x40;
		}
		else if(i == 5 && advertisementData && advertisementDataLength){
			memcpy(setupMsgData->data, advertisementData, advertisementDataLength);
		}
		else if(i == 6 && scanData && scanDataLength){
			memcpy(setupMsgData->data, scanData, scanDataLength);
		}

		ercd = nRF8001_sendSetupMessage(pnRF8001, &setupMsg);
		if(ercd != E_OK)
			i--;
	}

	// GATT
	for(i = 0 ; i < numAttributes ; i++){
		BLEAttribute* attribute = attributes[i];
		BLEUuid* puuid = &attribute->_uuid;

		if(attribute->_type == BLETypeService){
			setupMsgData->length  = 12 + puuid->_length;
			setupMsgData->cmd     = ACI_CMD_SETUP;
			setupMsgData->type    = 0x20;
			setupMsgData->offset  = gattSetupMsgOffset;

			setupMsgData->data[0] = 0x04;
			setupMsgData->data[1] = 0x04;
			setupMsgData->data[2] = puuid->_length;
			setupMsgData->data[3] = puuid->_length;
			setupMsgData->data[4] = (handle >> 8) & 0xff;
			setupMsgData->data[5] = handle & 0xff;
			handle++;

			setupMsgData->data[6] = (attribute->_type >> 8) & 0xff;
			setupMsgData->data[7] = attribute->_type & 0xff;
			setupMsgData->data[8] = ACI_STORE_LOCAL;
			memcpy(&setupMsgData->data[9], puuid->_data, puuid->_length);
			gattSetupMsgOffset += 9 + puuid->_length;

			nRF8001_sendSetupMessage(pnRF8001, &setupMsg);
		}
		else if(attribute->_type == BLETypeCharacteristic){
			BLECharacteristic *pcharc = (BLECharacteristic *)attribute;
			struct pipeInfo* pipeInfo = &pnRF8001->_pipeInfo[numPiped];

			memset(pipeInfo, 0, sizeof(struct pipeInfo));
			pipeInfo->pcharc = pcharc;
			if(pcharc->_properties){
				numPiped++;
				pipeInfo->startPipe = pipe;
				if(pcharc->_properties & BLENotify){
					pipeInfo->txPipe = pipe;
					pipe++;
				}

				if(pcharc->_properties & BLEIndicate){
					pipeInfo->txAckPipe = pipe;
					pipe++;
				}
				if(pcharc->_properties & BLEWriteWithoutResponse){
					pipeInfo->rxPipe = pipe;
					pipe++;
				}
				if(pcharc->_properties & BLEWrite){
					pipeInfo->rxAckPipe = pipe;
					pipe++;
				}
				if(pcharc->_properties & BLERead){
					pipeInfo->setPipe = pipe;
					pipe++;
				}
			}

			setupMsgData->length   = 15 + puuid->_length;
			setupMsgData->cmd      = ACI_CMD_SETUP;
			setupMsgData->type     = 0x20;
			setupMsgData->offset   = gattSetupMsgOffset;

			setupMsgData->data[0]  = 0x04;
			setupMsgData->data[1]  = 0x04;
			setupMsgData->data[2]  = 3 + puuid->_length;
			setupMsgData->data[3]  = 3 + puuid->_length;

			setupMsgData->data[4]  = (handle >> 8) & 0xff;
			setupMsgData->data[5]  = handle & 0xff;
			handle++;

			setupMsgData->data[6]  = (pcharc->_attr._type >> 8) & 0xff;
			setupMsgData->data[7]  = pcharc->_attr._type & 0xff;
			setupMsgData->data[8]  = ACI_STORE_LOCAL;
			setupMsgData->data[9]  = pcharc->_properties;
			setupMsgData->data[10] = handle & 0xff;
			setupMsgData->data[11] = (handle >> 8) & 0xff;
			pipeInfo->valueHandle = handle;
			handle++;

			memcpy(&setupMsgData->data[12], puuid->_data, puuid->_length);

			gattSetupMsgOffset += 12 + puuid->_length;

			nRF8001_sendSetupMessage(pnRF8001, &setupMsg);

			setupMsgData->length   = 12 + pcharc->_valueSize;
			setupMsgData->cmd      = ACI_CMD_SETUP;
			setupMsgData->type     = 0x20;
			setupMsgData->offset   = gattSetupMsgOffset;

			setupMsgData->data[0]  = 0x04;
			setupMsgData->data[1]  = 0x00;
			if(pcharc->_fixedLength)
				setupMsgData->data[0] |= 0x02;

			if(pcharc->_properties & BLERead)
				setupMsgData->data[1] |= 0x04;
			if(pcharc->_properties & (BLEWrite | BLEWriteWithoutResponse)){
				setupMsgData->data[0] |= 0x40;
				setupMsgData->data[1] |= 0x10;
			}
			if(pcharc->_properties & BLENotify)
				setupMsgData->data[0] |= 0x10;
			if(pcharc->_properties & BLEIndicate)
				setupMsgData->data[0] |= 0x20;
			setupMsgData->data[2]  = pcharc->_valueSize;
			if(pcharc->_fixedLength)
				setupMsgData->data[2]++;
			setupMsgData->data[3]  = pcharc->_valueLength;
			setupMsgData->data[4]  = (pipeInfo->valueHandle >> 8) & 0xff;
			setupMsgData->data[5]  = pipeInfo->valueHandle & 0xff;
			setupMsgData->data[6]  = 0x00;
			setupMsgData->data[7]  = 0x00;
			setupMsgData->data[8]  = 0x02;
			memset(&setupMsgData->data[9], 0x00, pcharc->_valueSize);
			memcpy(&setupMsgData->data[9], pcharc->_value, pcharc->_valueLength);

			gattSetupMsgOffset += 9 + pcharc->_valueSize;

			nRF8001_sendSetupMessage(pnRF8001, &setupMsg);

			if(pcharc->_properties & (BLENotify | BLEIndicate)){
				setupMsgData->length   = 14;
				setupMsgData->cmd      = ACI_CMD_SETUP;
				setupMsgData->type     = 0x20;
				setupMsgData->offset   = gattSetupMsgOffset;

				setupMsgData->data[0]  = 0x46;
				setupMsgData->data[1]  = 0x14;
				setupMsgData->data[2]  = 0x03;
				setupMsgData->data[3]  = 0x02;
				setupMsgData->data[4]  = (handle >> 8) & 0xff;
				setupMsgData->data[5]  = handle & 0xff;
				pipeInfo->configHandle = handle;
				handle++;

				setupMsgData->data[6]  = 0x29;
				setupMsgData->data[7]  = 0x02;
				setupMsgData->data[8]  = ACI_STORE_LOCAL;
				setupMsgData->data[9]  = 0x00;
				setupMsgData->data[10] = 0x00;

				gattSetupMsgOffset += 11;

				nRF8001_sendSetupMessage(pnRF8001, &setupMsg);
			}
		}
		else if(attribute->_type == BLETypeDescriptor){
			BLEDescriptor* descriptor = (BLEDescriptor *)attribute;

			setupMsgData->length   = 12 + descriptor->_valueSize;
			setupMsgData->cmd      = ACI_CMD_SETUP;
			setupMsgData->type     = 0x20;
			setupMsgData->offset   = gattSetupMsgOffset;

			setupMsgData->data[0]  = 0x04;
			setupMsgData->data[1]  = 0x04;
			setupMsgData->data[2]  = descriptor->_valueSize;
			setupMsgData->data[3]  = descriptor->_valueLength;
			setupMsgData->data[4]  = (handle >> 8) & 0xff;
			setupMsgData->data[5]  = handle & 0xff;
			handle++;

			setupMsgData->data[6]  = puuid->_data[1];
			setupMsgData->data[7]  = puuid->_data[0];
			setupMsgData->data[8]  = ACI_STORE_LOCAL;
			memcpy(&setupMsgData->data[9], descriptor->_value, descriptor->_valueLength);

			gattSetupMsgOffset += 9 + descriptor->_valueSize;

			nRF8001_sendSetupMessage(pnRF8001, &setupMsg);
		}
	}
	pnRF8001->_numPipeInfo = numPiped;

	// terminator
	setupMsgData->length   = 4;
	setupMsgData->cmd      = ACI_CMD_SETUP;
	setupMsgData->type     = 0x20;
	setupMsgData->offset   = gattSetupMsgOffset;

	setupMsgData->data[0]  = 0x00;

	gattSetupMsgOffset += 6;

	nRF8001_sendSetupMessage(pnRF8001, &setupMsg);

	// pipes
	for(i = 0 ; i < numPiped ; i++){
		struct pipeInfo pipeInfo = pnRF8001->_pipeInfo[i];

		setupMsgData->length   = 13;
		setupMsgData->cmd      = ACI_CMD_SETUP;
		setupMsgData->type     = 0x40;
		setupMsgData->offset   = pipeSetupMsgOffet;

		setupMsgData->data[0]  = 0x00;
		setupMsgData->data[1]  = 0x00;
		setupMsgData->data[2]  = pipeInfo.startPipe;
		setupMsgData->data[3]  = 0x00;
		setupMsgData->data[4]  = 0x00;
		setupMsgData->data[5]  = 0x04;
		setupMsgData->data[6]  = (pipeInfo.valueHandle >> 8) & 0xff;
		setupMsgData->data[7]  = pipeInfo.valueHandle & 0xff;
		setupMsgData->data[8]  = (pipeInfo.configHandle >> 8) & 0xff;
		setupMsgData->data[9]  = pipeInfo.configHandle & 0xff;

		if(pipeInfo.pcharc->_properties & BLEIndicate)
			setupMsgData->data[4] |= 0x04; // TX Ack
		if(pipeInfo.pcharc->_properties & BLENotify)
			setupMsgData->data[4] |= 0x02; // TX
		if(pipeInfo.pcharc->_properties & BLEWriteWithoutResponse)
			setupMsgData->data[4] |= 0x08; // RX Ack
		if(pipeInfo.pcharc->_properties & BLEWrite)
			setupMsgData->data[4] |= 0x10; // RX Ack
		if(pipeInfo.pcharc->_properties & BLERead)
			setupMsgData->data[4] |= 0x80; // Set

		pipeSetupMsgOffet += 10;

		nRF8001_sendSetupMessage(pnRF8001, &setupMsg);
	}
	nRF8001_sendCrc(pnRF8001);
}

/*
 *  nRF8001参照
 */
bool_t
nRF8001_poll(nRF8001* pnRF8001)
{
	if(lib_aci_event_get(&pnRF8001->_aciState, &pnRF8001->_aciData)){
		aci_evt_t* aciEvt = &pnRF8001->_aciData.evt;
		int i;
#ifdef NRF_8001_DEBUG
		uint32_t tmp1, tmp2;
		uint32_t tmp3, tmp4;

		syslog_3(LOG_NOTICE, " nRF8001_poll [%02x][%02x][%08x]", aciEvt->evt_opcode, aciEvt->params.device_started.device_mode, aciEvt);
#endif
		switch(aciEvt->evt_opcode){
		/*
		 *  スタートイベント(リセット後)
		 */
		case ACI_EVT_DEVICE_STARTED: {
			pnRF8001->_aciState.data_credit_total = aciEvt->params.device_started.credit_available;
			switch(aciEvt->params.device_started.device_mode) {
			case ACI_DEVICE_SETUP:
				/*
				 *  DEVICEセットアップ
				 */
#ifdef NRF_8001_DEBUG
				syslog_0(LOG_NOTICE, "Evt Device Started: Setup");
#endif
				break;

			case ACI_DEVICE_STANDBY:
				/*
				 *  DEVICEスタンバイ
				 */
#ifdef NRF_8001_DEBUG
				syslog_0(LOG_NOTICE, "Evt Device Started: Standby");
#endif
				if(aciEvt->params.device_started.hw_error){
					dly_tsk(20);	/* ハードウェアエラー */
				}
				else{
					lib_aci_connect(0/* in seconds : 0 means forever */, ADVERTISING_INTERVAL);
#ifdef NRF_8001_DEBUG
					syslog_0(LOG_NOTICE, "Advertising started");
#endif
				}
				break;
			default:
				break;
			}
		}
		break;	/* ここまで、DEVICEスタート状態 */

		case ACI_EVT_CMD_RSP:
			//If an ACI command response event comes with an error -> stop
			if(ACI_STATUS_SUCCESS != aciEvt->params.cmd_rsp.cmd_status){
				//ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
				//TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
				//all other ACI commands will have status code of ACI_STATUS_SCUCCESS for a successful command
#ifdef NRF_8001_DEBUG
				syslog_2(LOG_NOTICE, "ACI Command [%02x] Evt Cmd respone: Status [%02x]",
					aciEvt->params.cmd_rsp.cmd_opcode,
					aciEvt->params.cmd_rsp.cmd_status);
#endif
			}
			else{
				switch (aciEvt->params.cmd_rsp.cmd_opcode) {
				case ACI_CMD_GET_DEVICE_VERSION:
					syslog_2(LOG_NOTICE, "VERSION configuration_id[%04x] aci_ver[%02x]",
						aciEvt->params.cmd_rsp.params.get_device_version.configuration_id,
						aciEvt->params.cmd_rsp.params.get_device_version.aci_version);
					syslog_3(LOG_NOTICE, "   setup_format[%02x] setup[%08x] setp_status[%02x]",
						aciEvt->params.cmd_rsp.params.get_device_version.setup_format,
						aciEvt->params.cmd_rsp.params.get_device_version.setup_id,
						aciEvt->params.cmd_rsp.params.get_device_version.setup_status);
					break;

				case ACI_CMD_GET_DEVICE_ADDRESS: {
#ifdef NRF_8001_DEBUG
					syslog_4(LOG_NOTICE, "nRF8001_poll ACI_CMD_GET_DEVICE_ADDRESS type(%d) [%02x][%02x][%02x]",
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_type,
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own[5],
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own[4],
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own[3]);
					syslog_3(LOG_NOTICE, "nRF8001_poll   [%02x][%02x][%02x]",
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own[2],
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own[1],
						aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own[0]);
#endif
					if(pnRF8001->nRF8001AddressReceived){
						pnRF8001->nRF8001AddressReceived(pnRF8001, aciEvt->params.cmd_rsp.params.get_device_address.bd_addr_own);
					}
					break;
				}

				case ACI_CMD_GET_BATTERY_LEVEL: {
					float batteryLevel = aciEvt->params.cmd_rsp.params.get_battery_level.battery_level * 0.00352;
					if(pnRF8001->nRF8001BatteryLevelReceived){
						pnRF8001->nRF8001BatteryLevelReceived(pnRF8001, batteryLevel);
					}
					break;
				}

				case ACI_CMD_GET_TEMPERATURE: {
					float temperature = aciEvt->params.cmd_rsp.params.get_temperature.temperature_value / 4.0;
					if(pnRF8001->nRF8001TemperatureReceived){
						pnRF8001->nRF8001TemperatureReceived(pnRF8001, temperature);
					}
					break;
				}
				default:
					break;
				}
	        }
    	    break;

		case ACI_EVT_CONNECTED:

#ifdef NRF_8001_DEBUG
			syslog_3(LOG_NOTICE, "Evt Connected [%02x][%02x][%02x]",
				aciEvt->params.connected.dev_addr[5],
				aciEvt->params.connected.dev_addr[4],
				aciEvt->params.connected.dev_addr[3]):
			syslog_3(LOG_NOTICE, "              [%02x][%02x][%02x]",
				aciEvt->params.connected.dev_addr[2],
				aciEvt->params.connected.dev_addr[1],
				aciEvt->params.connected.dev_addr[0]);
#endif
			if(pnRF8001->nRF8001Connected){
				pnRF8001->nRF8001Connected(pnRF8001, aciEvt->params.connected.dev_addr);
			}
			pnRF8001->_aciState.data_credit_available = pnRF8001->_aciState.data_credit_total;
			break;

		case ACI_EVT_PIPE_STATUS:
#ifdef NRF_8001_DEBUG
			memcpy(&tmp1, &aciEvt->params.pipe_status.pipes_open_bitmap[0], 4);
			memcpy(&tmp2, &aciEvt->params.pipe_status.pipes_open_bitmap[4], 4);
			memcpy(&tmp3, &aciEvt->params.pipe_status.pipes_closed_bitmap[0], 4);
			memcpy(&tmp4, &aciEvt->params.pipe_status.pipes_closed_bitmap[4], 4);
			syslog_4(LOG_NOTICE, "Evt Pipe Status [%04x%04x][%04x%04x]", tmp1, tmp2, tmp3, tmp4);
#endif
			for(i = 0 ; i < pnRF8001->_numPipeInfo ; i++){
				struct pipeInfo* pipeInfo = &pnRF8001->_pipeInfo[i];

				if(pipeInfo->txPipe){
					pipeInfo->txPipeOpen = lib_aci_is_pipe_available(&pnRF8001->_aciState, pipeInfo->txPipe);
				}
				if(pipeInfo->txAckPipe){
					pipeInfo->txAckPipeOpen = lib_aci_is_pipe_available(&pnRF8001->_aciState, pipeInfo->txAckPipe);
				}

				bool subscribed = (pipeInfo->txPipeOpen || pipeInfo->txAckPipeOpen);

				if(pipeInfo->pcharc->_subscribed != subscribed){
					if(pnRF8001->nRF8001CharacteristicSubscribedChanged){
						pnRF8001->nRF8001CharacteristicSubscribedChanged(pnRF8001, pipeInfo->pcharc, subscribed);
					}
				}
			}
			break;

		case ACI_EVT_TIMING:
			break;

		case ACI_EVT_DISCONNECTED:
#ifdef NRF_8001_DEBUG
			syslog_0(LOG_NOTICE, "Evt Disconnected/Advertising timed out");
#endif
			// all characteristics unsubscribed on disconnect
			for(i = 0 ; i < pnRF8001->_numPipeInfo ; i++){
				struct pipeInfo* pipeInfo = &pnRF8001->_pipeInfo[i];

				if(pipeInfo->pcharc->_subscribed){
					if(pnRF8001->nRF8001CharacteristicSubscribedChanged){
						pnRF8001->nRF8001CharacteristicSubscribedChanged(pnRF8001, pipeInfo->pcharc, false);
					}
				}
			}
			if(pnRF8001->nRF8001Disconnected){
				pnRF8001->nRF8001Disconnected(pnRF8001);
			}

			lib_aci_connect(0/* in seconds  : 0 means forever */, ADVERTISING_INTERVAL);
#ifdef NRF_8001_DEBUG
			syslog_0(LOG_NOTICE, "Advertising started.");
#endif
			break;

		case ACI_EVT_DATA_RECEIVED: {
			uint8_t dataLen = aciEvt->len - 2;
			uint8_t pipe = aciEvt->params.data_received.rx_data.pipe_number;
#ifdef NRF_8001_DEBUG
			syslog_1(LOG_NOTICE, "Data Received, pipe = %d", aciEvt->params.data_received.rx_data.pipe_number);

			for(int i = 0 ; i < dataLen ; i++){
				syslog_2(LOG_NOTICE, "%d:%02x", i, aciEvt->params.data_received.rx_data.aci_data[i]);
	        }
#endif
			for(i = 0 ; i < pnRF8001->_numPipeInfo ; i++){
				struct pipeInfo* pipeInfo = &pnRF8001->_pipeInfo[i];

				if(pipeInfo->rxAckPipe == pipe || pipeInfo->rxPipe == pipe){
					if(pipeInfo->rxAckPipe == pipe){
						lib_aci_send_ack(&pnRF8001->_aciState, pipeInfo->rxAckPipe);
					}
					if(pnRF8001->nRF8001CharacteristicValueChanged){
						pnRF8001->nRF8001CharacteristicValueChanged(pnRF8001, pipeInfo->pcharc, aciEvt->params.data_received.rx_data.aci_data, dataLen);
					}
					break;
				}
			}
		}
		break;

		case ACI_EVT_DATA_CREDIT:
			pnRF8001->_aciState.data_credit_available = pnRF8001->_aciState.data_credit_available + aciEvt->params.data_credit.credit;
			break;

		case ACI_EVT_PIPE_ERROR:
			//See the appendix in the nRF8001 Product Specication for details on the error codes
#ifdef NRF_8001_DEBUG
			syslog_2(LOG_ERROR, "ACI Evt Pipe Error: Pipe #:%d, Pipe Error Code: 0x%x",
				aciEvt->params.pipe_error.pipe_number,
				aciEvt->params.pipe_error.error_code);
#endif
			//Increment the credit available as the data packet was not sent.
			//The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted
			//for the credit.
			if(ACI_STATUS_ERROR_PEER_ATT_ERROR != aciEvt->params.pipe_error.error_code){
				pnRF8001->_aciState.data_credit_available++;
			}
			break;

		case ACI_EVT_HW_ERROR:
#ifdef NRF_8001_DEBUG
			syslog_1(LOG_ERROR, "HW error: %d", aciEvt->params.hw_error.line_num);

			for(uint8_t counter = 0; counter <= (aciEvt->len - 3); counter++) {
				syslog_1(LOG_ERROR, "[%s]", aciEvt->params.hw_error.file_name[counter]);
			}
#endif
			lib_aci_connect(0/* in seconds, 0 means forever */, ADVERTISING_INTERVAL);
#ifdef NRF_8001_DEBUG
			syslog_0(LOG_ERROR, "Advertising started.");
#endif
			break;
		default:
			break;
		}
		return true;
	}
	else
		return false;
}

/*
 *  nRF8001キャラクティクス値更新
 */
bool_t
nRF8001_updateCharacteristicValue(nRF8001* pnRF8001, BLECharacteristic *pcharc)
{
	bool_t success = true;
	int i;

	for(i = 0 ; i < pnRF8001->_numPipeInfo ; i++){
		struct pipeInfo* pipeInfo = &pnRF8001->_pipeInfo[i];

		if(pipeInfo->pcharc == pcharc){
			if(pipeInfo->setPipe){
				success &= lib_aci_set_local_data(&pnRF8001->_aciState, pipeInfo->setPipe, (uint8_t*)pcharc->_value, pcharc->_valueLength);
			}
			if(pipeInfo->txPipe && pipeInfo->txPipeOpen){
				if(pcharc->canNotifyCharacteristic(pnRF8001, pcharc)){
					pnRF8001->_aciState.data_credit_available--;
					success &= lib_aci_send_data(pipeInfo->txPipe, (uint8_t*)pcharc->_value, pcharc->_valueLength);
				}
				else{
					success = false;
				}
			}

			if(pipeInfo->txAckPipe && pipeInfo->txAckPipeOpen){
				if(pcharc->canIndicateCharacteristic(pnRF8001, pcharc)){
					pnRF8001->_aciState.data_credit_available--;
					success &= lib_aci_send_data(pipeInfo->txAckPipe, (uint8_t*)pcharc->_value, pcharc->_valueLength);
				}
				else{
					success = false;
				}
			}
			break;
		}
	}
	return success;
}

/*
 *  nRF8001 NOTIFYキャラクティクス可能判定
 */
bool_t
nRF8001_canNotifyCharacteristic(nRF8001* pnRF8001, BLECharacteristic *pcharc) {
	return (lib_aci_get_nb_available_credits(&pnRF8001->_aciState) > 0);
}

/*
 *  nRF8001 INDICATEキャラクティクス可能判定
 */
bool_t
nRF8001_canIndicateCharacteristic(nRF8001* pnRF8001, BLECharacteristic *pcharc) {
	return (lib_aci_get_nb_available_credits(&pnRF8001->_aciState) > 0);
}

/*
 *  nRF8001切断
 */
void
nRF8001_disconnect(nRF8001* pnRF8001)
{
	lib_aci_disconnect(&pnRF8001->_aciState, ACI_REASON_TERMINATE);
}

/*
 *  nRF8001アドレス要求
 */
void
nRF8001_requestAddress(void)
{
	lib_aci_get_address();
}

/*
 *  nRF8001温度要求
 */
void
nRF8001_requestTemperature(void)
{
	lib_aci_get_temperature();
}

/*
 *  nRF8001バッテリレベル要求
 */
void
nRF8001_requestBatteryLevel(void)
{
	lib_aci_get_battery_level();
}

/*
 *  nRF8001セットアップモード待ち
 */
void
nRF8001_waitForSetupMode(nRF8001* pnRF8001)
{
	bool_t setupMode = false;

	while(!setupMode){
		if(lib_aci_event_get(&pnRF8001->_aciState, &pnRF8001->_aciData)){
			aci_evt_t* aciEvt = &pnRF8001->_aciData.evt;

			switch(aciEvt->evt_opcode){
			case ACI_EVT_DEVICE_STARTED: {
				switch(aciEvt->params.device_started.device_mode) {
				case ACI_DEVICE_SETUP:
					/**
					When the device is in the setup mode
					*/
#ifdef NRF_8001_DEBUG
					syslog_0(LOG_NOTICE, "Evt Device Started: Setup");
#endif
					setupMode = true;
					break;
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}
		else{
			dly_tsk(1);
		}
	}
}

/*
 *  nRF8001セットアップメッセージ送信
 */
ER
nRF8001_sendSetupMessage(nRF8001* pnRF8001, hal_aci_data_t* data)
{
	bool_t setupMsgSent = false;
	int i = 0;

	pnRF8001->_crcSeed = crc_16_ccitt(pnRF8001->_crcSeed, data->buffer, data->buffer[0] + 1);
	hal_aci_tl_send(data);

	while(!setupMsgSent){
		if(lib_aci_event_get(&pnRF8001->_aciState, &pnRF8001->_aciData)){
			aci_evt_t* aciEvt = &pnRF8001->_aciData.evt;

			if(aciEvt->evt_opcode == ACI_EVT_CMD_RSP && aciEvt->params.cmd_rsp.cmd_status == ACI_STATUS_TRANSACTION_CONTINUE){
#ifdef NRF_8001_DEBUG
				syslog_2(LOG_NOTICE, "Evt Cmd Rsp: Transaction Continue evt[%02x] sts[%02x]", aciEvt->evt_opcode, aciEvt->params.cmd_rsp.cmd_status);
#endif
				setupMsgSent = true;
				break;
			}
		}
		else{
			dly_tsk(1);
			i++;
		}
		if(i > 500)
			return E_TMOUT;
	}
	return E_OK;
}

/*
 *  nRF8001 CRC送信
 */
void
nRF8001_sendCrc(nRF8001* pnRF8001)
{
	bool_t setupMsgSent = false;
	hal_aci_data_t data;
	data.status_byte = 0;

	data.buffer[0]  = 3 + 3;
	data.buffer[1]  = ACI_CMD_SETUP;
	data.buffer[2]  = 0xf0;
	data.buffer[3]  = 0x00;
	data.buffer[4] = 0x03;
	pnRF8001->_crcSeed = crc_16_ccitt(pnRF8001->_crcSeed, data.buffer, data.buffer[0] - 1);
	data.buffer[5] = (pnRF8001->_crcSeed >> 8) & 0xff;
	data.buffer[6] = pnRF8001->_crcSeed & 0xff;

	hal_aci_tl_send(&data);

	while(!setupMsgSent){
		if(lib_aci_event_get(&pnRF8001->_aciState, &pnRF8001->_aciData)){
			aci_evt_t* aciEvt = &pnRF8001->_aciData.evt;

			switch(aciEvt->evt_opcode){
			case ACI_EVT_CMD_RSP: {
				switch(aciEvt->params.cmd_rsp.cmd_status) {
				case ACI_STATUS_TRANSACTION_COMPLETE:
#ifdef NRF_8001_DEBUG
					syslog_0(LOG_NOTICE, "Evt Cmd Rsp: Transaction Complete");
#endif
					setupMsgSent = true;
					break;
				default:
					break;
				}
			}
			break;
			default:
				break;
			}
		}
		else{
			dly_tsk(1);
		}
	}
}

