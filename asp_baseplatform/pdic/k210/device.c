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
 *  K210用デバイスドライバ
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "kernel_cfg.h"
#include "device.h"


Inline uint64_t
sil_rel_mem(const uint64_t *mem)
{
	uint64_t	data;

	data = *((const volatile uint64_t *) mem);
	return(data);
}

Inline void
sil_wrl_mem(uint64_t *mem, uint64_t data)
{
	*((volatile uint64_t *) mem) = data;
}

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))
#define sil_orl_mem(a, b)		sil_wrl_mem((a), sil_rel_mem(a) | (b))
#define sil_andl_mem(a, b)		sil_wrl_mem((a), sil_rel_mem(a) & ~(b))
#define sil_modl_mem(a, b, c)	sil_wrl_mem((a), (sil_rel_mem(a) & (~b)) | (c))

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))


/*
 *  GPIOモードの内部定義
 */
#define GPIO_MODE               0x00000001
#define GPIO_MODE_IT            0x00010000
#define RISING_EDGE             0x00100000
#define FALLING_EDGE            0x00200000
#define HIGH_LEVEL              0x00400000
#define LOW_LEVEL               0x00800000
#define GPIO_OUTPUT_TYPE        0x00000010

#define GPIO_ID                 0
#define GPIOHS_ID               1

#ifndef NON_INT_GPIOHS_IO
#define NON_INT_GPIOHS_IO       4
#endif

typedef struct {
	uint8_t  maxpin;
	uint8_t  funcbase;
	uint16_t diroff;
	uint16_t outoff;
	uint16_t inoff;
} gpio_pcb_type;

static const gpio_pcb_type pcb_table[2] = {
	{GPIO_MAX_PINNO,   FUNC_GPIO0,
	 TOFF_GPIO_DIRECTION,   TOFF_GPIO_DATA_OUTOUT,  TOFF_GPIO_DATA_INPUT},
	{GPIOHS_MAX_PINNO, FUNC_GPIOHS0,
	 TOFF_GPIOHS_OUTPUT_EN, TOFF_GPIOHS_OUTPUT_VAL, TOFF_GPIOHS_INPUT_VAL}
};

int8_t arduino_gpio_table[ARDUINO_GPIO_PORT];

/*
 *  GPIOの種別IDを取り出す
 */
static int gpio_get_no(unsigned long base)
{
	if(base == TADR_GPIO_BASE)
		return 0;
	else if(base == TADR_GPIOHS_BASE)
		return 1;
	else
		return -1;
}

/*
 *  GPIOの初期設定関数
 */
void
gpio_setup(unsigned long base, GPIO_Init_t *init, uint32_t pin)
{
	const gpio_pcb_type *pt;
	uint32_t dir, io_number, off;
	uint8_t  ch_sel;
	int no = gpio_get_no(base);

	if(no < 0 || init == NULL)
		return;

	pt  = &pcb_table[no];
	if(pin >= pt->maxpin || init->pull >= GPIO_PULLMAX)
		return;
	dir = init->mode & GPIO_MODE;

    for(io_number = 0, off = TOFF_FPIOA_IO ; io_number < FPIOA_NUM_IO ; io_number++, off += 4){
		ch_sel = sil_rew_mem((uint32_t *)(TADR_FPIOA_BASE+off)) & FPIOA_CH_SEL;
        if(ch_sel == (pt->funcbase + pin))
            break;
    }
	if(io_number >= FPIOA_NUM_IO)
		return;

	switch(init->pull){
	case GPIO_NOPULL:
		sil_andw_mem((uint32_t *)(TADR_FPIOA_BASE+off), (FPIOA_PU | FPIOA_PD));
		break;
	case GPIO_PULLDOWN:
		sil_andw_mem((uint32_t *)(TADR_FPIOA_BASE+off), FPIOA_PU);
		sil_orw_mem((uint32_t *)(TADR_FPIOA_BASE+off), FPIOA_PD);
		break;
	case GPIO_PULLUP:
		sil_orw_mem((uint32_t *)(TADR_FPIOA_BASE+off), FPIOA_PU);
		sil_andw_mem((uint32_t *)(TADR_FPIOA_BASE+off), FPIOA_PD);
		break;
	default:
		break;
    }

	if(no == GPIO_ID){	/* GPIO */
		sil_modw_mem((uint32_t *)(base+TOFF_GPIO_DIRECTION), (1<<pin), (dir<<pin));
	}
	else{				/* GPIOHS */
		uint32_t off, off_d;
		off = dir ? TOFF_GPIOHS_OUTPUT_EN : TOFF_GPIOHS_INPUT_EN;
		off_d = !dir ? TOFF_GPIOHS_OUTPUT_EN : TOFF_GPIOHS_INPUT_EN;
		sil_andw_mem((uint32_t *)(base+off_d), 1<<pin);
		sil_orw_mem((uint32_t *)(base+off), 1<<pin);

		sil_andw_mem((uint32_t *)(base+TOFF_GPIOHS_RISE_IE), (1<<pin));
		sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_RISE_IP), (1<<pin));
		sil_andw_mem((uint32_t *)(base+TOFF_GPIOHS_FALL_IE), (1<<pin));
		sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_FALL_IP), (1<<pin));
		sil_andw_mem((uint32_t *)(base+TOFF_GPIOHS_LOW_IE), (1<<pin));
		sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_LOW_IP), (1<<pin));
		sil_andw_mem((uint32_t *)(base+TOFF_GPIOHS_HIGH_IE), (1<<pin));
		sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_HIGH_IP), (1<<pin));
		if((init->mode & GPIO_MODE_IT) != 0){
			if((init->mode & FALLING_EDGE) != 0)
				sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_FALL_IE), (1<<pin));
			if((init->mode & RISING_EDGE) != 0)
				sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_RISE_IE), (1<<pin));
			if((init->mode & LOW_LEVEL) != 0)
				sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_LOW_IE), (1<<pin));
			if((init->mode & HIGH_LEVEL) != 0)
				sil_orw_mem((uint32_t *)(base+TOFF_GPIOHS_HIGH_IE), (1<<pin));
		}
	}
}

