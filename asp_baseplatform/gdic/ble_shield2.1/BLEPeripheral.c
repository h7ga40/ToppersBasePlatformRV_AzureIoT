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


#define DEFAULT_DEVICE_NAME "Arduino"
#define DEFAULT_APPEARANCE  0x0000

static unsigned char _devicevalue[NUM_DEVICE_NAME];
static unsigned char _appearvalue[NUM_APPEAR_VALUE];
static unsigned char _servicevalue[NUM_SERVICE_VALUE];

static BLEService _genericAccessService = {
	{"1800", {0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	BLETypeService
};

static BLECharacteristic _deviceNameCharacteristic = {
	{{"2a00", {0x00, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	BLETypeCharacteristic},
	BLERead,        /* _properties */
	19,             /* _valueSize */
	_devicevalue,   /* _value */
	0,              /* _valueLength */
	0,              /* _fixedLength */
	false,          /* _notify */
	false,          /* _written */
	false,          /* _subscribed */
	NULL,           /* _pblePeripheral */
	NULL,           /* characteristicValueChanged */
	NULL,           /* canNotifyCharacteristic */
	NULL,           /* canIndicateCharacteristic */
	{0, 0, 0}       /* _eventHandlers[3] */
};

static BLECharacteristic _appearanceCharacteristic = {
	{{"2a01", {0x01, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	BLETypeCharacteristic},
	BLERead,        /* _properties */
	2,              /* _valueSize */
	_appearvalue,   /* _value */
	0,              /* _valueLength */
	0,              /* _fixedLength */
	false,          /* _notify */
	false,          /* _written */
	false,          /* _subscribed */
	NULL,           /* _pblePeripheral */
	NULL,           /* characteristicValueChanged */
	NULL,           /* canNotifyCharacteristic */
	NULL,           /* canIndicateCharacteristic */
	{0, 0, 0}       /* _eventHandlers[3] */
};

static BLEService _genericAttributeService = {
	{"1801", {0x01, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	BLETypeService
};

static BLECharacteristic _servicesChangedCharacteristic = {
	{{"2a05", {0x05, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	BLETypeCharacteristic},
	BLEIndicate,    /* _properties */
	4,              /* _valueSize */
	_servicevalue,  /* _value */
	0,              /* _valueLength */
	0,              /* _fixedLength */
	false,          /* _notify */
	false,          /* _written */
	false,          /* _subscribed */
	NULL,           /* _pblePeripheral */
	NULL,           /* characteristicValueChanged */
	NULL,           /* canNotifyCharacteristic */
	NULL,           /* canIndicateCharacteristic */
	{0, 0, 0}       /* _eventHandlers[3] */
};

static bool_t
BLEPeripheral_characteristicValueChanged(nRF8001* nRF8001, BLECharacteristic *pcharc)
{
	return nRF8001_updateCharacteristicValue(nRF8001, pcharc);
}

static bool_t
BLEPeripheral_canNotifyCharacteristic(nRF8001* nRF8001, BLECharacteristic *pcharc)
{
	return nRF8001_canNotifyCharacteristic(nRF8001, pcharc);
}

static bool_t
BLEPeripheral_canIndicateCharacteristic(nRF8001* nRF8001, BLECharacteristic *pcharc)
{
	return nRF8001_canIndicateCharacteristic(nRF8001, pcharc);
}


/*
 *  BLEPeripheralのセットアップ
 */
void
BLEPeripheral_setup(BLEPeripheral *pblep, unsigned char req, unsigned char rdy, unsigned char rst)
{
	memset(pblep, 0, sizeof(BLEPeripheral));
	pblep->_nRF8001._aciState.aci_pins.reqn_pin = req;
	pblep->_nRF8001._aciState.aci_pins.rdyn_pin = rdy;
	pblep->_nRF8001._aciState.aci_pins.reset_pin = rst;

	pblep->_nRF8001._aciState.aci_pins.board_name = BOARD_DEFAULT;
	pblep->_nRF8001._aciState.aci_pins.active_pin  = UNUSED;
	pblep->_nRF8001._aciState.aci_pins.optional_chip_sel_pin = UNUSED;
	pblep->_nRF8001._aciState.aci_pins.interface_is_interrupt = false;
	pblep->_nRF8001._aciState.aci_pins.event_taskid = 0;
	pblep->_nRF8001._pipeInfo = NULL;
	pblep->_nRF8001._numPipeInfo = 0;
	pblep->_nRF8001._crcSeed = 0xFFFF;
	pblep->_nRF8001._pblePeripheral = pblep;

	pblep->_attributes[0] = &_genericAccessService;
	_deviceNameCharacteristic._valueLength = strlen(DEFAULT_DEVICE_NAME);
	memcpy(_devicevalue, DEFAULT_DEVICE_NAME, _deviceNameCharacteristic._valueLength);
	_deviceNameCharacteristic._pblePeripheral = pblep;
	pblep->_attributes[1] = (BLEAttribute*)&_deviceNameCharacteristic;
	_appearanceCharacteristic._pblePeripheral = pblep;
	pblep->_attributes[2] = (BLEAttribute*)&_appearanceCharacteristic;
	pblep->_attributes[3] = &_genericAttributeService;
	_servicesChangedCharacteristic._pblePeripheral = pblep;
	pblep->_attributes[4] = (BLEAttribute*)&_servicesChangedCharacteristic;
	pblep->_numAttributes = 5;
	pblep->_central._peripheral = pblep;
	pblep->_central._connected  = false;
	memset(pblep->_central._address, 0x00, 6);

	pblep->_manufacturerData = NULL;
	pblep->_manufacturerDataLength = 0;
}

/*
 *  BLEPeripheral開始
 */
void
BLEPeripheral_begin(BLEPeripheral *pblep)
{
	uint8_t advertisementData[20];
	uint8_t scanData[20];
	uint8_t advertisementDataLength = 0;
	uint8_t scanDataLength = 0;
	int i;

	if(pblep->_advertisedServiceUuid){
		unsigned char advertisedServiceUuidLength = sizeof(pblep->_advertisedServiceUuid);
		advertisementDataLength = 2 + advertisedServiceUuidLength;
		advertisementData[0] = (advertisedServiceUuidLength > 2) ? 0x06 : 0x02;
		advertisementData[1] = advertisedServiceUuidLength;
		memcpy(&advertisementData[2], pblep->_advertisedServiceUuid, advertisedServiceUuidLength);
	}
	else if(pblep->_manufacturerData && pblep->_manufacturerDataLength > 0) {
		if(pblep->_manufacturerDataLength > sizeof(advertisementData))
			pblep->_manufacturerDataLength = sizeof(advertisementData);

		advertisementDataLength = 2 + pblep->_manufacturerDataLength;
		advertisementData[0] = 0xff;
		advertisementData[1] = pblep->_manufacturerDataLength;
		memcpy(&advertisementData[2], pblep->_manufacturerData, pblep->_manufacturerDataLength);
	}

	if(pblep->_localName){
		unsigned char originalLocalNameLength = strlen(pblep->_localName);
		unsigned char localNameLength = originalLocalNameLength;

		if(localNameLength > sizeof(scanData))
			localNameLength = sizeof(scanData);
		scanDataLength = 2 + localNameLength;
		scanData[0] = (originalLocalNameLength > sizeof(scanData)) ? 0x08 : 0x09;
		scanData[1] = localNameLength;
		memcpy(&scanData[2], pblep->_localName, localNameLength);
	}

	for(i = 0 ; i < pblep->_numAttributes ; i++){
		BLEAttribute* attribute = pblep->_attributes[i];
		if(attribute->_type == BLETypeCharacteristic){
			BLECharacteristic* pcharc = (BLECharacteristic*)attribute;
			pcharc->characteristicValueChanged = BLEPeripheral_characteristicValueChanged;
			pcharc->canNotifyCharacteristic    = BLEPeripheral_canNotifyCharacteristic;
			pcharc->canIndicateCharacteristic  = BLEPeripheral_canIndicateCharacteristic;
		}
	}

	nRF8001_begin(&pblep->_nRF8001, advertisementData, advertisementDataLength, scanData, scanDataLength, pblep->_attributes, pblep->_numAttributes);
	nRF8001_requestAddress();
}

/*
 *  BLEPeripheralの状態ポーリング
 */
void
BLEPeripheral_poll(BLEPeripheral *pblep)
{
	nRF8001_poll(&pblep->_nRF8001);
}

/*
 *  BLEPeripheral UUIDのセット
 */
void
BLEPeripheral_setAdvertisedServiceUuid(BLEPeripheral *pblep, const char* advertisedServiceUuid)
{
	pblep->_advertisedServiceUuid = advertisedServiceUuid;
}

/*
 *  BLEPeripheral MANUFACURER DATAのセット
 */
void
BLEPeripheral_setManufacturerData(BLEPeripheral *pblep, const unsigned char manufacturerData[], unsigned char manufacturerDataLength)
{
	pblep->_manufacturerData = manufacturerData;
	pblep->_manufacturerDataLength = manufacturerDataLength;
}

/*
 *  BLEPeripheral LOCAL NAMEのセット
 */
void
BLEPeripheral_setLocalName(BLEPeripheral *pblep, const char* localName)
{
	pblep->_localName = localName;
}

/*
 *  BLEPeripheral ATTRIBUTEの追加
 */
void
BLEPeripheral_addAttribute(BLEPeripheral *pblep, BLEAttribute* attribute)
{
	if(attribute->_type == BLETypeCharacteristic){
		BLECharacteristic *pcharc = (BLECharacteristic*)attribute;
		pcharc->_pblePeripheral = pblep;
	}
	pblep->_attributes[pblep->_numAttributes++] = attribute;
}

/*
 *  BLEPeripheral DEVICENMAEのセット
 */
void
BLEPeripheral_setDeviceName(BLEPeripheral *pblep, const char* deviceName)
{
	BLECharacteristic_setValue2(&_deviceNameCharacteristic, deviceName);
}

/*
 *  BLEPeripheral APPEARANCEのセット
 */
void
BLEPeripheral_setAppearance(BLEPeripheral *pblep, unsigned short appearance)
{
	BLECharacteristic_setValue1(&_appearanceCharacteristic, (unsigned char *)&appearance, sizeof(appearance));
}

/*
 *  BLEPeripheral切断
 */
void
BLEPeripheral_disconnect(BLEPeripheral *pblep)
{
	nRF8001_disconnect(&pblep->_nRF8001);
}

/*
 *  BLEPeripheral セントラル構造体の取り出し
 */
BLECentral *
BLEPeripheral_central(BLEPeripheral *pblep)
{
	BLEPeripheral_poll(pblep);
	return &pblep->_central;
}

/*
 *  BLEPeripheral セントラル接続判定
 */
bool_t
BLEPeripheral_connected(BLEPeripheral *pblep)
{
	BLEPeripheral_poll(pblep);
	return BLECentral_connected(&pblep->_central);
}

/*
 *  BLEPeripheral ペリフェラルのイベントハンドラを設定
 */
void
BLEPeripheral_setEventHandler(BLEPeripheral *pblep, uint8_t event, BLEPeripheralEventHandler eventHandler)
{
	if(event < 2){
		pblep->_eventHandlers[event] = eventHandler;
	}
}

/*
 *  BLEPeripheral バージョン取得
 */
bool_t
BLEPeripheral_version(BLEPeripheral *pblep)
{
	return lib_aci_device_version();
}


/*
 *  nRF8001Connectedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001Connected(nRF8001* nRF8001, const unsigned char* address)
{
	BLEPeripheral *pblep = nRF8001->_pblePeripheral;
	memcpy(pblep->_central._address, address, 6);
	pblep->_central._connected = true;

#ifdef BLE_PERIPHERAL_DEBUG
	syslog_5(LOG_NOTICE, "Peripheral connected to central:%02:%02:%02:%02:%02:XX ", address[0], address[1], address[2], address[3], address[4]);
#endif

	BLEPeripheralEventHandler eventHandler = pblep->_eventHandlers[BLEConnected];
	if(eventHandler)
		eventHandler(&pblep->_central);
}

/*
 *  nRF8001Disconnectedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001Disconnected(nRF8001* nRF8001)
{
	BLEPeripheral *pblep = nRF8001->_pblePeripheral;
#ifdef BLE_PERIPHERAL_DEBUG
	unsigned char *address = pblep->_central._address;
	syslog_5(LOG_NOTICE, "Peripheral disconnected from central:%02:%02:%02:%02:%02:XX ", address[0], address[1], address[2], address[3], address[4]);
#endif

	BLEPeripheralEventHandler eventHandler = pblep->_eventHandlers[BLEDisconnected];
	if(eventHandler)
		eventHandler(&pblep->_central);
	pblep->_central._connected = false;
}

/*
 *  nRF8001CharacteristicValueChangedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001CharacteristicValueChanged(nRF8001* nRF8001, BLECharacteristic *pcharc, const unsigned char* value, unsigned char valueLength)
{
	BLEPeripheral *pblep = nRF8001->_pblePeripheral;

	if(valueLength > pcharc->_valueSize)
		pcharc->_valueLength = pcharc->_valueSize;
	else
		pcharc->_valueLength = valueLength;
	memcpy(pcharc->_value, value, pcharc->_valueLength);

	pcharc->_notify  = true;
	pcharc->_written = true;

	BLECharacteristicEventHandler eventHandler = pcharc->_eventHandlers[BLEWritten];
	if(eventHandler)
		eventHandler(&pblep->_central, pcharc);
}

/*
 *  nRF8001CharacteristicSubscribedChangedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001CharacteristicSubscribedChanged(nRF8001* nRF8001, BLECharacteristic *pcharc, bool_t subscribed)
{
	BLEPeripheral *pblep = nRF8001->_pblePeripheral;
	pcharc->_subscribed = subscribed;

	BLECharacteristicEventHandler eventHandler = pcharc->_eventHandlers[subscribed ? BLESubscribed : BLEUnsubscribed];

	if(eventHandler)
		eventHandler(&pblep->_central, pcharc);
}

/*
 *  nRF8001AddressReceivedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001AddressReceived(nRF8001* nRF8001, const unsigned char* address)
{
#ifdef BLE_PERIPHERAL_DEBUG
	syslog_5(LOG_NOTICE, "Peripheral address:%02:%02:%02:%02:%02:XX ", address[0], address[1], address[2], address[3], address[4]);
#endif
}

/*
 *  nRF8001TemperatureReceivedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001TemperatureReceived(nRF8001* nRF8001, float temperature)
{
#ifdef BLE_PERIPHERAL_DEBUG
	int32_t  tmp1 = temperature;
	uint32_t tmp2;

	tmp2 = (temperature - tmp1) * 100;
	syslog_2(LOG_NOTICE, "Temperature = %d.%02d", tmp1, tmp2);
#endif
}

/*
 *  nRF8001BatteryLevelReceivedデフォルトコールバック関数
 */
void
BLEPeripheral_nRF8001BatteryLevelReceived(nRF8001* nRF8001, float batteryLevel)
{
#ifdef BLE_PERIPHERAL_DEBUG
	int32_t tmp1 = batteryLevel;
	uint32_t tmp2;

	tmp2 = (batteryLevel - tmp1) * 10000;
	syslog_2(LOG_NOTICE, "Battery level = %d.%04d", tmp1, tmp2);
#endif
}

