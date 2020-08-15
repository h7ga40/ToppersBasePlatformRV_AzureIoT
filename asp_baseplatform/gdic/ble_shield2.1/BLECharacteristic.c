/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2019 by TOPPERS PROJECT
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

#include <stdlib.h>
#include <utility/lib_aci.h>
#include <utility/aci_setup.h>
#include "BLECentral.h"
#include "BLECharacteristic.h"
#include "BLEPeripheral.h"

#define MIN(a, b) ((a) > (b) ? (b) : (a))

static unsigned int
convert_hex(const char a)
{
	if(a >= '0' && a <= '9')
		return a - '0';
	else if(a >= 'A' && a <= 'F')
		return a - 'A' + 10;
	else if(a >= 'a' && a <= 'f')
		return a - 'a' + 10;
	else
		return 0;
}

static int
uuid_setup(BLEUuid* puuid, const char *str)
{
  int i, len = 0;

  puuid->_str = str;
  puuid->_length = 0;
  for (i = strlen(str) - 1; i >= 0 && puuid->_length < MAX_UUID_LENGTH; i -= 2) {
    if (str[i] == '-') {
      i++;
      continue;
    }

	len = convert_hex(str[i - 1]) << 4;
	len |= convert_hex(str[i]);
	puuid->_data[puuid->_length] = len;

    puuid->_length++;
  }
  return puuid->_length;
}

/*
 *  BLEServiceセットアップ
 */
void
BLEService_setup(BLEService* pserv, const char *uuid)
{
	uuid_setup(&pserv->_uuid, uuid);
	pserv->_type = BLETypeService;
}

/*
 *  BLECharacteristicセットアップ
 */
void
BLECharacteristic_setup(BLECharacteristic* pcharc, const char *uuid, unsigned char properties, unsigned char *value, unsigned char len)
{
	memset(pcharc, 0, sizeof(BLECharacteristic));
	uuid_setup(&pcharc->_attr._uuid, uuid);
	pcharc->_attr._type  = BLETypeCharacteristic;
	pcharc->_properties  = properties;
	pcharc->_valueSize   = MIN(BLE_ATTRIBUTE_MAX_VALUE_LENGTH, len);
	pcharc->_value = value;
	pcharc->_valueLength = 0;
	pcharc->_fixedLength = 0;
	pcharc->_notify      = false;
	pcharc->_written     = false;
	pcharc->_subscribed  = false;
}

/*
 *  BLECharacteristic値設定(値の長さあり)
 */
bool_t
BLECharacteristic_setValue1(BLECharacteristic *pcharc, const unsigned char value[], unsigned char length)
{
	bool_t success = true;

	if(length > pcharc->_valueSize)
		pcharc->_valueLength = pcharc->_valueSize;
	else
		pcharc->_valueLength = length;

	memcpy(pcharc->_value, value, pcharc->_valueLength);

	if(pcharc->characteristicValueChanged != NULL){
		BLEPeripheral *pblep = pcharc->_pblePeripheral;
		success = pcharc->characteristicValueChanged(&pblep->_nRF8001, pcharc);
	}
	return success;
}

/*
 *  BLECharacteristic 値設定(値の長さなし)
 */
bool_t
BLECharacteristic_setValue2(BLECharacteristic *pcharc, const char* value)
{
	return BLECharacteristic_setValue1(pcharc, (const unsigned char *)value, strlen(value));
}

/*
 *  BLECharacteristic 値設定(設定後通知)
 */
void
BLECharacteristic_setValue(BLECharacteristic *pcharc, BLECentral* central, const unsigned char value[], unsigned char length)
{
	BLECharacteristicEventHandler eventHandler = pcharc->_eventHandlers[BLEWritten];

	BLECharacteristic_setValue1(pcharc, value, length);
	pcharc->_written = true;
	if(eventHandler != NULL)
		eventHandler(central, pcharc);
}

/*
 *  BLECharacteristic 通知確認
 */
bool_t
BLECharacteristic_notified(BLECharacteristic *pcharc)
{
	bool_t notify = pcharc->_notify;

	if(notify){
		pcharc->_notify = false;
	}
	return notify;
}

/*
 *  BLECharacteristic 書き込み確認
 */
bool_t
BLECharacteristic_written(BLECharacteristic *pcharc)
{
	bool_t written = pcharc->_written;

	if(written){
		pcharc->_written = false;
	}
	return written;
}


/*
 *  BLECharacteristic Subscribed設定
 */
void
BLECharacteristic_setSubscribed(BLECharacteristic *pcharc, BLECentral* central, bool_t subscribed)
{
	BLECharacteristicEventHandler eventHandler = pcharc->_eventHandlers[subscribed ? BLESubscribed : BLEUnsubscribed];

	pcharc->_subscribed = subscribed;
	if(eventHandler != NULL)
		eventHandler(central, pcharc);
}

/*
 *  BLECharacteristic Notify可能判定
 */
bool_t
BLECharacteristic_canNotify(BLECharacteristic *pcharc)
{
	BLEPeripheral *pblep = pcharc->_pblePeripheral;
	if(pblep == NULL)
		return false;
	if(pcharc->canNotifyCharacteristic != NULL)
		return pcharc->canNotifyCharacteristic(&pblep->_nRF8001, pcharc);
	else
		return false;
}

/*
 *  BLECharacteristic Indicate可能判定
 */
bool_t
BLECharacteristic_canIndicate(BLECharacteristic *pcharc)
{
	BLEPeripheral *pblep = pcharc->_pblePeripheral;
	if(pblep == NULL)
		return false;
	if(pcharc->canIndicateCharacteristic != NULL)
		return pcharc->canIndicateCharacteristic(&pblep->_nRF8001, pcharc);
	else
		return false;
}

/*
 *  BLECharacteristic コールバック関数設定
 */
void
BLECharacteristic_setEventHandler(BLECharacteristic *pcharc, uint8_t event, void *eventHandler)
{
	if(event < 3){
		pcharc->_eventHandlers[event] = eventHandler;
	}
}