/*
 *  GPIOピン設定
 */
void
gpio_set_pin(unsigned long base, uint8_t pin, uint8_t value)
{
	const gpio_pcb_type *pt;
	int no = gpio_get_no(base);
	uint32_t dir;

	if(no < 0)
		return;
	pt  = &pcb_table[no];
	if(pin >= pt->maxpin)
		return;
	dir = (sil_rew_mem((uint32_t *)(base+pt->diroff)) >> pin) & 1;
	if(dir == 0)
		return;
	value &= 1;
	sil_modw_mem((uint32_t *)(base+pt->outoff), (1<<pin), (value<<pin));
}

/*
 *  GPIOピンデータ取得
 */
uint8_t
gpio_get_pin(unsigned long base, uint8_t pin)
{
	const gpio_pcb_type *pt;
	int no = gpio_get_no(base);
	uint32_t dir, off;

	if(no < 0)
		return 0;
	pt  = &pcb_table[no];
	if(pin >= pt->maxpin)
		return 0;
	dir = (sil_rew_mem((uint32_t *)(base+pt->diroff)) >> pin) & 1;
	off = dir ? pt->outoff : pt->inoff;
	return (sil_rew_mem((uint32_t *)(base+off)) >> pin) & 1;
}

/*
 *  GPIOHSの空きファンクション番号を返す
 */
int
gpio_get_gpiohno(uint8_t fpio_pin, bool_t intreq)
{
#if NON_INT_GPIOHS_IO > 0
	static uint8_t io0 = 0;
#endif
	static uint8_t io1 = NON_INT_GPIOHS_IO;
	int io = -1;

	if(arduino_gpio_table[fpio_pin] >= 0)
		return (int)arduino_gpio_table[fpio_pin];
#if NON_INT_GPIOHS_IO > 0
	else if(intreq && io0 < (NON_INT_GPIOHS_IO-1)){
		io = (int)io0++;
		arduino_gpio_table[fpio_pin] = io;
	}
#endif
	else{
		if(io1 <= (FUNC_GPIOHS31-FUNC_GPIOHS0)){
			io = (int)io1++;
			arduino_gpio_table[fpio_pin] = io;
		}
    }
	return io;
}


/*
 *  DMACの設定関数
 */
#define DMAC_COM_INTVALUE     (DMAC_COM_INTSTATUS_SLVIF_DEC_ERR   | DMAC_COM_INTSTATUS_SLVIF_WR2RO_ERR |\
                               DMAC_COM_INTSTATUS_SLVIF_RD2WO_ERR | DMAC_COM_INTSTATUS_SLVIF_WRONHOLD_ERR |\
                               DMAC_COM_INTSTATUS_SLVIF_UNDEFREG_ERR)

#define DMAC_CH_CFG_DEFAULT   (DMACCH_CFG_SRC_MULTBLKTYPE | DMACCH_CFG_DST_MULTBLKTYPE |\
                               DMACCH_CFG_TT_FC | DMACCH_CFG_HS_SEL_SRC | DMACCH_CFG_HS_SEL_DST |\
                               DMACCH_CFG_SRC_PER | DMACCH_CFG_DST_PER)

#define DMAC_CH_CTL_SRC       (DMACCH_CTL_SMS | DMACCH_CTL_SINC | DMACCH_CTL_SRC_TR_WIDTH | DMACCH_CTL_SRC_MSIZE)
#define DMAC_CH_CTL_DST       (DMACCH_CTL_DMS | DMACCH_CTL_DINC | DMACCH_CTL_DST_TR_WIDTH | DMACCH_CTL_DST_MSIZE)
#define DMAC_CH_CTL_DEFAULT   (DMAC_CH_CTL_SRC | DMAC_CH_CTL_DST)

#define DMAC_CH_INT_ERROR     (DMACCH_ENABLE_SRC_DEC_ERR_INSTAT   | DMACCH_ENABLE_DST_DEC_ERR_INSTAT    |\
                               DMACCH_ENABLE_SRC_SLV_ERR_INSTAT   | DMACCH_ENABLE_DST_SLV_ERR_INSTAT)
#define DMAC_CH_INT_DEFAULT   (DMACCH_ENABLE_DMA_TFR_DONE_INTSTAT | DMACCH_ENABLE_SRC_TRANSCOMP_INTSTAT |\
                               DMACCH_ENABLE_DST_TRANSCOMP_INSTAT | DMAC_CH_INT_ERROR)

