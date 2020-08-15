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
 *  K210 DVPデバイスドライバの外部宣言
 *
 */

#ifndef _DVP_H_
#define _DVP_H_

#include <kernel.h>
#include <target_syssvc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TADR_DVP_BASE       (DVP_BASE_ADDR)
#define TOFF_DVP_CFG            0x0000	/* (RW) DVP Config Register */
  #define DVP_CFG_START_INT_ENABLE      0x00000001
  #define DVP_CFG_FINISH_INT_ENABLE     0x00000002
  #define DVP_CFG_AI_OUTPUT_ENABLE      0x00000004
  #define DVP_CFG_DISPLAY_OUTPUT_ENABLE 0x00000008
  #define DVP_CFG_AUTO_ENABLE           0x00000010
  #define DVP_CFG_BURST_SIZE_4BEATS     0x00000100
  #define DVP_CFG_FORMAT_MASK           0x00000600
  #define DVP_CFG_RGB_FORMAT            0x00000000
  #define DVP_CFG_YUV_FORMAT            0x00000200
  #define DVP_CFG_Y_FORMAT              0x00000600
  #define DVP_CFG_HREF_BURST_NUM_MASK   0x000FF000
  #define DVP_CFG_LINE_NUM_MASK         0x3FF00000
#define TOFF_DVP_R_ADDR         0x0004	/* (RW) RED ADDRESS */
#define TOFF_DVP_G_ADDR         0x0008	/* (RW) GREEN ADDRESS */
#define TOFF_DVP_B_ADDR         0x000C	/* (RW) BLUE ADDRESS */
#define TOFF_DVP_CMOS_CFG       0x0010	/* (RW) DVP CMOS Config Register */
  #define DVP_CMOS_CLK_DIV_MASK         0x000000FF
  #define DVP_CMOS_CLK_DIV(x)           ((x) << 0)
  #define DVP_CMOS_CLK_ENABLE           0x00000100
  #define DVP_CMOS_RESET                0x00010000
  #define DVP_CMOS_POWER_DOWN           0x01000000
#define TOFF_DVP_SCCB_CFG       0x0014	/* (RW) DVP SCCB Config Register */
  #define DVP_SCCB_BYTE_NUM_MASK        0x00000003
  #define DVP_SCCB_BYTE_NUM_2           0x00000001
  #define DVP_SCCB_BYTE_NUM_3           0x00000002
  #define DVP_SCCB_BYTE_NUM_4           0x00000003
  #define DVP_SCCB_SCL_LCNT_MASK        0x0000FF00
  #define DVP_SCCB_SCL_HCNT_MASK        0x00FF0000
  #define DVP_SCCB_RDATA_BYTE_MASK      0xFF000000
#define TOFF_DVP_SCCB_CTL       0x0018	/* (RW) DVP SCCB Control Register */
  #define DVP_SCCB_WRITE_DATA_ENABLE    0x00000001
  #define DVP_SCCB_DEVICE_ADDRESS       0x000000FF
  #define DVP_SCCB_REG_ADDRESS          0x0000FF00
  #define DVP_SCCB_WDATA_BYTE0          0x00FF0000
  #define DVP_SCCB_WDATA_BYTE1          0xFF000000
#define TOFF_DVP_AXI            0x001C	/* (RW) DVP AXI Register */
  #define DVP_AXI_GM_MLEN_MASK          0x000000FF
  #define DVP_AXI_GM_MLEN_1BYTE         0x00000000
  #define DVP_AXI_GM_MLEN_4BYTE         0x00000003
#define TOFF_DVP_STS            0x0020	/* (RW) DVP STS Register */
  #define DVP_STS_FRAME_START           0x00000001
  #define DVP_STS_FRAME_START_WE        0x00000002
  #define DVP_STS_FRAME_FINISH          0x00000100
  #define DVP_STS_FRAME_FINISH_WE       0x00000200
  #define DVP_STS_DVP_EN                0x00010000
  #define DVP_STS_DVP_EN_WE             0x00020000
  #define DVP_STS_SCCB_EN               0x01000000
  #define DVP_STS_SCCB_EN_WE            0x02000000
#define TOFF_DVP_RGB_ADDR       0x0028	/* (RW) DVP RGB ADDRESS */


/*
 *  BURSTモード設定
 */
#define DVP_BURST_ENABLE        DVP_CFG_BURST_SIZE_4BEATS
#define DVP_BURST_DISABLE       0

/*
 *  AUTOモード設定
 */
