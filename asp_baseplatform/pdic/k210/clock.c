/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2010 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2012-2015 by GT Development Center RICOH COMPANY,LTD. JAPAN
 *  Copyright (C) 2015-2019 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の (1)～(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
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
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id$
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#define _SYS__STDINT_H
#define clock_t int
#include <time.h>
#include "kernel_cfg.h"
#include "device.h"
#include "monitor.h"

#define RTC_CLOCK   32768	/* LSE_VALUE */

static int_t date_func(int argc, char **argv);
static int_t time_func(int argc, char **argv);
static int_t intmode_func(int argc, char **argv);
static int_t wakeup_func(int argc, char **argv);
static int_t clock_func(int argc, char **argv);

/*
 *  ＲＴＣコマンドテーブル
 */
static const COMMAND_INFO rtc_command_info[] = {
	{"DATE",	date_func    },	/* 日にち設定 */
	{"TIME",	time_func    },	/* 時間設定 */
	{"INTM",	intmode_func },	/* INTMODE設定 */
	{"WAKEUP",	wakeup_func  },	/* WAKEUP設定 */
	{"CLOCK",	clock_func   }	/* 時計 */
};

#define NUM_RTC_CMD     (sizeof(rtc_command_info)/sizeof(COMMAND_INFO))

static const char rtc_name[] = "RTC";
static const char rtc_help[] =
"  Rtc     DATE  (year #) (month #) (day #)\n"
"          TIME  (hour #) (min   #) (sec #)\n"
"          INTM  mode(0-4)\n"
"          WAKEUP time(sec)\n"
"          CLOCK DATE:TIME\n";

static const char monthday[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static COMMAND_LINK rtc_command_link = {
	NULL,
	NUM_RTC_CMD,
	rtc_name,
	NULL,
	rtc_help,
	&rtc_command_info[0]
};

/*
 *  ローカルatox
 */
static int atoix(char *s)
{
	int c, val = 0;

	while((c = *s++) != 0){
		if(c >= '0' && c <= '9')
			val = val * 10 + (c - '0');
	}
	return val;
}

/*
 *  TICK EVENT
 */
static void tick_event(int mode)
{
	syslog_1(LOG_NOTICE, "rtc tick event(%d) !", mode);
}

/*
 *  WAKUP EVENT
 */
static void wakeup_event(int reg)
{
	syslog_1(LOG_NOTICE, "rtc wakeup event[%08x] !", reg);
}
#ifndef NOT_USE_TBP_MKTIME_GMTIME
/*
 *  mktime関数
 */
time_t mktime(struct tm *ptm)
{
	int day = ptm->tm_year *365;
	int yday, i; 

	day += (ptm->tm_year+2) / 4;
	for(i = 1, yday = 0 ; i < ptm->tm_mon ; i++)
		yday += monthday[i-1];
	if(ptm->tm_mon > 2 && ((ptm->tm_year+2) % 4) == 0)
		yday++;
	yday += ptm->tm_mday - 1;
	day  += yday;
	ptm->tm_wday = (day+4) % 7;
	ptm->tm_yday = yday+1;
	syslog_3(LOG_INFO, "yday[%d] wday[%d] day[%d]", yday+1, ptm->tm_wday, ptm->tm_yday);
	return day * (24*60*60) + ptm->tm_hour * 60*60 + ptm->tm_min * 60 + ptm->tm_sec;
}

/*
 * gmtime_r関数
 */
struct tm *gmtime_r(const time_t *pt, struct tm *ptm)
{
	int day = *pt / (24*60*60);
	int time, i, yday, mday;

	time = *pt - day * 24 * 60 * 60;
	ptm->tm_wday = (day+4) % 7;
	for(i = 365, ptm->tm_year = 0 ; i < day ; i += 365){
		if(((ptm->tm_year+2) % 4) == 0)
			i++;
		ptm->tm_year++;
	}

	ptm->tm_yday = yday = day - i + 365 + 1;
	for(ptm->tm_mon = 1 ; ptm->tm_mon <= 12 && yday > 0 ; ptm->tm_mon++){
		mday = monthday[ptm->tm_mon-1];
		if(ptm->tm_mon == 2 && ((ptm->tm_year+2) % 4) == 0)
			mday++;
		if(yday < mday){
			ptm->tm_mday = yday;
			break;
		}
		yday -= mday;
	}
	ptm->tm_hour = time / (60*60);
	time -= ptm->tm_hour * 60 * 60;
	ptm->tm_min  = time / 60;
	ptm->tm_sec = time % 60;
	return ptm;
}
#endif
int
_gettimeofday (struct timeval * tp, void * tzvp)
{
	struct tm timedate;
	if (!tp) return 0;
	rtc_get_time((struct tm2 *)&timedate);
	uint64_t time = (uint64_t)mktime(&timedate) * (uint64_t)1000000;
	tp->tv_sec = time / 1000000;
	tp->tv_usec = time - (tp->tv_sec * 1000000);
	return 0;
}

/*
 *  RTCコマンド設定関数
 */
void rtc_info_init(intptr_t exinf)
{
	setup_command(&rtc_command_link);
}

/*
 *  日付設定コマンド関数
 */
static int_t date_func(int argc, char **argv)
{
	struct tm timedate;
	int arg1 = 2016;

	ER ret = rtc_get_time((struct tm2 *)&timedate);
	if (ret != E_OK){
		printf("rtc_get_time %s\n", itron_strerror(ret));
	}
	if(argc > 1)
		arg1 = atoix(argv[1]);
	if(argc > 2)
		timedate.tm_mon = atoix(argv[2]) - 1;
	else
		timedate.tm_mon = 0;
	if(argc > 3)
		timedate.tm_mday = atoix(argv[3]);
	else
		timedate.tm_mday = 1;
	printf("%04d/%02d/%02d\n", arg1, timedate.tm_mon + 1, timedate.tm_mday);
	timedate.tm_year = arg1-1900;
	timedate.tm_isdst = 0;
	mktime((struct tm *)&timedate);
	ret = rtc_set_time((struct tm2 *)&timedate);
	if (ret != E_OK){
		printf("rtc_set_time %s\n", itron_strerror(ret));
	}
	return 0;
}

/*
 *  時間設定コマンド関数
 */
static int_t time_func(int argc, char **argv)
{
	struct tm2 timedate;

	ER ret = rtc_get_time(&timedate);
	if (ret != E_OK){
		printf("rtc_get_time %s\n", itron_strerror(ret));
	}
	if(argc > 1)
		timedate.tm_hour = atoix(argv[1]);
	else
		timedate.tm_hour = 0;
	if(argc > 2)
		timedate.tm_min = atoix(argv[2]);
	else
		timedate.tm_min = 0;
	if(argc > 3)
		timedate.tm_sec = atoix(argv[3]);
	else
		timedate.tm_sec = 0;
	printf("%02d:%02d:%02d\n", timedate.tm_hour, timedate.tm_min, timedate.tm_sec);
	timedate.tm_isdst = 0;
	mktime((struct tm *)&timedate);
	ret = rtc_set_time(&timedate);
	if (ret != E_OK){
		printf("rtc_set_time %s\n", itron_strerror(ret));
	}
	return 0;
}

/*
 *  TICK割込み設定
 */
static int_t intmode_func(int argc, char **argv)
{
	int8_t mode = -1;
	if(argc > 1){
		mode = atoix(argv[1]);
		if(mode > 3)
			printf("reset intmode(%d)\n", rtc_intmode(mode, NULL));
		else
			printf("setup intmode(%d)\n", rtc_intmode(mode, tick_event));
	}
	else
		printf("current intmode(%d)\n", rtc_intmode(mode, NULL));
	return 0;
}

/*
 *  WAKEUPイベント設定関数
 */
static int_t wakeup_func(int argc, char **argv)
{
	uint32_t count = 0;
	struct tm2 timedate;
	RTC_Alarm_t alarm_param;

	rtc_get_time(&timedate);
	if(argc > 1){
		count = atoix(argv[1]);
		alarm_param.alarmmask = RTC_ALARM_SECOND | RTC_ALARM_MINUTE | RTC_ALARM_HOUR;
		alarm_param.callback  = wakeup_event;
		timedate.tm_sec += count;
		while(timedate.tm_sec >= 60){
			timedate.tm_sec -= 60;
			if(++timedate.tm_min >= 60){
				timedate.tm_min -= 60;
				if(++timedate.tm_hour >= 24)
					timedate.tm_hour = 0;
			}
		}
		rtc_setalarm(&alarm_param, &timedate);
		printf("wakeup %02d:%02d:%02d !\n", timedate.tm_hour, timedate.tm_min, timedate.tm_sec);
	}
	else{
		alarm_param.alarmmask = 0;
		rtc_setalarm(&alarm_param, &timedate);
		printf("stop wakeup event !\n");
	}
	return 0;
}

/*
 *  時刻取出しコマンド関数
 */
static int_t clock_func(int argc, char **argv)
{
	struct tm2 timedate;

	ER ret = rtc_get_time(&timedate);
	if (ret != E_OK){
		printf("rtc_get_time %s\n", itron_strerror(ret));
	}
	mktime((struct tm *)&timedate);
	printf("  %04d/%02d/%02d yday[%d]\n", timedate.tm_year+1900, timedate.tm_mon+1, timedate.tm_mday, timedate.tm_yday);
	printf("    %02d:%02d:%02d wday[%d]\n", timedate.tm_hour, timedate.tm_min, timedate.tm_sec, timedate.tm_wday);
	return 0;
}