#define DMAC_CHANNEL_ENABLE   (DMAC_CHEN_CH1_EN | DMAC_CHEN_CH2_EN | DMAC_CHEN_CH3_EN |\
                               DMAC_CHEN_CH4_EN | DMAC_CHEN_CH5_EN | DMAC_CHEN_CH6_EN)

static DMA_Handle_t *pDmaHandler[NUM_DMA_CHANNEL];

/*
 *  DMACの初期化
 */
static void
dmac_init(void)
{
    uint64_t tmp;

	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_EN_PERI), SYSCTL_CLK_EN_PERI_DMA_CLK_EN);

	/*
	 *  DMACリセット
	 */
	sil_orl_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_RESET), DMAC_RESET_RST);
	while((sil_rel_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_RESET)) & DMAC_RESET_RST) != 0);

    /*
	 *  COMMON割込みクリア
	 */
	sil_wrl_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_COM_INTCLEAR), DMAC_COM_INTVALUE);

	/*
	 *  DMAC無効化
	 */
	sil_wrl_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_CFG), 0);
	while(sil_rel_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_CFG)) != 0);

	/*
	 *  チャネル無効化
	 */
	tmp = sil_rel_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_CHEN));
    tmp &= ~DMAC_CHANNEL_ENABLE;
	sil_wrl_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_CHEN), tmp);

	/*
	 *  DMAC有効化
	 */
	sil_wrl_mem((uint64_t *)(TADR_DMAC_BASE+TOFF_DMAC_CFG), (DMAC_CFG_DMAC_EN | DMAC_CFG_INT_EN));
}

/*
 *  チャンネルCFG/CTLレジスタ設定
 */
static void
dma_config(DMA_Handle_t *hdma)
{
	uint64_t cfg, ctl;

	/* DMA-CFGレジスタ設定 */
    cfg = sil_rel_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_CFG));
	cfg &= ~(DMAC_CH_CFG_DEFAULT | DMACCH_CFG_SRC_HWHS_POL | DMACCH_CFG_DST_HWHS_POL | DMACCH_CFG_CH_PRIOR);
	cfg |= (hdma->Init.SrcMultBlock << 0) | (hdma->Init.DrcMultBlock << 2);
	cfg |= (uint64_t)hdma->Init.Direction << 32;
	cfg |= ((uint64_t)hdma->Init.SrcHandShake << 35) | ((uint64_t)hdma->Init.DrcHandShake << 36);
	cfg |= ((uint64_t)hdma->Init.SrcHwhsPol << 37)   | ((uint64_t)hdma->Init.DrcHwhsPol << 38);
	cfg |= ((uint64_t)hdma->chnum << 39)    | ((uint64_t)hdma->chnum << 44);
	cfg |= (uint64_t)hdma->Init.Priority << 49;
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_CFG), cfg);

	/* DMA-CTLレジスタ設定 */
    ctl = sil_rel_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_CTL));
	ctl &= ~(DMAC_CH_CTL_DEFAULT | DMACCH_CTL_IOC_BLKTFR);
	ctl |= (hdma->Init.SrcMaster << 0) | (hdma->Init.DstMaster << 2);
	ctl |= (hdma->Init.SrcInc << 4)    | (hdma->Init.DstInc << 6);
	ctl |= (hdma->Init.SrcTransWidth << 8) | (hdma->Init.DstTransWidth << 11);
	ctl |= (hdma->Init.SrcBurstSize << 14) | (hdma->Init.DstBurstSize << 18);
	ctl |= (uint64_t)hdma->Init.IocBlkTrans << 58;
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_CTL), ctl);
}

/*
 *  CHANNEL DMA初期化関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_init(DMA_Handle_t *hdma)
{
	/* パラメータチェック */
	if(hdma == NULL || hdma->chnum >= NUM_DMA_CHANNEL)
		return E_PAR;
	if(pDmaHandler[hdma->chnum] != NULL)
		return E_OBJ;

	hdma->base  = TADR_DMAC_BASE;
	hdma->cbase = hdma->base + TOFF_DMAC_CHANNEL + hdma->chnum * DMAC_CHANNEL_WINDOW_SIZE;
	select_dma_channel(hdma->chnum, hdma->Init.Request);
	pDmaHandler[hdma->chnum] = hdma;

	/* DMA-CFG/CTLレジスタ設定 */
	dma_config(hdma);

	/* エラー状態をクリア */
	hdma->status    = DMA_STATUS_READY;
	hdma->ErrorCode = DMA_ERROR_NONE;
	return E_OK;
}

/*
 *  チャネルDMA終了関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_deinit(DMA_Handle_t *hdma)
{
	/* パラメータチェック */
	if(hdma == NULL || hdma->chnum >= NUM_DMA_CHANNEL)
		return E_PAR;

	/*
	 *  チャンネルの無効化
	 */
	dma_end(hdma);
	pDmaHandler[hdma->chnum] = NULL;

	/* エラー状態をクリア */
	hdma->ErrorCode = DMA_ERROR_NONE;
	return E_OK;
}

/*
 *  CHANNEL DMAリセット関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_reset(DMA_Handle_t *hdma)
{
	uint32_t tick = 0;

	/* パラメータチェック */
	if(hdma == NULL || hdma->chnum >= NUM_DMA_CHANNEL)
		return E_PAR;

	while((sil_rel_mem((uint64_t *)(hdma->base+TOFF_DMAC_CHEN)) & (1 << hdma->chnum)) != 0){
		dly_tsk(1);
		if(tick++ >= DMA_TRS_TIMEOUT)
			return E_TMOUT;
	}

	/* DMA-CFGレジスタ設定 */
	dma_config(hdma);
	return E_OK;
}

