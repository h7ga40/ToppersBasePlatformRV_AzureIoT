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

#ifndef _BLEPERIPHERAL_H_
#define _BLEPERIPHERAL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define MAX_ATTRIBUTE       32
#define NB_BASE_SETUP_MESSAGES 7
#define NUM_DEVICE_NAME     20
#define NUM_APPEAR_VALUE    4
#define NUM_SERVICE_VALUE   4

#ifndef _BLE_ATTRIBUTE_H_
typedef struct _nRF8001 nRF8001;
#endif

enum BLEPeripheralEvent {
	BLEConnected = 0,
	BLEDisconnected = 1
};

struct pipeInfo {
	BLECharacteristic  *pcharc;

	uint16_t           valueHandle;
	uint16_t           configHandle;

	uint8_t            startPipe;
	uint8_t            txPipe;
	uint8_t            txAckPipe;
	uint8_t            rxPipe;
	uint8_t            rxAckPipe;
	uint8_t            setPipe;

	bool_t             txPipeOpen;
	bool_t             txAckPipeOpen;
};

struct _nRF8001 {
	struct aci_state_t _aciState;
	hal_aci_evt_t      _aciData;

	void*              _pblePeripheral;
	struct pipeInfo*   _pipeInfo;
	uint8_t            _numPipeInfo;
	uint16_t           _crcSeed;

	void (*nRF8001Connected)(nRF8001* nRF8001, const unsigned char* address);
	void (*nRF8001Disconnected)(nRF8001* nRF8001);
	void (*nRF8001CharacteristicValueChanged)(nRF8001* nRF8001, BLECharacteristic *pcharc, const unsigned char* value, unsigned char valueLength);
	void (*nRF8001CharacteristicSubscribedChanged)(nRF8001* nRF8001, BLECharacteristic *pcharc, bool_t subscribed);
	void (*nRF8001AddressReceived)(nRF8001* nRF8001, const unsigned char* address);
	void (*nRF8001TemperatureReceived)(nRF8001* nRF8001, float temperature);
	void (*nRF8001BatteryLevelReceived)(nRF8001* nRF8001, float batteryLevel);
};


typedef void (*BLEPeripheralEventHandler)(BLECentral* central);

typedef struct _BLEPeripheral {
	nRF8001            _nRF8001;

	const char         *_advertisedServiceUuid;
	const uint8_t      *_manufacturerData;
	uint8_t            _manufacturerDataLength;
	const char*        _localName;

	BLEAttribute*      _attributes[MAX_ATTRIBUTE];
	uint8_t            _numAttributes;

	BLECentral         _central;
	BLEPeripheralEventHandler _eventHandlers[2];
} BLEPeripheral;


extern void nRF8001_begin(nRF8001* pnRF8001, const uint8_t *advertisementData, uint8_t advertisementDataLength, const uint8_t *scanData,
                    uint8_t scanDataLength, BLEAttribute** attributes, uint8_t numAttributes);
extern bool_t nRF8001_poll(nRF8001* pnRF8001);
extern bool_t nRF8001_updateCharacteristicValue(nRF8001* pnRF8001, BLECharacteristic *pcharc);
extern bool_t nRF8001_canNotifyCharacteristic(nRF8001* pnRF8001, BLECharacteristic* pcharc);
extern bool_t nRF8001_canIndicateCharacteristic(nRF8001* pnRF8001, BLECharacteristic* pcharc);
extern void nRF8001_disconnect(nRF8001* pnRF8001);
extern void nRF8001_requestAddress(void);
extern void nRF8001_requestTemperature(void);
extern void nRF8001_requestBatteryLevel(void);
extern void nRF8001_waitForSetupMode(nRF8001* pnRF8001);
extern ER nRF8001_sendSetupMessage(nRF8001* pnRF8001, hal_aci_data_t* data);
extern void nRF8001_sendCrc(nRF8001* pnRF8001);

extern void BLEPeripheral_setup(BLEPeripheral *pblep, unsigned char req, unsigned char rdy, unsigned char rst);
extern void BLEPeripheral_begin(BLEPeripheral *pblep);
extern void BLEPeripheral_poll(BLEPeripheral *pblep);
extern void BLEPeripheral_setAdvertisedServiceUuid(BLEPeripheral *pblep, const char* advertisedServiceUuid);
extern void BLEPeripheral_setManufacturerData(BLEPeripheral *pblep, const unsigned char manufacturerData[], unsigned char manufacturerDataLength);
extern void BLEPeripheral_setLocalName(BLEPeripheral *pblep, const char* localName);
extern void BLEPeripheral_addAttribute(BLEPeripheral *pblep, BLEAttribute* attribute);
extern void BLEPeripheral_setDeviceName(BLEPeripheral *pblep, const char* deviceName);
extern void BLEPeripheral_setAppearance(BLEPeripheral *pblep, unsigned short appearance);
extern void BLEPeripheral_disconnect(BLEPeripheral *pblep);
extern BLECentral *BLEPeripheral_central(BLEPeripheral *pblep);
extern bool_t BLEPeripheral_connected(BLEPeripheral *pblep);
extern void BLEPeripheral_setEventHandler(BLEPeripheral *pblep, uint8_t event, BLEPeripheralEventHandler eventHandler);
extern bool_t BLEPeripheral_version(BLEPeripheral *pblep);
extern void BLEPeripheral_nRF8001Connected(nRF8001* nRF8001, const unsigned char* address);
extern void BLEPeripheral_nRF8001Disconnected(nRF8001* nRF8001);
extern void BLEPeripheral_nRF8001CharacteristicValueChanged(nRF8001* nRF8001, BLECharacteristic* pcharc, const unsigned char* value, unsigned char valueLength);
extern void BLEPeripheral_nRF8001CharacteristicSubscribedChanged(nRF8001* nRF8001, BLECharacteristic* pcharc, bool_t subscribed);
extern void BLEPeripheral_nRF8001AddressReceived(nRF8001* nRF8001, const unsigned char* address);
extern void BLEPeripheral_nRF8001TemperatureReceived(nRF8001* nRF8001, float temperature);
extern void BLEPeripheral_nRF8001BatteryLevelReceived(nRF8001* nRF8001, float batteryLevel);

#ifdef __cplusplus
}
#endif

#endif	/* _BLEPERIPHERAL_H_ */
