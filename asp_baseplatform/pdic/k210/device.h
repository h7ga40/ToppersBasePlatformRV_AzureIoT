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
 * K210用デバイスドライバの外部宣言
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*
 *  TOPPERS BASE PLATFORM情報
 */
#define TOPPERS_BASE_PLATFORM_K210
#define SPI_DRIVER_NOSTART

/*
 *  バージョン情報
 */
#define	TPLATFORM_PRVER 0x0012		/* プラットフォームのバージョン番号 */

/*
 * ピン設定
 */

#define PINPOSITION0    0
#define PINPOSITION1    1
#define PINPOSITION2    2
#define PINPOSITION3    3
#define PINPOSITION4    4
#define PINPOSITION5    5
#define PINPOSITION6    6
#define PINPOSITION7    7
#define PINPOSITION8    8
#define PINPOSITION9    9
#define PINPOSITION10   10
#define PINPOSITION11   11
#define PINPOSITION12   12
#define PINPOSITION13	13
#define PINPOSITION14	14
#define PINPOSITION15	15
#define PINPOSITION16	16
#define PINPOSITION17	17
#define PINPOSITION18	18
#define PINPOSITION19	19
#define PINPOSITION20	20
#define PINPOSITION21	21
#define PINPOSITION22	22
#define PINPOSITION23	23
#define PINPOSITION24	24
#define PINPOSITION25	25
#define PINPOSITION26	26
#define PINPOSITION27	27
#define PINPOSITION28	28
#define PINPOSITION29	29
#define PINPOSITION30	30
#define PINPOSITION31	31

#define GPIO_PIN_0      (1<<PINPOSITION0)
#define GPIO_PIN_1      (1<<PINPOSITION1)
#define GPIO_PIN_2      (1<<PINPOSITION2)
#define GPIO_PIN_3      (1<<PINPOSITION3)
#define GPIO_PIN_4      (1<<PINPOSITION4)
#define GPIO_PIN_5      (1<<PINPOSITION5)
#define GPIO_PIN_6      (1<<PINPOSITION6)
#define GPIO_PIN_7      (1<<PINPOSITION7)
#define GPIO_PIN_8      (1<<PINPOSITION8)
#define GPIO_PIN_9      (1<<PINPOSITION9)
#define GPIO_PIN_10     (1<<PINPOSITION10)
#define GPIO_PIN_11     (1<<PINPOSITION11)
#define GPIO_PIN_12     (1<<PINPOSITION12)
#define GPIO_PIN_13     (1<<PINPOSITION13)
#define GPIO_PIN_14     (1<<PINPOSITION14)
#define GPIO_PIN_15     (1<<PINPOSITION15)
#define GPIO_PIN_16     (1<<PINPOSITION16)
#define GPIO_PIN_17     (1<<PINPOSITION17)
#define GPIO_PIN_18     (1<<PINPOSITION18)
#define GPIO_PIN_19     (1<<PINPOSITION19)
#define GPIO_PIN_20     (1<<PINPOSITION20)
#define GPIO_PIN_21     (1<<PINPOSITION21)
#define GPIO_PIN_22     (1<<PINPOSITION22)
#define GPIO_PIN_23     (1<<PINPOSITION23)
#define GPIO_PIN_24     (1<<PINPOSITION24)
#define GPIO_PIN_25     (1<<PINPOSITION25)
#define GPIO_PIN_26     (1<<PINPOSITION26)
#define GPIO_PIN_27     (1<<PINPOSITION27)
#define GPIO_PIN_28     (1<<PINPOSITION28)
#define GPIO_PIN_29     (1<<PINPOSITION29)
#define GPIO_PIN_30     (1<<PINPOSITION30)
#define GPIO_PIN_31     (1<<PINPOSITION31)

#define ARDUINO_GPIO_PORT   48

/*
 *  GPIOモードパラメータ
 */
#define GPIO_MODE_INPUT     0x00000000	/* Input Floating Mode */
#define GPIO_MODE_OUTPUT    0x00000001	/* Output Mode */

/*
 *  GPIO-割込みモードパラメータ
 */
#define GPIO_MODE_IT_RISING             0x00110000	/* Interrupt Mode with Rising edge trigger detection */
#define GPIO_MODE_IT_FALLING            0x00210000	/* Interrupt Mode with Falling edge trigger detection */
#define GPIO_MODE_IT_HIGH               0x00410000	/* Interrupt Mode High Level */
#define GPIO_MODE_IT_LOW                0x00810000	/* Interrupt Mode Low Level */