/*
 *  チャネルDMA開始関数
 *  parameter1  hdma:       DMAハンドラへのポインタ
 *  parameter2  SrcAddress: ソースアドレス
 *  parameter3  DstAddress: デスティネーションアドレス
 *  parameter4  DataLength: 転送長
 *  return ER値
 */
ER
dma_start(DMA_Handle_t *hdma, uintptr_t SrcAddress, uintptr_t DstAddress, uint32_t DataLength)
{
	uint64_t tmp;

	/* パラメータチェック */
	if(hdma == NULL || hdma->chnum >= NUM_DMA_CHANNEL)
		return E_PAR;

	hdma->status = DMA_STATUS_BUSY;
	hdma->ErrorCode = DMA_ERROR_NONE;

	/*
	 *  割込み要因クリア
	 */
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_INTCLEAR), 0xFFFFFFFF);

	/*
	 *  転送設定
	 */
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_SAR), (uint64_t)SrcAddress);
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_DAR), (uint64_t)DstAddress);
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_BLOCK_TS), DataLength - 1);

	/*
	 *  DMAC有効化
	 */
	sil_wrl_mem((uint64_t *)(hdma->base+TOFF_DMAC_CFG), (DMAC_CFG_DMAC_EN | DMAC_CFG_INT_EN));
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_INTSTATUS_EN), DMAC_CH_INT_DEFAULT);

	/* DMA開始 */
	tmp = sil_rel_mem((uint64_t *)(hdma->base+TOFF_DMAC_CHEN));
	tmp |= (DMAC_CHEN_CH1_EN | DMAC_CHEN_CH1_EN_WE) << hdma->chnum;
	sil_wrl_mem((uint64_t *)(hdma->base+TOFF_DMAC_CHEN), tmp);
	return E_OK;
}

/*
 *  チャネルDMA停止関数
 *  parameter1  hdma  : DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_end(DMA_Handle_t *hdma)
{
	uint64_t tmp;

	if(hdma == NULL || hdma->chnum >= NUM_DMA_CHANNEL)
		return E_PAR;

	/* DMA停止 */
	tmp = sil_rel_mem((uint64_t *)(hdma->base+TOFF_DMAC_CHEN));
	tmp &= ~DMAC_CHEN_CH1_EN << hdma->chnum;
	tmp |= DMAC_CHEN_CH1_EN_WE << hdma->chnum;
	sil_wrl_mem((uint64_t *)(hdma->base+TOFF_DMAC_CHEN), tmp);

	hdma->status    = DMA_STATUS_READY;
	return E_OK;
}

/*
 *  チャネルDMA割込み処理関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 */
void
dma_inthandler(DMA_Handle_t *hdma)
{
	uint64_t intstatus, intstatus_en;

	intstatus    = sil_rel_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_INTSTATUS));
	intstatus_en = sil_rel_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_INTSTATUS_EN));
	syslog_2(LOG_DEBUG, "dma_inthandler instatus[%08x][%08x]", (int)intstatus, (int)intstatus_en);
	if((intstatus & intstatus_en & DMACCH_ENABLE_DMA_TFR_DONE_INTSTAT) != 0){
		hdma->status = DMA_STATUS_READY_TRN1;
		if(hdma->xfercallback != NULL)
			hdma->xfercallback(hdma);
	}
	else if((intstatus & intstatus_en & DMACCH_ENABLE_SRC_TRANSCOMP_INTSTAT) != 0){
		hdma->status = DMA_STATUS_READY_TRN2;
		if(hdma->xfercallback != NULL)
			hdma->xfercallback(hdma);
	}
	else if((intstatus & intstatus_en & DMACCH_ENABLE_DST_TRANSCOMP_INSTAT) != 0){
		hdma->status = DMA_STATUS_READY_TRN3;
		if(hdma->xfercallback != NULL)
			hdma->xfercallback(hdma);
	}
	if((intstatus & intstatus_en & DMAC_CH_INT_ERROR) != 0){
		hdma->ErrorCode |= intstatus & intstatus_en & DMAC_CH_INT_ERROR;
		if(hdma->errorcallback != NULL)
			hdma->errorcallback(hdma);
	}
	sil_wrl_mem((uint64_t *)(hdma->cbase+TOFF_DMAC_CH_INTCLEAR), intstatus);
}

/*
 *  チャネルDMA 割込みサービスルーチン
 */
void
channel_dmac_isr(intptr_t exinf)
{
	dma_inthandler(pDmaHandler[(uint32_t)exinf]);
}

ER
select_dma_channel(uint8_t channel, uint8_t select)
{
	uint32_t dma_sel;

	if(select >= NUM_DMA_SELECT)
		return E_PAR;
	if(channel < DMA_CHANNEL5){
		dma_sel  = sil_rew_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_DMA_SEL0));
		dma_sel &= ~(0x3F << (channel * 6));
		dma_sel |= select << (channel * 6);
		sil_wrw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_DMA_SEL0), dma_sel);
		return E_OK;
	}
	else if(channel == DMA_CHANNEL5){
		dma_sel  = sil_rew_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_DMA_SEL1));
		dma_sel &= ~0x3F;
		dma_sel |= select;
		sil_wrw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_DMA_SEL1), dma_sel);
		return E_OK;
	}
	else
	    return E_PAR;
}


