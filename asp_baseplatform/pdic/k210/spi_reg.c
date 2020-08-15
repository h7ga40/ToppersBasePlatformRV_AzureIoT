/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2019 by TOPPERS PROJECT Educational Working Group.
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
 * 
 *  SPIレガシードライバ関数群
 *
 */
#include "kernel.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "device.h"
#include "spi.h"

/*
 *  SPI受信実行関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  pdata: 受信バッファへのポインタ
 *  parameter3  length: 受信サイズ
 *  return ERコード
 */
ER
spi_transmit(SPI_Handle_t *hspi, uint8_t *ptxData, uint16_t length)
{
	uint32_t *pTx;
	int32_t  ss;
	uint32_t i;
	ER ercd = E_OK;

	if(hspi == NULL)
		return E_PAR;
	ss = hspi->Init.SsNo;
	if(hspi->hdmatx != NULL){
		pTx = malloc(length * sizeof(uint32_t));
		if(pTx == NULL)
			return E_OBJ;
		for(i = 0 ; i < length ; i++)
			pTx[i] = ptxData[i];
	}
	else
		pTx = (uint32_t *)ptxData;

#if SPI_WAIT_TIME != 0
	ercd = spi_core_transmit(hspi, ss, (uint8_t*)pTx, length);
#else
	if((ercd = spi_core_transmit(hspi, ss, (uint8_t*)pTx, length)) == E_OK){
		ercd = spi_wait(hspi, 50);
	}
#endif
	if(hspi->hdmatx != NULL)
		free(pTx);
	return ercd;
}

/*
 *  SPI送信実行関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  pdata: 送信バッファへのポインタ
 *  parameter3  length: 送信サイズ
 *  return ERコード
 */
ER
spi_receive(SPI_Handle_t *hspi, uint8_t *pdata, uint16_t length)
{
	uint32_t *pRx;
	int32_t  ss;
	uint32_t i;
	ER  ercd = E_OK;

	if(hspi == NULL)
		return E_PAR;
	ss = hspi->Init.SsNo;
	pRx = malloc(length * sizeof(uint32_t));
	if(pRx == NULL)
		return E_OBJ;
#if SPI_WAIT_TIME != 0
	ercd = spi_core_receive(hspi, ss, pRx, length);
#else
	if((ercd = spi_core_receive(hspi, ss, pRx, length)) == E_OK)
		ercd = spi_wait(hspi, 50);
#endif
	for(i = 0 ; i < length ; i++)
		pdata[i] = pRx[i];
	free(pRx);
	return ercd;
}

/*
 *  SPI送受信実行関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  ptxdata: 送信バッファへのポインタ
 *  parameter3  prxdata: 受信バッファへのポインタ
 *  parameter4  length: 送受信サイズ
 *  return ERコード
 */
ER
spi_transrecvx(SPI_Handle_t *hspi, uint8_t *ptxdata, uint8_t *prxdata, uint16_t length)
{
	uint32_t *pTx, *pRx;
	int32_t  ss;
	uint32_t i;
	ER  ercd = E_OK;

	if(hspi == NULL)
		return E_PAR;
	ss = hspi->Init.SsNo;
	if(hspi->hdmatx != NULL){
		pRx = malloc(length * sizeof(uint32_t) * 2);
		if(pRx == NULL)
			return E_OBJ;
		pTx = pRx + length;
		for(i = 0 ; i < length ; i++)
			pTx[i] = ptxdata[i];
	}
	else{
		pRx = malloc(length * sizeof(uint32_t));
		if(pRx == NULL)
			return E_OBJ;
		pTx = (uint32_t *)ptxdata;
	}
#if SPI_WAIT_TIME != 0
	ercd = spi_core_transrecv(hspi, ss, (uint8_t *)pTx, (uint8_t *)pRx, length);
#else
	if((ercd = spi_core_transrecv(hspi, ss, (uint8_t *)pTx, (uint8_t *)pRx, length)) == E_OK)
		ercd = spi_wait(hspi, 50);
#endif
	for(i = 0 ; i < length ; i++)
		prxdata[i] = pRx[i];
	free(pRx);
	return ercd;
}