/*
 *  GPIOプルアップダウンパラメータ
 */
#define GPIO_NOPULL         0x00000000	/* No Pull-up or Pull-down activation  */
#define GPIO_PULLUP         0x00000001	/* Pull-up activation                  */
#define GPIO_PULLDOWN       0x00000002	/* Pull-down activation                */
#define GPIO_PULLMAX        (GPIO_PULLDOWN+1)

/*
 *  GPIO初期化設定
 */
typedef struct
{
    uint32_t    mode;		/* specifies the operating mode for the selected pins. */
	uint32_t    pull;		/* specifies the Pull-up or Pull-Down */
}GPIO_Init_t;

extern void gpio_setup(unsigned long base, GPIO_Init_t *init, uint32_t pin);
extern void gpio_set_pin(unsigned long base, uint8_t pin, uint8_t value);
extern uint8_t gpio_get_pin(unsigned long base, uint8_t pin);
extern int  gpio_get_gpiohno(uint8_t fpio_pin, bool_t intreq);


/*
 *  DMAチャンネル定義
 */
#define DMA_CHANNEL0         0
#define DMA_CHANNEL1         1
#define DMA_CHANNEL2         2
#define DMA_CHANNEL3         3
#define DMA_CHANNEL4         4
#define DMA_CHANNEL5         5
#define NUM_DMA_CHANNEL      (NUM_DMAC_CHANNEL)

/*
 *  DMA選択定義
 */
#define DMA_SELECT_SSI0_RX_REQ  0
#define DMA_SELECT_SSI0_TX_REQ  1
#define DMA_SELECT_SSI1_RX_REQ  2
#define DMA_SELECT_SSI1_TX_REQ  3
#define DMA_SELECT_SSI2_RX_REQ  4
#define DMA_SELECT_SSI2_TX_REQ  5
#define DMA_SELECT_SSI3_RX_REQ  6
#define DMA_SELECT_SSI3_TX_REQ  7
#define DMA_SELECT_I2C0_RX_REQ  8
#define DMA_SELECT_I2C0_TX_REQ  9
#define DMA_SELECT_I2C1_RX_REQ  10
#define DMA_SELECT_I2C1_TX_REQ  11
#define DMA_SELECT_I2C2_RX_REQ  12
#define DMA_SELECT_I2C2_TX_REQ  13
#define DMA_SELECT_UART1_RX_REQ 14
#define DMA_SELECT_UART1_TX_REQ 15
#define DMA_SELECT_UART2_RX_REQ 16
#define DMA_SELECT_UART2_TX_REQ 17
#define DMA_SELECT_UART3_RX_REQ 18
#define DMA_SELECT_UART3_TX_REQ 19
#define DMA_SELECT_AES_REQ      20
#define DMA_SELECT_SHA_RX_REQ   21
#define DMA_SELECT_AI_RX_REQ    22
#define DMA_SELECT_FFT_RX_REQ   23
#define DMA_SELECT_FFT_TX_REQ   24
#define DMA_SELECT_I2S0_TX_REQ  25
#define DMA_SELECT_I2S0_RX_REQ  26
#define DMA_SELECT_I2S1_TX_REQ  27
#define DMA_SELECT_I2S1_RX_REQ  28
#define DMA_SELECT_I2S2_TX_REQ  29
#define DMA_SELECT_I2S2_RX_REQ  30
#define DMA_SELECT_I2S0_BF_DIR_REQ    31
#define DMA_SELECT_I2S0_BF_VOICE_REQ  32
#define NUM_DMA_SELECT          33

/*
 *  DMAステータス定義
 */
#define DMA_STATUS_BUSY         0x0001		/* BUSY */
#define DMA_STATUS_READY_HMEM0  0x0002		/* DMA Mem0 Half process success */
#define DMA_STATUS_READY_HMEM1  0x0004		/* DMA Mem1 Half process success */
#define DMA_STATUS_READY_MEM0   0x0008		/* DMA Mem0 process success      */
#define DMA_STATUS_READY_ERROR  0x0100		/* DMA Error end */

/*
 *  DMA転送方向定義
 */