/*
 *  WDOGドライバ
 */
/*
 *  SPIOポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_WDOG(wdogid)        ((uint_t)((wdogid) - 1))

typedef struct {
	unsigned long   base;
	uint32_t        reset;
	uint32_t        threshold;
	uint32_t        thshift;
	uint32_t        bus_en;
	uint32_t        peri_en;
} WDT_PortControlBlock;

static const WDT_PortControlBlock wdt_table[2] = {
	{TADR_WDT0_BASE, SYSCTL_PERI_RESET_WDT0_RESET, SYSCTL_CLK_TH6_WD0_CLK_THHD,
	 0, SYSCTL_CLK_EN_CENT_APB1_CLK_EN, SYSCTL_CLK_EN_PERI_WDT0_CLK_EN},
	{TADR_WDT1_BASE, SYSCTL_PERI_RESET_WDT1_RESET, SYSCTL_CLK_TH6_WD1_CLK_THHD,
	 8, SYSCTL_CLK_EN_CENT_APB1_CLK_EN, SYSCTL_CLK_EN_PERI_WDT1_CLK_EN}
};

static WDT_Handle_t WdtHandle[NUM_WDOGPORT];


/*
 *  WDT無効化
 */
static void
wdt_disable(WDT_Handle_t *hwdt)
{
	sil_wrw_mem((uint32_t *)(hwdt->base+TOFF_WDT_CRR), WDT_CRR_MASK);
	sil_andw_mem((uint32_t *)(hwdt->base+TOFF_WDT_CR), WDT_CR_ENABLE);
}

/*
 *  WINDOW WATCH-DOG初期化
 *  parameter1  port: WDOGポート番号
 *  return WDTハンドラへのポインタ
 */
WDT_Handle_t *
wdt_init(ID port)
{
	const WDT_PortControlBlock *pcb;
	WDT_Handle_t *hwdt;
	uint8_t  no;
	uint32_t source = SYSCTRL_CLOCK_FREQ_IN0;
	uint32_t threshold;

	if(port < WDOG1_PORTID || port >= NUM_WDOGPORT)
		return NULL;

	no = INDEX_WDOG(port);
	pcb = &wdt_table[no];
	hwdt = &WdtHandle[no];

	hwdt->base  = pcb->base;
	hwdt->wdtno = no;
	hwdt->callback = NULL;

	/*
	 *  WDTリセット
	 */
	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_PERI_RESET), pcb->reset);
	sil_dly_nse(10*1000);
	sil_andw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_PERI_RESET), pcb->reset);

	/*
	 *  WDT初期化
	 */
	sil_andw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_TH6), pcb->threshold);
	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_EN_CENT), pcb->bus_en);
	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_EN_PERI), pcb->peri_en);

	sil_modw_mem((uint32_t *)(hwdt->base+TOFF_WDT_CR), WDT_CR_RMOD_MASK, WDT_CR_RMOD_INTERRUPT);

	threshold = sil_rew_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_TH6)) & pcb->threshold;
	threshold >>= pcb->thshift;

	hwdt->pclk = source / ((threshold + 1) * 2);

    wdt_disable(hwdt);
    return hwdt;
}

/*
 *  WINDOW WATCH-DOG終了設定
 *  parameter1  hwdt: WDOGハンドラ
 *  return ERコード
 */
ER
wdt_deinit(WDT_Handle_t *hwdt)
{
	const WDT_PortControlBlock *pcb;

	if(hwdt == NULL)
		return E_PAR;
	pcb = &wdt_table[hwdt->wdtno];
    wdt_disable(hwdt);
	sil_andw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_EN_PERI), pcb->peri_en);
	hwdt->base = 0;
	return E_OK;
}

/*
 *  WDOGタイマー開始
 *  parameter1  hwdt: WDOGハンドラ
 *  parameter2  mode: タイムアウトモード
 *  parameter3  tmout: タイムアウト時間(ms)
 *  return ERコード
 */
ER_UINT
wdt_start(WDT_Handle_t *hwdt, uint32_t mode, uint32_t timeout_ms)
{
	uint8_t  m_top;
    uint64_t clk;
	uint32_t torr;

	if(hwdt == NULL)
		return E_PAR;
	clk = ((uint64_t)timeout_ms * hwdt->pclk / 1000) >> 16;
	for(m_top = 0, clk >>=1 ; m_top < 16 && clk > 0  ; m_top++){
		clk >>= 1;
	}
	if(m_top > 0xf)
        m_top = 0xf;

	sil_modw_mem((uint32_t *)(hwdt->base+TOFF_WDT_CR), WDT_CR_RMOD_MASK, mode);

	/*
	 *  WDTタイムアウト設定
	 */
	torr = (m_top << 4) | m_top;
	sil_wrw_mem((uint32_t *)(hwdt->base+TOFF_WDT_TORR), torr);

	/*
	 *  WDT有効化
	 */
	sil_wrw_mem((uint32_t *)(hwdt->base+TOFF_WDT_CRR), WDT_CRR_MASK);
	sil_orw_mem((uint32_t *)(hwdt->base+TOFF_WDT_CR), WDT_CR_ENABLE);
    return (1UL << (m_top + 16 + 1)) * 1000UL / hwdt->pclk;
}