#define DVP_AUTOMODE_ENABLE     DVP_CFG_AUTO_ENABLE
#define DVP_AUTOMODE_DISABLE    0

/*
 *  DVPイメージフォーマット設定
 */
#define DVP_FORMAT_RGB          DVP_CFG_RGB_FORMAT
#define DVP_FORMAT_YUY          DVP_CFG_YUV_FORMAT
#define DVP_FORMAT_Y            DVP_CFG_Y_FORMAT

/*
 *  DVP状態定義
 */
#define DVP_STATE_INIT          0
#define DVP_STATE_READY         1
#define DVP_STATE_ACTIVATE      2
#define DVP_STATE_STARTED       3
#define DVP_STATE_FINISH        4


#define INHNO_DVP     IRQ_VECTOR_DVP	/* 割込みハンドラ番号 */
#define INTNO_DVP     IRQ_VECTOR_DVP	/* 割込み番号 */
#define INTPRI_DVP    -3		/* 割込み優先度 */
#define INTATR_DVP    0			/* 割込み属性 */

#ifndef TOPPERS_MACRO_ONLY

/*
 *  DVP 設定初期設定構造体
 */
typedef struct
{
	uint32_t              Freq;				/* DVP 周波数設定 */
	uint32_t              RedAddr;			/* DVP Readアドレス設定 */
	uint32_t              GreenAddr;		/* DVP Greenアドレス設定 */
    uint32_t              BlueAddr;			/* DVP Blueアドレス設定 */
	uint32_t              RGBAddr;			/* DVP RGBアドレス設定 */

	uint32_t              Format;			/* DVP イメージフォーマット */
	uint32_t              BurstMode;		/* DVP ブーストモード設定 */
	uint32_t              AutoMode;			/* DVP オートモード設定 */
	uint32_t              GMMlen;			/* DVP GM MLEN設定 */
	uint16_t              Width;			/* DVP 幅設定 */
	uint16_t              Height;			/* DVP 高さ設定 */
	uint8_t               num_sccb_reg;		/* DVP SCCBレジスタ長 */
	int8_t                CMosPClkPin;		/* DVP PCLKピン設定 */
	int8_t                CMosXClkPin;		/* DVP XCLKピン設定 */
	int8_t                CMosHRefPin;		/* DVP HREFピン設定 */
	int8_t                CMosPwDnPin;		/* DVP パワーダウン設定 */
	int8_t                CMosVSyncPin;		/* DVP VSYNCピン設定 */
	int8_t                CMosRstPin;		/* DVP リセットピン設定 */
	int8_t                SccbSClkPin;		/* DVP SCCB SCLKピン設定 */
	int8_t                SccbSdaPin;		/* DVP SCCB SDAピン設定 */
	uint8_t               dummy;
	uint16_t              IntNo;			/* DVP 割込み番号 */
	int                   semlock;			/* SPI ロックセマフォ値 */
}DVP_Init_t;

/*
 *  SPIハンドラ
 */
typedef struct _DVP_Handle_t
{
	unsigned long         base;				/* DVP registers base address */
	DVP_Init_t            Init;				/* DVP communication parameters */
	ID                    semid;			/* DVP semaphore id */
	volatile uint16_t     state;			/* DVP state */
	volatile uint16_t     ErrorCode;		/* DVP Error code */
}DVP_Handle_t;


extern ER dvp_init(DVP_Handle_t *hdvp);
extern ER dvp_deinit(DVP_Handle_t *hdvp);
extern ER dvp_activate(DVP_Handle_t *hdvp, bool_t run);
extern ER dvp_set_image_format(DVP_Handle_t *hdvp);
extern ER dvp_set_image_size(DVP_Handle_t *hdvp);
extern uint32_t dvp_sccb_set_clk_rate(DVP_Handle_t *hdvp, uint32_t clk_rate);
extern ER dvp_dcmi_reset(DVP_Handle_t *hdvp, bool_t reset);
extern ER dvp_dcmi_powerdown(DVP_Handle_t *hdvp, bool_t down);
extern void dvp_sccb_send_data(DVP_Handle_t *hdvp, uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data);
extern uint8_t dvp_sccb_receive_data(DVP_Handle_t *hdvp, uint8_t dev_addr, uint16_t reg_addr);
extern void dvp_handler(void);

#endif /* TOPPERS_MACRO_ONLY */

#ifdef __cplusplus
}
#endif

#endif	/* _DVP_H_ */