#define DMA_MEMORY_TO_MEMORY    0x0000		/* Memory to memory direction(DMAC)     */
#define DMA_MEMORY_TO_PERIPH    0x0001		/* Memory to peripheral direction(DMAC) */
#define DMA_PERIPH_TO_MEMORY    0x0002		/* Peripheral to memory direction(DMAC) */
#define DMA_PERIPH_TO_PERIPH    0x0003		/* Peripheral to peripheral direction(DMAC) */
#define PRF_PERIPH_TO_MEMORY1   0x0004		/* Peripheral to memory direction(PRF-SRC) */
#define PRF_PERIPH_TO_PERIPH    0x0005		/* Peripheral to peripheral direction(PRF) */
#define PRF_MEMORY_TO_PERIPH    0x0006		/* Memory to peripheral direction(PRF) */
#define PRF_PERIPH_TO_MEMORY2   0x0007		/* Peripheral to memory direction(PRF-DST) */

/*
 *  DMAマルチブロックタイプ
 */
#define DMAC_MULTBLOCK_CONT     0x0000		/* Continuous multiblock type */
#define DMAC_MULTBLOCK_RELOAD   0x0001		/* Reload multiblock type */
#define DMAC_MULTBLOCK_SHADOW   0x0002		/* Shadow register based multiblock type */
#define DMAC_MULTBLOCK_LINKED   0x0003		/* Linked lisr bases multiblock type */

/*
 *  DMAソフトウェア-ハードウェアハンドシェイク
 */
#define DMAC_HS_HARDWARE        0x0000		/* Hardware handshaking */
#define DMAC_HS_SOFTWARE        0x0001		/* Software handshaking */

/*
 *  DMAハードウェアハンドシェイク極性
 */
#define DMAC_HWHS_POLARITY_LOW  0x0000		/* polarity low */
#define DMAC_HWHS_POLARITY_HIGH 0x00001		/* polarity high */

/*
 *  DMA優先度定義
 */
#define DMAC_PRIORITY_0         0x0000
#define DMAC_PRIORITY_1         0x0001
#define DMAC_PRIORITY_2         0x0002
#define DMAC_PRIORITY_3         0x0003
#define DMAC_PRIORITY_4         0x0004
#define DMAC_PRIORITY_5         0x0005
#define DMAC_PRIORITY_6         0x0006
#define DMAC_PRIORITY_7         0x0007


/*
 *  DMAマスター定義
 */
#define DMAC_MASTER1            0x0000		/* Master #1 */
#define DMAC_MASTER2            0x0001		/* Master #2 */

/*
 *  DMAインクリメントモード
 */
#define DMAC_ADDR_INCREMENT     0x0000		/* インクリメント */
#define DMAC_ADDR_NOCHANGE      0x0001		/* 未変更 */

/*
 *  DMA転送幅定義
 */
#define DMAC_TRANS_WIDTH_8      0x0000		/* 8ビット */
#define DMAC_TRANS_WIDTH_16     0x0001		/* 16ビット */
#define DMAC_TRANS_WIDTH_32     0x0002		/* 32ビット */
#define DMAC_TRANS_WIDTH_64     0x0003		/* 64ビット */
#define DMAC_TRANS_WIDTH_128    0x0004		/* 128ビット */
#define DMAC_TRANS_WIDTH_256    0x0005		/* 256ビット */

/*
 *  DMAブースト転送サイズ定義
 */
#define DMAC_MSIZE_1            0x0000		/* 1バイト */
#define DMAC_MSIZE_4            0x0001		/* 4バイト */
#define DMAC_MSIZE_8            0x0002		/* 8バイト */
#define DMAC_MSIZE_16           0x0003		/* 16バイト */
#define DMAC_MSIZE_32           0x0004		/* 32バイト */
#define DMAC_MSIZE_64           0x0005		/* 64バイト */
#define DMAC_MSIZE_128          0x0006		/* 128バイト */
#define DMAC_MSIZE_256          0x0007		/* 256バイト */

#define DMA_TRS_TIMEOUT         2000	/* 2秒 */

/*
 *  DMA初期化構造体定義
 */
