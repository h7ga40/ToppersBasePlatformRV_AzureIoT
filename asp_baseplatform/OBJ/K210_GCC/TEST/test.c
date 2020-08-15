/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  $Id$
 */

/* 
 *  D13にUSER-SWを設定した場合、設定により割込みを起動する
 *  WDTに割込みモードのWDT設定を行う
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "target_syssvc.h"
#include "kernel_cfg.h"
#include "device.h"
#include "pinmode.h"
#include "test.h"

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

#define SW_PIN   13


static void
pinModex(uint8_t dwPin, uint8_t dwMode){ 
    int gpionumx = gpio_get_gpiohno(getGpioPin(dwPin), true);
	GPIO_Init_t init = {0};
	int gpionum;

	gpionum = gpionumx;
	syslog_4(LOG_NOTICE, "## dwPin(%d) MD_PIN_MAP(%d) gpionum(%d) gpionumx(%d)  ##", dwPin, getGpioPin(dwPin), gpionum, gpionumx);
    if(gpionum >= 0){
        uint8_t function = FUNC_GPIOHS0 + gpionum;
        fpioa_set_function(getGpioPin(dwPin), function);
		switch(dwMode){
		case INPUT:
			init.mode = GPIO_MODE_INPUT;
//			init.mode = GPIO_MODE_IT_RISING;
			init.pull = GPIO_NOPULL;
			break;
		case INPUT_PULLDOWN:
			init.mode = GPIO_MODE_INPUT;
			init.pull = GPIO_PULLDOWN;
			break;
		case INPUT_PULLUP:
			init.mode = GPIO_MODE_INPUT;
			init.pull = GPIO_PULLUP;
			break;
		case OUTPUT:
		default:
			init.mode = GPIO_MODE_OUTPUT;
			init.pull = GPIO_PULLDOWN;
			break;
		}
        gpio_setup(TADR_GPIOHS_BASE, &init, (uint8_t)gpionum);
    }
    return ;
}

/*
 *  WDT割込みコールバック
 */
static void
wdt_int_handler(WDT_Handle_t *hwdt)
{
	syslog_1(LOG_NOTICE, "wdt_int_handler[%x] !", hwdt);
}

/*
 *  USER-SW割込みハンドラ
 */
void
sw_int_handler(void)
{
	uint32_t ipr = sil_rew_mem((uint32_t *)(TADR_GPIOHS_BASE+TOFF_GPIOHS_RISE_IP));
	uint32_t ipf = sil_rew_mem((uint32_t *)(TADR_GPIOHS_BASE+TOFF_GPIOHS_FALL_IP));
	syslog_2(LOG_EMERG, "## sw_int_handler r[%08x] f(%08x] ##", ipr, ipf);
	sil_wrw_mem((uint32_t *)(TADR_GPIOHS_BASE+TOFF_GPIOHS_RISE_IP), ipr);
}

/*
 *  メインタスク
 */
void
main_task(intptr_t exinf)
{
	WDT_Handle_t *hwdt;
	ER_UINT	ercd;
	int     i;
	uint32_t cnt;

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	pinModex(SW_PIN, INPUT);
	hwdt = wdt_init(WDOG1_PORTID);
	hwdt->callback = wdt_int_handler;
	cnt = wdt_start(hwdt, WDT_MODE_INTERRUPT, 10000);
	syslog_1(LOG_NOTICE, "## cnt(%d) ##", cnt);
	for(i = 0 ; i < 500000 ; i++){
		syslog_1(LOG_NOTICE, "## sw(%d) ##", digitalRead(SW_PIN));
		dly_tsk(1000);
		if(i == 15){
			wdt_deinit(hwdt);
			syslog_0(LOG_NOTICE, "WDT STOP");
		}
	}
	slp_tsk();

	syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ext_ker());
	assert(0);
}
