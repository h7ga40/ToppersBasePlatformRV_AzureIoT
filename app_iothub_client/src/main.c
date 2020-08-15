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
 *  ESP-WROOM-02テストの本体
 *  シールド上のプッシュスイッチがESP-WROOM-02の
 *  リセットスイッチです．リセット後、ATコマンドを使って、
 *  ESP-WROOM-02と通信ができます。
 *  例えば、AT+GMRは、AT GMR(return)でESP-WROOM-02に送信され
 *  ESP-WROOM-02からの受信データはコンソールに表示されます．
 */

#include <stddef.h>
#include <stdbool.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <target_syssvc.h>
#include <stdio.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "device.h"
#include "monitor.h"
#include "main.h"
#include "esp_at_socket.h"

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

/*static*/ uint32_t heap_area[256*1024];

intptr_t heap_param[2] = {
	(intptr_t)heap_area,
	sizeof(heap_area)
};

static char    aTxBuffer[TX_BUF_SIZE];
static uint8_t aRxBuffer[RX_BUF_SIZE];

int     rx_mode = MODE_DEFAULT;
queue_t rx_queue;

/*
 *  デバイスコマンド番号
 */
#define BASE_CMD_LEN     4

static int_t mode_func(int argc, char **argv);
static int_t count_func(int argc, char **argv);
static int_t at_func(int argc, char **argv);

/*
 *  デバイスコマンドテーブル
 */
static const COMMAND_INFO at_command_info[] = {
	{"MODE",	mode_func},
	{"TCNT",	count_func},
	{"",		at_func}
};

#define NUM_DEVICE_CMD   (sizeof(at_command_info)/sizeof(COMMAND_INFO))

static const char at_name[] = "AT";
static const char at_help[] =
"  AT      MODE (no) echo mode\n"
"          TCNT      recived count\n"
"          any command\n";

static COMMAND_LINK at_command_link = {
	NULL,
	NUM_DEVICE_CMD,
	at_name,
	NULL,
	at_help,
	&at_command_info[0]
};

static int a2i(char *str)
{
	int num = 0;

	while(*str >= '0' && *str <= '9'){
		num = num * 10 + *str++ - '0';
	}
	return num;
}

/*
 *  ATコマンド設定関数
 */
void at_info_init(intptr_t exinf)
{
	setup_command(&at_command_link);
}

/*
 *  ECHOモード設定関数
 */
static int_t mode_func(int argc, char **argv)
{
	int mode = rx_mode;

	if(argc >= 2){
		mode = a2i(argv[1]);
		if(mode < 0 || mode > MODE_ECHO_HEX)
			mode = MODE_DEFAULT;
		rx_mode = mode;
	}
	printf("AT ECHO MODE(%d)\n", mode);
	return mode;
}

/*
 *  受信カウントコマンド取得関数
 */
static int_t count_func(int argc, char **argv)
{
	printf("AT RECIVED COUNT(%d)(%d)\n", rx_queue.size, rx_queue.clen);
	return rx_queue.clen;
}


/*
 *  AT設定コマンド関数
 */
static int_t at_func(int argc, char **argv)
{
	int  i, arg_count = BASE_CMD_LEN;
	char *p = aTxBuffer;
	char *s, c;
	ER_UINT result;
	int  caps = 1;

	for(i = 0 ; i < argc ; i++){
		arg_count++;
		arg_count += strlen(argv[i]);
	}
	*p++ = 'A';
	*p++ = 'T';
	if(arg_count > BASE_CMD_LEN){
		for(i = 0 ; i < argc ; i++){
			s = argv[i];
			*p++ = '+';
			while(*s != 0){
				c = *s++;
				if(c == '"')
					caps ^= 1;
				if(caps && c >= 'a' && c <= 'z')
					c -= 0x20;
				*p++ = c;
			}
		}
	}
	*p++ = '\r';
	*p++ = '\n';
	*p++ = 0;
	result = serial_wri_dat(AT_PORTID, (const char *)aTxBuffer, arg_count);
	if(result < 0){
		syslog_1(LOG_ERROR, "AT command send error(%d) !", result);
	}
	printf("%d:%s", arg_count, aTxBuffer);
	return arg_count;
}