typedef struct
{
	uint16_t              Request;		/* DMA選択 */
	uint16_t              Direction;	/* DMA転送方向 */
	uint16_t              SrcMultBlock;	/* ソースマルチブロックタイプ */
	uint16_t              DrcMultBlock;	/* デスティネーションマルチブロックタイプ */
	uint16_t              SrcHandShake;	/* ソースハンドシェイク */
	uint16_t              DrcHandShake;	/* デスティネーションハンドシェイク */
	uint16_t              SrcHwhsPol;	/* ソースハードウェアハンドシェイク極性 */
	uint16_t              DrcHwhsPol;	/* デスティネーションハードウェアハンドシェイク極性 */
	uint16_t              Priority;		/* 優先度 */
	uint16_t              SrcMaster;	/* ソースマスター設定 */
	uint16_t              DstMaster;	/* デスティネーションマスター設定 */
	uint16_t              SrcInc;		/* ソースインクリメント設定 */
	uint16_t              DstInc;		/* デスティネーションインクリメント設定 */
	uint16_t              SrcTransWidth;/* ソース転送幅 */
	uint16_t              DstTransWidth;/* デスティネーション転送幅 */
	uint16_t              SrcBurstSize;	/* ソースバーストサイズ */
	uint16_t              DstBurstSize;	/* デスティネーションバーストサイズ */
	uint16_t              IocBlkTrans;	/* IOCブロック転送 */
}DMA_Init_t;

/*
 *  DMAハンドラ構造体定義
 */
typedef struct __DMA_Handle_t DMA_Handle_t;
struct __DMA_Handle_t
{
	unsigned long         base;			/* DMA Port base address */
	unsigned long         cbase;		/* DMA Channel Port address */
	DMA_Init_t            Init;			/* DMA communication parameters */
	uint16_t              chnum;		/* channel number */
	volatile uint16_t     status;		/* DMA status */
	volatile uint16_t     ErrorCode;	/* DMA Error code */
	uint16_t              dummy;
	void                  (*xfercallback)(DMA_Handle_t * hdma);		/* DMA transfer complete callback */
	void                  (*errorcallback)(DMA_Handle_t * hdma);	/* DMA transfer error callback */
	void                  *localdata;	/* DMA local data */
};

/*
 *  DMAステータス定義
 */
#define DMA_STATUS_READY        0x0000	/* READY */
#define DMA_STATUS_BUSY         0x0001	/* BUSY */
#define DMA_STATUS_READY_TRN1   0x0008	/* DMA Mem process success      */
#define DMA_STATUS_READY_TRN2   0x0010	/* DMA SRC process success      */
#define DMA_STATUS_READY_TRN3   0x0020	/* DMA DST process success      */
#define DMA_STATUS_READY_ERROR  0x0100	/* DMA Error end */

/*
 *  DMAエラー定義
 */ 
#define DMA_ERROR_NONE          0x0000	/* No error */
#define DMA_ERROR_SRC_DODE      0x0020	/* Source Decode error */
#define DMA_ERROR_DST_DECODE    0x0040	/* Destination Decode error */
#define DMA_ERROR_SRC_SLV       0x0080	/* Source Slave Error Status Enable */
#define DMA_ERROR_DST_SLV       0x0100	/* Destination Slave Error Status Enable */
#define DMA_ERROR_LLI_RD_DEC    0x0200	/* LLI Read Decode Error Status */
#define DMA_ERROR_LLI_WR_DEC    0x0400	/* LLI Write Decode Error Status */
#define DMA_ERROR_LLI_RD_SLV    0x0800	/* LLI Read Slave Error Status Enable */
#define DMA_ERROR_LLI_WR_SLV    0x1000	/* LLI Write Slave Error Status Enable */
#define DMA_ERROR_TIMEOUT       0x0001	/* Timeout error */

extern ER dma_init(DMA_Handle_t *hdma);
extern ER dma_deinit(DMA_Handle_t *hdma);
extern ER dma_reset(DMA_Handle_t *hdma);
extern ER dma_start(DMA_Handle_t *hdma, uintptr_t SrcAddress, uintptr_t DstAddress, uint32_t DataLength);
extern ER dma_end(DMA_Handle_t *hdma);
extern void channel_dmac_isr(intptr_t exinf);
extern ER select_dma_channel(uint8_t channel, uint8_t select);


/*
 *  WDOGドライバ
 */
#define WDOG1_PORTID        1
#define WDOG2_PORTID        2
#define NUM_WDOGPORT        2

/*
 *  WDOGモード定義
 */
#define WDT_MODE_REST       WDT_CR_RMOD_RESET
#define WDT_MODE_INTERRUPT  WDT_CR_RMOD_INTERRUPT

/*
 *  WDOGハンドラ定義
 */
typedef struct __WDT_Handle_t WDT_Handle_t;
struct __WDT_Handle_t
{
	unsigned long       base;
	uint32_t            wdtno;
	uint32_t            pclk;
	void                (*callback)(WDT_Handle_t *hwdt);
};