/*
 *  WDOGタイマー停止
 *  parameter1  hwdt: WDOGハンドラ
 *  return ERコード
 */
ER
wdt_stop(WDT_Handle_t *hwdt)
{
	if(hwdt == NULL)
		return E_PAR;
    wdt_disable(hwdt);
	return E_OK;
}

/*
 *  WDOG割込サービスコール
 */
void
wdog_isr(intptr_t exinf)
{
	WDT_Handle_t *hwdt = &WdtHandle[INDEX_WDOG(exinf)];
	uint32_t eoi;

	eoi = sil_rew_mem((uint32_t *)(hwdt->base+TOFF_WDT_EOI));
	sil_wrw_mem((uint32_t *)(hwdt->base+TOFF_WDT_EOI), eoi);
	if(hwdt->callback != NULL)
		hwdt->callback(hwdt);
}


/*
 *  RTCデバイス
 */

#define RTC_MASK_VALUE  (RTC_RCTL_TIMER_MASK   | RTC_RCTL_ALARAM_MASK |\
                         RTC_RCTL_INT_CNT_MASK | RTC_RCTL_INT_REG_MASK)

static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};


uint8_t rtc_timer_mode;
static void (*tick_func)(int);
static void (*alarm_func)(int);

Inline bool_t
rtc_in_range(int value, int min, int max)
{
    return ((value >= min) && (value <= max));
}

static int
rtc_get_wday(int year, int month, int day)
{
    /* Magic method to get weekday */
    int weekday = (day += month < 3 ? year-- : year - 2, 23 * month / 9 + day + 4 + year / 4 - year / 100 + year / 400) % 7;
    return weekday;
}

static int
rtc_year_is_leap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int
rtc_get_yday(int year, int month, int day)
{
    int leap = rtc_year_is_leap(year);

    return days[leap][month] + day;
}

/*
 *  RTCプロテクション設定
 */
static void
rtc_protect_set(int enable)
{
	if(enable){
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_REGISTER_CTRL), RTC_MASK_VALUE);
	}
	else{
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_REGISTER_CTRL), RTC_MASK_VALUE);
	}
}

ER
rtc_timer_set_mode(uint8_t timer_mode)
{
	uint32_t regctl = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_REGISTER_CTRL));
    unsigned long freq = (unsigned long)SYS_CLOCK / 26000000;
    unsigned long start_cycle;

	switch(timer_mode){
	case RTC_TIMER_RUNNING:
		regctl |= RTC_RCTL_READ_ENABLE;
		regctl &= ~RTC_RCTL_WRITE_ENABLE;
		break;
	case RTC_TIMER_SETTING:
		regctl &= ~RTC_RCTL_READ_ENABLE;
		regctl |= RTC_RCTL_WRITE_ENABLE;
		break;
	case RTC_TIMER_PAUSE:
	default:
		regctl &= ~(RTC_RCTL_READ_ENABLE | RTC_RCTL_WRITE_ENABLE);
		break;
    }

    /* Wait for 1/26000000 s to sync data */
	start_cycle = read_csr(mcycle);
    while((read_csr(mcycle) - start_cycle) < freq){
        continue;
	}

	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_REGISTER_CTRL), regctl);
	rtc_timer_mode = timer_mode;
    return E_OK;
}

/*
 *  RTC初期化関数
 */
void rtc_init(intptr_t exinf)
{
    /* Reset RTC */
	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_PERI_RESET), SYSCTL_PERI_RESET_RTC_RESET);
    sil_dly_nse(10*1000);
	sil_andw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_PERI_RESET), SYSCTL_PERI_RESET_RTC_RESET);

    /* Enable RTC */
	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_EN_CENT), SYSCTL_CLK_EN_CENT_APB1_CLK_EN);
	sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_CLK_EN_PERI), SYSCTL_CLK_EN_PERI_RTC_CLK_EN);

    /* Unprotect RTC */
    rtc_protect_set(0);
    rtc_timer_set_mode(RTC_TIMER_SETTING);
    /* Set RTC clock frequency */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_INITIAL_COUNT), SYSCTRL_CLOCK_FREQ_IN0);

	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_COURRNT_COUNT), 1);
    /* Set RTC mode to timer running mode */
    rtc_timer_set_mode(RTC_TIMER_RUNNING);

	tick_func = NULL;
	alarm_func = NULL;
}

/*
 *  RTCの時刻設定関数
 *
 *  時刻の設定はPONIXのtm構造体を使用する
 *  PONIXのインクルードがない場合を考慮し、同一項目のtm2をドライバとして定義する。
 */
ER
rtc_set_time(struct tm2 *pt)
{
	uint32_t tm_wday = rtc_get_wday(pt->tm_year + 1900, pt->tm_mon, pt->tm_mday);
	uint32_t extend  = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_EXTENDED));
	int human_year = pt->tm_year + 1900;
	int rtc_year = human_year % 100;
	int rtc_century = human_year / 100;

	if(!rtc_in_range(pt->tm_sec, 0, 59))
		return E_PAR;
	if(!rtc_in_range(pt->tm_min, 0, 59))
		return E_PAR;
	if(!rtc_in_range(pt->tm_hour, 0, 23))
		return E_PAR;
	if(!rtc_in_range(pt->tm_mday, 1, 31))
		return E_PAR;
	if(!rtc_in_range(pt->tm_mon, 1, 12))
		return E_PAR;
	if(!rtc_in_range(rtc_year, 0, 99) || !rtc_in_range(rtc_century, 0, 31))
		return E_PAR;

	/*
	 *  RTC日時をセットアップ
	 */
	rtc_timer_set_mode(RTC_TIMER_SETTING);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_DATE),
		tm_wday | (pt->tm_mday << 8) | (pt->tm_mon << 16) | (rtc_year << 20));
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_TIME),
		(pt->tm_sec << 10) | (pt->tm_min << 16) | (pt->tm_hour << 24));
	extend &= ~RTC_EXTENDED_CENTURY;
	extend |= rtc_century;
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_EXTENDED), extend);
	rtc_timer_set_mode(RTC_TIMER_RUNNING);
	return E_OK;
}