int wolfSSL_Debugging_ON(void);
int esp_at_rx_handler(void *data, int len);

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	T_SERIAL_RPOR k_rpor;
	queue_t *rxque;
	ER_UINT	ercd;
	int i, j, len;
	int dlen = 0;
	uint8_t ch;

	//wolfSSL_Debugging_ON();
	init_esp_at();

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

	/*
	 * キューバッファの初期化
	 */
	rxque = &rx_queue;
	rxque->size = RX_BUF_SIZE;
	rxque->clen  = 0;
	rxque->head = 0;
	rxque->tail = 0;
	rxque->pbuffer = aRxBuffer;

	/*
	 *  ESP-WROOM02用シリアルポートの初期化
	 */
	ercd = serial_opn_por(AT_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.(AT)",
									itron_strerror(ercd), SERCD(ercd));
		slp_tsk();
	}
	SVC_PERROR(serial_ctl_por(AT_PORTID, 0));
	while(dlen < 500000){
		serial_ref_por(AT_PORTID, &k_rpor);
		len = k_rpor.reacnt;
		if((rxque->size - rxque->clen) < len)
			len = rxque->size - rxque->clen;
		if(len > 0){
			if(rxque->head >= rxque->tail){
				if((rxque->size - rxque->head) < len)
					i = rxque->size - rxque->head;
				else
					i = len;
				j = serial_rea_dat(AT_PORTID, (char *)&rxque->pbuffer[rxque->head], i);
				rxque->head += j;
				if(rxque->head >= rxque->size)
					rxque->head -= rxque->size;
				rxque->clen += j;
				len -= j;
			}
			if(len > 0){
				j = serial_rea_dat(AT_PORTID, (char *)&rxque->pbuffer[rxque->head], len);
				rxque->head += j;
				if(rxque->head >= rxque->size)
					rxque->head -= rxque->size;
				rxque->clen += j;
				len -= j;
			}
		}
		while(rxque->clen > 0 && rx_mode != MODE_NOECHO){
			if(rx_mode == MODE_ECHO_CHAR){
				ch = rxque->pbuffer[rxque->tail];
				if(ch >= 0x7f)
					ch = '.';
				else if(ch < 0x20 && ch != '\r' && ch != '\n')
					ch = '.';
				putchar(ch);
			}
			else
				printf("%02x ", rxque->pbuffer[rxque->tail]);
			rxque->clen--;
			rxque->tail++;
			if(rxque->tail >= rxque->size)
				rxque->tail -= rxque->size;
			dlen++;
			if((dlen % 32) == 0 && rx_mode == MODE_ECHO_HEX){
				printf("\n");
				dly_tsk(50);
			}
		}
		if (rxque->clen > 0) {
			len = rxque->clen;
			if (rxque->tail >= rxque->head) {
				if ((rxque->size - rxque->tail) < len)
					i = rxque->size - rxque->tail;
				else
					i = len;
				j = esp_at_rx_handler((char *)&rxque->pbuffer[rxque->tail], i);
				rxque->tail += j;
				if (rxque->tail >= rxque->size)
					rxque->tail -= rxque->size;
				rxque->clen -= j;
				len -= j;
			}
			if (len > 0) {
				j = esp_at_rx_handler((char *)&rxque->pbuffer[rxque->tail], len);
				rxque->tail += j;
				if (rxque->tail >= rxque->size)
					rxque->tail -= rxque->size;
				rxque->clen -= j;
				len -= j;
			}
		}
		else {
			dly_tsk(20);
		}
	}
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();

	syslog(LOG_NOTICE, "Sample program ends.");
//	SVC_PERROR(ext_ker());
}

int EmbedReceive(struct WOLFSSL *ssl, char *buf, int sz, void *_ctx)
{
	syslog(LOG_NOTICE, "EmbedReceive");
	return 0;
}

int EmbedSend(struct WOLFSSL* ssl, char* buf, int sz, void* _ctx)
{
	syslog(LOG_NOTICE, "EmbedSend");
	return 0;
}