#define INHNO_WDOG1   IRQ_VECTOR_WDT0	/* 割込みハンドラ番号 */
#define INTNO_WDOG1   IRQ_VECTOR_WDT0	/* 割込み番号 */
#define INHNO_WDOG2   IRQ_VECTOR_WDT1	/* 割込みハンドラ番号 */
#define INTNO_WDOG2   IRQ_VECTOR_WDT1	/* 割込み番号 */
#define INTPRI_WDOG   -1		/* 割込み優先度 */
#define INTATR_WDOG   0			/* 割込み属性 */

extern WDT_Handle_t *wdt_init(ID port);
extern ER wdt_deinit(WDT_Handle_t *hwdt);
extern ER_UINT wdt_start(WDT_Handle_t *hwdt, uint32_t mode, uint32_t timeout_ms);
extern ER wdt_stop(WDT_Handle_t *hwdt);
extern void wdog_isr(intptr_t exinf);


/*
 *  RTCドライバ
 */

/*
 *  RTCモード定義
 */
#define RTC_TIMER_PAUSE         0	/* Timer pause */
#define RTC_TIMER_RUNNING       1	/* Timer time running */
#define RTC_TIMER_SETTING       2	/* Timer time setting */
#define RTC_TIMER_MAX           3	/* Max count of this enum*/

/*
 *  RTC TICK割込みモード
 */
#define RTC_INT_SECOND          0	/* 秒単位 */
#define RTC_INT_MINUTE          1	/* 分単位 */
#define RTC_INT_HOUR            2	/* 時間単位 */
#define RTC_INT_DAY             3	/* 日単位 */

/*
 *  RTC ALARMマスク
 */
#define RTC_ALARM_SECOND        0x02000000	/* 秒マスク */
#define RTC_ALARM_MINUTE        0x04000000	/* 分マスク */
#define RTC_ALARM_HOUR          0x08000000	/* 時マスク */
#define RTC_ALARM_WEEK          0x10000000	/* 週マスク */
#define RTC_ALARM_DAY           0x20000000	/* 日マスク */
#define RTC_ALARM_MONTH         0x40000000	/* 月マスク */
#define RTC_ALARM_YEAR          0x80000000	/* 年マスク */

/*
 *  RTC割込み設定
 */
#define INHNO_RTC     IRQ_VECTOR_RTC	/* 割込みハンドラ番号 */
#define INTNO_RTC     IRQ_VECTOR_RTC	/* 割込み番号 */
#define INTPRI_RTC    -7		/* 割込み優先度 */
#define INTATR_RTC    0			/* 割込み属性 */

/*
 *  日時設定用の構造体を定義
 */
struct tm2 {
  int	tm_sec;			/* 秒 */
  int	tm_min;			/* 分 */
  int	tm_hour;		/* 時 */
  int	tm_mday;		/* 月中の日 */
  int	tm_mon;			/* 月 */
  int	tm_year;		/* 年 */
  int	tm_wday;		/* 曜日 */
  int	tm_yday;		/* 年中の日 */
  int	tm_isdst;
};

/*
 *  RTCアラーム構造体
 */
typedef struct
{
	uint32_t            alarmmask;			/* アラームマスク設定 */
	void                (*callback)(int);	/* アラームコールバック */
}RTC_Alarm_t;

extern void rtc_init(intptr_t exinf);
extern void rtc_info_init(intptr_t exinf);
extern ER rtc_timer_set_mode(uint8_t timer_mode);
extern ER rtc_set_time(struct tm2 *pt);
extern ER rtc_get_time(struct tm2 *pt);
extern ER_UINT rtc_intmode(int8_t mode, void *func);
extern ER rtc_setalarm(RTC_Alarm_t *parm, struct tm2 *ptm);
extern void rtc_int_handler(void);

extern uint8_t rtc_timer_mode;

/*
 *  デバイス初期化
 */
extern void device_init(intptr_t exinf);

/*
 *  SPI0/DVPのモード設定
 */
extern ER select_spi0_dvp_mode(uint8_t en);


/*
 *  FPIOAにファンクションを設定する
 *  param1  number      The IO number
 *  param2  function    The function enum number
 *  return  0-OK
 */
extern ER fpioa_set_function(int number, uint8_t function);


/*
 *  PLLクロック取得
 */
extern uint32_t get_pll_clock(uint8_t no);



#ifdef __cplusplus
}
#endif

#endif