/*
 *  RTCの時刻取り出し関数
 *
 *  時刻の設定はPONIXのtm構造体を使用する
 *  PONIXのインクルードがない場合を考慮し、同一項目のtm2をドライバとして定義する。
 */
ER rtc_get_time(struct tm2 *pt)
{
	uint32_t date = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_DATE));
	uint32_t time = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_TIME));
	uint32_t century = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_EXTENDED)) & RTC_EXTENDED_CENTURY;

    if(rtc_timer_mode != RTC_TIMER_RUNNING)
        return E_OBJ;

    pt->tm_sec = (time & RTC_TIME_SECOND) >> 10;	/* 0-60, follow C99 */
    pt->tm_min = (time & RTC_TIME_MINUTE) >> 16;	/* 0-59 */
    pt->tm_hour = (time & RTC_TIME_HOUR) >> 24;	/* 0-23 */
    pt->tm_mday = (date & RTC_DATE_DAY) >> 8;		/* 1-31 */
    pt->tm_mon  = (date & RTC_DATE_MONTH) >> 16;	/* 1-12 */
    pt->tm_year = (((date & RTC_DATE_YEAR) >> 20) % 100) + (century * 100) - 1900;
    pt->tm_wday = date & RTC_DATE_WEEK;                                                 /* 0-6 */
    pt->tm_yday = rtc_get_yday(pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday) % 366; /* 0-365 */
    pt->tm_isdst = 0;
	return E_OK;
}

/*
 *  RTC TICK割込み設定
 */
ER_UINT
rtc_intmode(int8_t mode, void *func)
{
	uint32_t intctl = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_INTERRUPT_CTRL));
	uint_t cmode;
	if(mode >= 0){
	    rtc_timer_set_mode(RTC_TIMER_SETTING);
		if(func == NULL){
			if(alarm_func == NULL)
				intctl &= ~RTC_INT_TICK_ENABLE;
			tick_func = NULL;
		}
		else{
			intctl &= ~RTC_INT_TICK_INT_MODE;
			intctl |= ((mode & 3) << 2) | RTC_INT_TICK_ENABLE;
			tick_func = (void (*)(int))func;
		}
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_INTERRUPT_CTRL), intctl);
	    rtc_timer_set_mode(RTC_TIMER_RUNNING);
	}
	cmode  = ((intctl & RTC_INT_TICK_ENABLE) ^ 1) << 2;
	cmode |= (intctl & RTC_INT_TICK_INT_MODE) >> 2;
	return cmode;
}

/*
 *  RTCアラーム設定
 *  parameter1 : parm: Pointer to Alarm structure
 *  parameter2 : ptm: Pointer to struct tm2
 *  return ERコード
 */
ER
rtc_setalarm(RTC_Alarm_t *parm, struct tm2 *ptm)
{
	uint32_t intctl = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_INTERRUPT_CTRL));
	int human_year = ptm->tm_year + 1900;
	int rtc_year = human_year % 100;
	int rtc_century = human_year / 100;
	uint32_t alarmmask;

	if(parm == NULL || ptm == NULL)
		return E_PAR;
	alarmmask = parm->alarmmask & RTC_INT_ARARM_C_MASK;
	if(alarmmask != 0 && parm->callback == NULL)
		return E_PAR;

	if(!rtc_in_range(ptm->tm_sec, 0, 59)){
		if((alarmmask & RTC_ALARM_SECOND) != 0)
			return E_PAR;
		ptm->tm_sec = 0;
	}
	if(!rtc_in_range(ptm->tm_min, 0, 59)){
		if((alarmmask & RTC_ALARM_MINUTE) != 0)
			return E_PAR;
		ptm->tm_min = 0;
	}
	if(!rtc_in_range(ptm->tm_hour, 0, 23)){
		if((alarmmask & RTC_ALARM_HOUR) != 0)
			return E_PAR;
		ptm->tm_hour = 0;
	}
	if(!rtc_in_range(ptm->tm_mday, 1, 31)){
		if((alarmmask & RTC_ALARM_DAY) != 0)
			return E_PAR;
		ptm->tm_mday = 1;
	}
	if(rtc_in_range(ptm->tm_wday, 0, 6)){
		if((alarmmask & RTC_ALARM_WEEK) != 0)
			return E_PAR;
		ptm->tm_wday = 0;
	}
	if(!rtc_in_range(ptm->tm_mon, 1, 12)){
		if((alarmmask & RTC_ALARM_MONTH) != 0)
			return E_PAR;
		ptm->tm_mon = 1;
	}
	if(!rtc_in_range(rtc_year, 0, 99) || !rtc_in_range(rtc_century, 0, 31)){
		if((alarmmask & RTC_ALARM_YEAR) != 0)
			return E_PAR;
		ptm->tm_year = 0;
	}

	/*
	 *  RTC日時、割込みをセットアップ
	 */
	rtc_timer_set_mode(RTC_TIMER_SETTING);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_ALARM_DATE),
		ptm->tm_wday | (ptm->tm_mday << 8) | (ptm->tm_mon << 16) | (rtc_year << 20));
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_ALARM_TIME),
		(ptm->tm_sec << 10) | (ptm->tm_min << 16) | (ptm->tm_hour << 24));

	if(tick_func == NULL)
		intctl &= ~RTC_INT_TICK_ENABLE;
	intctl &= ~(RTC_INT_ALARM_ENABLE | RTC_INT_ARARM_C_MASK);
	alarm_func = NULL;
	if(alarmmask != 0){
		intctl |= alarmmask | RTC_INT_TICK_ENABLE | RTC_INT_ALARM_ENABLE;
		alarm_func = (void (*)(int))parm->callback;
	}
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_INTERRUPT_CTRL), intctl);
	rtc_timer_set_mode(RTC_TIMER_RUNNING);
	return E_OK;
}

