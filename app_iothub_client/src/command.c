#include <kernel.h>
#include <t_syslog.h>
#include <stdio.h>
#include <t_stdlib.h>
#include <stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "monitor.h"
#include "device.h"

/*
 *  デバイスコマンド番号
 */
static int_t led_func(int argc, char **argv);
#ifdef USE_PINGSEND
static int_t png_func(int argc, char **argv);
#endif
extern int iothub_client_main(int argc, char **argv);
extern int dps_csgen_main(int argc, char *argv[]);
extern int set_cs_main(int argc, char **argv);
extern int set_proxy_main(int argc, char **argv);
extern int clear_proxy_main(int argc, char **argv);
extern int set_wifi_main(int argc, char **argv);

/*
 *  デバイスコマンドテーブル
 */
static const COMMAND_INFO device_command_info[] = {
	{"LED",		led_func},
#ifdef USE_PINGSEND
	{"PING",    png_func},
#endif
	{"IOT",		iothub_client_main},
	{"CSGEN",	dps_csgen_main},
	{"SETCS",	set_cs_main},
	{"PROXY",	set_proxy_main},
	{"CPRX",	clear_proxy_main},
	{"WIFI",	set_wifi_main},
};

#define NUM_DEVICE_CMD   (sizeof(device_command_info)/sizeof(COMMAND_INFO))

static const char device_name[] = "DEVICE";
static const char device_help[] =
"  Device  LED (no) (on-1,off-0)  led   control\n"
#ifdef USE_PINGSEND
"          PING addr              send  ping   \n"
#endif
"          IOT                    connect azure\n"
"          CSGEN               provision device\n"
"          SETCS          set connection string\n"
"          PROXY                  set proxy    \n"
"          CPRX                   clear proxy  \n"
"          WIFI                   set ssid pwd \n";

#define LED_PIN       3		/* D13 */
static const uint16_t led_pattern[1] = {
	LED_PIN
};

static COMMAND_LINK device_command_link = {
	NULL,
	NUM_DEVICE_CMD,
	device_name,
	NULL,
	device_help,
	&device_command_info[0]
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
 *  DEVICEコマンド設定関数
 */
void device_info_init(intptr_t exinf)
{
	setup_command(&device_command_link);
}

/*
 *  ダイレクトデジタルピン出力
 */
void
digitalWrite(uint8_t Pin, int dwVal){
    int8_t gpio_pin = gpio_get_gpiohno(Pin, false);

    if( gpio_pin >= 0){
        gpio_set_pin(TADR_GPIOHS_BASE, (uint8_t)gpio_pin, dwVal);
    }
}

/*
 *  LED設定コマンド関数
 */
static int_t led_func(int argc, char **argv)
{
	int_t    arg1=0, arg2;

	if(argc < 3)
		return -1;
	arg1 = a2i(argv[1]);
	arg2 = a2i(argv[2]);
	if(arg1 >= 1 && arg1 <= 4){
		if(arg2 != 0)
			digitalWrite(led_pattern[arg1-1], 1);
		else
			digitalWrite(led_pattern[arg1-1], 0);
	}
	return 0;
}


#ifdef USE_PINGSEND
/*
 *  PING送信マンド関数
 */
static int_t png_func(int argc, char **argv)
{
	const char *addr;
	uint32_t paddr = 0;
	int      a, i;

	if(argc < 2)
		return -1;
	addr = argv[1];
	for(i = a = 0 ; i < 4 ; addr++){
		if(*addr <= '9' && *addr >= '0'){
			a *= 10;
			a += *addr - '0';
		}
		else{
			paddr |= a << (i * 8);
			a = 0;
			i++;
		}
		if(*addr == 0)
			break;
	}
	set_pingaddr(paddr);
	return 0;
}
#endif