/*
 *  RTC割込みハンドラ
 */
void
rtc_int_handler(void)
{
	uint32_t intctl = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_INTERRUPT_CTRL));
	uint32_t cdate  = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_DATE));
	uint32_t ctime  = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_TIME));

	if((intctl & RTC_INT_TICK_ENABLE) != 0 && tick_func != NULL){
		int    mode = (intctl & RTC_INT_TICK_INT_MODE) >> 2;
		bool_t tickact = true;
		switch(mode){
		case RTC_INT_MINUTE:
			if((ctime & RTC_TIME_SECOND) != 0)
				tickact = false;
			break;
		case RTC_INT_HOUR:
			if((ctime & (RTC_TIME_SECOND | RTC_TIME_MINUTE)) != 0)
				tickact = false;
			break;
		default:
			break;
		}
		if(tickact)
			tick_func(mode);
	}

	if((intctl & RTC_INT_ALARM_ENABLE) != 0 && alarm_func != NULL){
		uint32_t adate = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_ALARM_DATE));
		uint32_t atime = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_ALARM_TIME));
		uint32_t tdate = cdate;
		uint32_t ttime = ctime;

		if((intctl & RTC_ALARM_YEAR) == 0){
			tdate &= ~RTC_DATE_YEAR;
			adate &= ~RTC_ALARM_DATE_YEAR;
		}
		if((intctl & RTC_ALARM_MONTH) == 0){
			tdate &= ~RTC_DATE_MONTH;
			adate &= ~RTC_ALARM_DATE_MONTH;
		}
		if((intctl & RTC_ALARM_DAY) == 0){
			tdate &= ~RTC_DATE_DAY;
			adate &= ~RTC_ALARM_DATE_DAY;
		}
		if((intctl & RTC_ALARM_WEEK) == 0){
			tdate &= ~RTC_DATE_WEEK;
			adate &= ~RTC_ALARM_DATE_WEEK;
		}
		if((intctl & RTC_ALARM_HOUR) == 0){
			ttime &= ~RTC_TIME_HOUR;
			atime &= ~RTC_ALARM_TIME_HOUR;
		}
		if((intctl & RTC_ALARM_MINUTE) == 0){
			ttime &= ~RTC_TIME_MINUTE;
			atime &= ~RTC_ALARM_TIME_MINUTE;
		}
		if((intctl & RTC_ALARM_SECOND) == 0){
			ttime &= ~RTC_TIME_SECOND;
			atime &= ~RTC_ALARM_TIME_SECOND;
		}
		if(tdate == adate && ttime == atime)
			alarm_func(intctl);
    }
}


/*
 *  デバイス初期化
 */
void
device_init(intptr_t exinf)
{
	unsigned long sbase = TADR_SYSCTL_BASE;
	uint32_t i;

	/*
	 *  GPIOクロック設定
	 */
	sil_orw_mem((uint32_t *)(sbase+TOFF_SYSCTL_CLK_EN_CENT), SYSCTL_CLK_EN_CENT_APB0_CLK_EN);
	sil_orw_mem((uint32_t *)(sbase+TOFF_SYSCTL_CLK_EN_PERI), SYSCTL_CLK_EN_PERI_GPIO_CLK_EN);

	/*
	 *  Arduino用GPIOテーブル初期化
	 */
	for(i = 0 ; i < ARDUINO_GPIO_PORT ; i++){
		arduino_gpio_table[i] = -1;
	}

	/*
	 *  DMAC初期化
	 */
	dmac_init();
}

/*
 *  SPI0-DVPモード設定
 *  parameter1  en: 有効(1)、無効(0)
 *  return ERコード
 */
ER
select_spi0_dvp_mode(uint8_t en)
{
	if(en == 0){
		sil_andw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_MISC), SYSCTL_MISC_SPI_DVP_DATA_ENABLE);
		return E_OK;
	}
	else if(en == 1){
		sil_orw_mem((uint32_t *)(TADR_SYSCTL_BASE+TOFF_SYSCTL_MISC), SYSCTL_MISC_SPI_DVP_DATA_ENABLE);
		return E_OK;
	}
	else
	    return E_PAR;
}

