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
#include <stddef.h>
#include <stdbool.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <target_syssvc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "device.h"
#include "monitor.h"
#include "main.h"
#include "esp_at_socket.h"
#include "azure_c_shared_utility/esp_at_socket.h"
#include "azure_c_shared_utility/optimize_size.h"

#ifdef _MSC_VER
extern int sprintf_s(char* dst, size_t dstSizeInBytes, const char* format, ...);
#endif // _MSC_VER

//#define AT_DEBUG
time_t MINIMUM_YEAR;

typedef enum at_param_type_t {
	at_param_number,
	at_param_string,
	at_param_symbol,
} at_param_type_t;

typedef enum at_parse_state_t {
	at_none,
	at_informtion,
	at_response,
	at_link_id,
	at_parameter,
	at_number,
	at_string_str,
	at_string_esc,
	at_string_edq,
	at_symbol,
	at_line_text,
	at_cr,
	at_crlf,
	at_receive_data,
} at_parse_state_t;

typedef struct esp_serial_state_t esp_serial_state_t;
typedef bool (*set_parameter_t)(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value);
typedef at_parse_state_t (*exec_response_t)(esp_serial_state_t *esp_state, int pos);

typedef struct at_response_table_t {
	const char *name;
	set_parameter_t next_parameter;
	exec_response_t exec_response;
} at_response_table_t;

typedef enum at_response_kind_t {
	at_response_kind_no_set,
	at_response_kind_ok,
	at_response_kind_send_ok,
	at_response_kind_busy,
	at_response_kind_ready,
	at_response_kind_progress,
	at_response_kind_connect,
	at_response_kind_closed,
	at_response_kind_error,
	at_response_kind_fail,
	at_response_kind_already_connect,
	at_response_kind_received,
} at_response_kind_t;

typedef enum at_symbol_kind_t {
	at_symbol_kind_unknown,
	at_symbol_kind_apip,
	at_symbol_kind_apmac,
	at_symbol_kind_staip,
	at_symbol_kind_stamac,
	at_symbol_kind_gateway,
	at_symbol_kind_netmask,
} at_symbol_kind_t;

typedef enum at_connection_mode_t {
	at_single_connection_mode,
	at_multiple_connections_mode
} at_connection_mode_t;

typedef enum at_encryption_method_t {
	at_encryption_method_unknown,
	at_encryption_method_open,
	at_encryption_method_wep,
	at_encryption_method_wpa_psk,
	at_encryption_method_wpa2_psk,
	at_encryption_method_wpa_wpa2_psk,
	at_encryption_method_wpa2_enterprise,
} at_encryption_method_t;

typedef struct esp_ap_status_t {
	at_encryption_method_t encryption_method;
	char ssid[36];
	int rssi;
	struct ether_addr mac;
	int channel;
	int freq_offset;
	int freq_calibration;
} esp_ap_status_t;

typedef struct esp_ap_config_t {
	char ssid[36];
	char pwd[64];
	int channel;
	at_encryption_method_t encryption_method;
	int max_connection;
	int ssid_hidden;
} esp_ap_config_t;

typedef enum at_wifi_mode_t {
	at_wifi_mode_unknown,
	at_wifi_mode_station,
	at_wifi_mode_soft_ap,
	at_wifi_mode_soft_ap_station,
}at_wifi_mode_t;

typedef enum at_wifi_state_t {
	at_wifi_state_disconnected,
	at_wifi_state_connected,
	at_wifi_state_got_ip,
	at_wifi_state_busy,
}at_wifi_state_t;

typedef struct esp_socket_state_t {
	at_wifi_mode_t wifi_mode;
	at_wifi_state_t wifi_state;
	at_connection_mode_t connection_mode;
	struct ether_addr ap_mac;
	struct in_addr ap_ip;
	struct in_addr ap_gateway;
	struct in_addr ap_netmask;
	struct ether_addr sta_mac;
	struct in_addr sta_ip;
	struct in_addr sta_gateway;
	struct in_addr sta_netmask;
	char ssid[36];
	struct ether_addr baseid;
	int channel;
	int rssi;
	at_symbol_kind_t symbol_kind;
	int ap_status_count;
	esp_ap_status_t ap_status[10];
	esp_ap_config_t config;
	int link_id;
	int length;
	struct in_addr remote_ip;
	in_port_t remote_port;
	int received;
	int lost;
	time_t date_time;
} esp_socket_state_t;

typedef enum esp_state_t {
	esp_state_echo_off,
	esp_state_reset,
	esp_state_set_station_mode,
	esp_state_auto_connect_to_ap_off,
	esp_state_set_multiple_connections_mode,
	esp_state_show_remote_ip_and_port,
	esp_state_connect_ap,
	esp_state_update_time,
	esp_state_get_time,
	esp_state_done
} esp_state_t;

typedef struct esp_serial_state_t {
	esp_state_t esp_state;
	char *ssid;
	char *pwd;
	ID dtqid;
	at_parse_state_t parse_state;
	char string[64];
	int string_pos;
	int number;
	const at_response_table_t *response;
	at_response_kind_t response_kind;
	int param_pos;
	esp_socket_state_t params;
	esp_socket_state_t current_state;
} esp_serial_state_t;

typedef enum connection_state_t {
	connection_state_disconnected,
	connection_state_connecting,
	connection_state_connected,
	connection_state_disconnecting,
} connection_state_t;

typedef struct at_connection_t {
	int link_id;
	bool used;
	connection_state_t state;
	const char *ssl;
	bool blocking;
	unsigned int timeout;
	esp_serial_state_t *esp_state;
	uint8_t *rx_buff;
	uint32_t rx_pos_w;
	uint32_t rx_pos_r;
} at_connection_t;

esp_serial_state_t esp_serial_state;
extern const at_response_table_t response_table[];
extern const int response_table_count;
ER esp_serial_clear(esp_serial_state_t *esp_state, TMO tmo);
ER esp_serial_read(esp_serial_state_t *esp_state, at_response_kind_t *res_kind, TMO tmo);
ER esp_serial_write(esp_serial_state_t *esp_state, const char *text, at_response_kind_t *res_kind, TMO tmo);
int connection_read_data(at_connection_t *connection, void *data, int len);

at_connection_t connections[5] = {
	{ 0, false },
	{ 1, false },
	{ 2, false },
	{ 3, false },
	{ 4, false },
};

at_connection_t *new_connection()
{
	for (int i = 0; i < sizeof(connections) / sizeof(connections[0]); i++) {
		at_connection_t *connection = &connections[i];
		if (connection->used)
			continue;

		memset(&connection->used, 0, sizeof(*connection) - offsetof(at_connection_t, used));
		connection->rx_buff = calloc(1, AT_CONNECTION_RX_BUFF_SIZE);
		if (connection->rx_buff == NULL)
			return NULL;

		connection->used = true;
		return connection;
	}

	return NULL;
}

void delete_connection(at_connection_t *connection)
{
#ifdef AT_DEBUG
	printf("delete_connection %d\n", connection->link_id);
#endif
	free(connection->rx_buff);
	connection->used = false;
}

at_connection_t *get_connection(int link_id)
{
	for (int i = 0; i < sizeof(connections) / sizeof(connections[0]); i++) {
		if (connections[i].link_id == link_id) {
			return &connections[i];
		}
	}

	return NULL;
}

void init_esp_at()
{
	struct tm tm = {
        0,  /* tm_sec */
        0,  /* tm_min */
        0,  /* tm_hour */
        1,  /* tm_mday */
        0,  /* tm_mon */
        2020 - 1900,  /* tm_year */
	};
	MINIMUM_YEAR = mktime(&tm);

	esp_serial_state.current_state.wifi_mode = at_wifi_mode_station;
	esp_serial_state.dtqid = DTQ_ESP_AT;
}

const at_response_table_t *get_response(const char *name)
{
	const at_response_table_t *result;

	result = response_table;
	for (int i = 0; i < response_table_count; i++, result++) {
		if (strcmp(result->name, name) == 0)
			return result;
	}

	return NULL;
}

at_symbol_kind_t get_symbol(const char *name)
{
	if (strcmp("APIP", name) == 0)
		return at_symbol_kind_apip;
	if (strcmp("APMAC", name) == 0)
		return at_symbol_kind_apmac;
	if (strcmp("STAIP", name) == 0)
		return at_symbol_kind_staip;
	if (strcmp("STAMAC", name) == 0)
		return at_symbol_kind_stamac;
	if (strcmp("gateway", name) == 0)
		return at_symbol_kind_gateway;
	if (strcmp("netmask", name) == 0)
		return at_symbol_kind_netmask;
	return at_symbol_kind_unknown;
}

bool append_char(esp_serial_state_t *esp_state, int c)
{
	if (esp_state->string_pos >= sizeof(esp_state->string)) {
		esp_state->string[0] = '\0';
		return false;
	}

	esp_state->string[esp_state->string_pos++] = c;

	return true;
}

bool proc_atc_res(esp_serial_state_t *esp_state, int c)
{
	switch (esp_state->parse_state) {
	case at_none:
		if (c == '+') {
			esp_state->string_pos = 0;
			esp_state->string[esp_state->string_pos++] = c;
			esp_state->parse_state = at_informtion;
			return false;
		}
		else if (isalpha(c) || c == '>') {
			esp_state->string_pos = 0;
			esp_state->string[esp_state->string_pos++] = c;
			esp_state->parse_state = at_response;
			if (c == '>') {
				append_char(esp_state, '\0');
				esp_state->response = get_response(esp_state->string);
				if (esp_state->response != NULL) {
#ifdef AT_DEBUG
					printf("\033[35m%s\033[0m\r\n", esp_state->response->name);
#endif
					esp_state->parse_state = esp_state->response->exec_response(esp_state, esp_state->param_pos);
					esp_state->response = NULL;
					return true;
				}
			}
			return false;
		}
		else if (isdigit(c)) {
			esp_state->number = c - '0';
			esp_state->parse_state = at_link_id;
			return false;
		}
		else if (c == '\r') {
			esp_state->parse_state = at_crlf;
			return false;
		}
		return false;
	case at_informtion:
		if ((c == ':') || (c == ',')) {
			append_char(esp_state, '\0');
			esp_state->response = get_response(esp_state->string);
			if (esp_state->response != NULL) {
				esp_state->param_pos = 0;
				memset(&esp_state->params, 0, sizeof(esp_state->params));
				esp_state->parse_state = at_parameter;
				return false;
			}
		}
		else if (isalpha(c) || isdigit(c) || c == ' ') {
			append_char(esp_state, c);
			esp_state->parse_state = at_informtion;
			return false;
		}
		break;
	case at_response:
		if ((c == '\r') || (c == '.')) {
			append_char(esp_state, '\0');
			esp_state->response = get_response(esp_state->string);
			if (esp_state->response != NULL) {
				esp_state->param_pos = 0;
				memset(&esp_state->params, 0, sizeof(esp_state->params));
				esp_state->parse_state = at_crlf;
				return false;
			}
#ifdef AT_DEBUG
			else{
				append_char(esp_state, '\0');
				printf("\"%s\"\r\n", esp_state->string);
			}
#endif
		}
		else if (isalpha(c) || isdigit(c) || c == ' ') {
			append_char(esp_state, c);
			esp_state->parse_state = at_response;
			return false;
		}
#ifdef AT_DEBUG
		else{
			append_char(esp_state, '\0');
			printf("\"%s\"\r\n", esp_state->string);
		}
#endif
		break;
	case at_link_id:
		if (isdigit(c)) {
			esp_state->parse_state = at_link_id;
			esp_state->number *= 10;
			esp_state->number += c - '0';
			return false;
		}
		else if (c == ',') {
			esp_state->param_pos = 0;
			memset(&esp_state->params, 0, sizeof(esp_state->params));
			esp_state->params.link_id = esp_state->number;
			esp_state->string_pos = 0;
			esp_state->parse_state = at_response;
			return false;
		}
		break;
	case at_parameter:
		if (isdigit(c)) {
			esp_state->parse_state = at_number;
			esp_state->number = c - '0';
			return false;
		}
		else if (c == '"') {
			esp_state->string_pos = 0;
			esp_state->string[esp_state->string_pos] = '\0';
			esp_state->parse_state = at_string_str;
			return false;
		}
		else if (isalpha(c)) {
			esp_state->string_pos = 0;
			esp_state->string[esp_state->string_pos++] = c;
			esp_state->parse_state = at_line_text;
			return false;
		}
		break;
	case at_number:
		if (isdigit(c)) {
			esp_state->parse_state = at_number;
			esp_state->number *= 10;
			esp_state->number += c - '0';
			return false;
		}
		else if (c == ',') {
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_number, esp_state->number)) {
				esp_state->parse_state = at_parameter;
				return false;
			}
		}
		else if (c == ':') {
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_number, esp_state->number)) {
#ifdef AT_DEBUG
				printf("\033[35m%s\033[0m\r\n", esp_state->response->name);
#endif
				esp_state->parse_state = esp_state->response->exec_response(esp_state, esp_state->param_pos);
				esp_state->string_pos = 0;
				esp_state->response = NULL;
				return true;
			}
		}
		else if (c == '\r') {
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_number, esp_state->number)) {
				esp_state->parse_state = at_crlf;
				return false;
			}
		}
		break;
	case at_string_str:
		if (c == '"') {
			esp_state->parse_state = at_string_edq;
			return false;
		}
		else if (c == '\\') {
			esp_state->parse_state = at_string_esc;
			return false;
		}
		else {
			append_char(esp_state, c);
			esp_state->parse_state = at_string_str;
			return false;
		}
		break;
	case at_string_esc:
		append_char(esp_state, c);
		esp_state->parse_state = at_string_str;
		return false;
	case at_string_edq:
		if (c == ',') {
			append_char(esp_state, '\0');
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_string, (intptr_t)esp_state->string)) {
				esp_state->parse_state = at_parameter;
				return false;
			}
		}
		else if (c == '\r') {
			append_char(esp_state, '\0');
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_string, (intptr_t)esp_state->string)) {
				esp_state->parse_state = at_crlf;
				return false;
			}
		}
		break;
	case at_symbol:
		if (c == ',' || c == ':') {
			append_char(esp_state, '\0');
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_symbol, (intptr_t)get_symbol(esp_state->string))) {
				esp_state->parse_state = at_parameter;
				return false;
			}
		}
		else if (c == '\r') {
			append_char(esp_state, '\0');
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_symbol, (intptr_t)get_symbol(esp_state->string))) {
				esp_state->parse_state = at_crlf;
				return false;
			}
		}
		else if (isalpha(c) || isdigit(c) || c == ' ') {
			append_char(esp_state, c);
			esp_state->parse_state = at_symbol;
			return false;
		}
		break;
	case at_line_text:
		if (c == '\r') {
			append_char(esp_state, '\0');
			esp_state->param_pos++;
			if (esp_state->response->next_parameter(esp_state, esp_state->param_pos, at_param_symbol, (intptr_t)esp_state->string)) {
				esp_state->parse_state = at_crlf;
				return false;
			}
		}
		else if (isprint(c)) {
			append_char(esp_state, c);
			esp_state->parse_state = at_line_text;
			return false;
		}
		break;
	case at_cr:
		if (c == '\r') {
			esp_state->parse_state = at_crlf;
			return false;
		}
		break;
	case at_crlf:
		if (c == '\n') {
			if (esp_state->response != NULL) {
#ifdef AT_DEBUG
				printf("\033[35m%s\033[0m\r\n", esp_state->response->name);
#endif
				esp_state->parse_state = esp_state->response->exec_response(esp_state, esp_state->param_pos);
			}
			else {
				esp_state->parse_state = at_none;
			}
			esp_state->string_pos = 0;
			esp_state->response = NULL;
			return true;
		}
		else if (c == '\r' || (c == '.')) {
			esp_state->parse_state = at_crlf;
			return false;
		}
		break;
	case at_receive_data: {
		esp_state->current_state.received++;
		at_connection_t *connection = get_connection(esp_state->current_state.link_id);
		if (connection != NULL) {
			int pos = connection->rx_pos_w + 1;
			if (pos >= AT_CONNECTION_RX_BUFF_SIZE)
				pos = 0;
			if (pos != connection->rx_pos_r) {
				connection->rx_buff[connection->rx_pos_w] = c;
				connection->rx_pos_w = pos;
			}
			else {
				esp_state->current_state.lost++;
			}
		}
		else {
			esp_state->current_state.lost++;
		}
		if (esp_state->current_state.received >= esp_state->current_state.length) {
#ifdef AT_DEBUG
			printf("\033[35mrecv:%d, lost:%d\033[0m\r\n", esp_state->current_state.received, esp_state->current_state.lost);
#endif
			esp_state->parse_state = at_none;
			esp_state->response_kind = at_response_kind_received;
			return true;
		}
		return false;
	}
	}
#ifdef AT_DEBUG
	if (esp_state->parse_state != at_none)
		printf("\033[35mparse error. state:%d, char:%d\033[0m\r\n", esp_state->parse_state, c);
#endif
	esp_state->parse_state = at_none;
	esp_state->response = NULL;
	return false;
}

ER esp_serial_clear(esp_serial_state_t *esp_state, TMO tmo)
{
	return ini_dtq(esp_state->dtqid);
}

ER esp_serial_read(esp_serial_state_t *esp_state, at_response_kind_t *res_kind, TMO tmo)
{
	ER ret;
	intptr_t data = 0;
	ret = trcv_dtq(esp_state->dtqid, (intptr_t *)&data, tmo);
	if ((ret != E_OK) && (ret != E_TMOUT)) {
		printf("trcv_dtq error %s", itron_strerror(ret));
	}
	*res_kind = (at_response_kind_t)data;
	return ret;
}

ER esp_serial_write(esp_serial_state_t *esp_state, const char *text, at_response_kind_t *res_kind, TMO tmo)
{
	ER ret;

	ret = esp_serial_clear(esp_state, 100);
	if ((ret != E_OK) && (ret != E_TMOUT)) {
		printf("esp_serial_clear error %s", itron_strerror(ret));
		return ret;
	}

#ifdef AT_DEBUG
	printf("\033[32m%s\033[0m", text);
#endif
	ret = serial_wri_dat(AT_PORTID, text, (uint_t)strlen(text));
	if (ret < 0) {
		printf("serial_wri_dat error %s", itron_strerror(ret));
		return ret;
	}

	return esp_serial_read(esp_state, res_kind, tmo);
}

int connection_read_data(at_connection_t *connection, void *data, int len)
{
	esp_serial_state_t *esp_state = connection->esp_state;
	int ret = 0;

	for (unsigned char *pos = (unsigned char *)data, *end = &pos[len]; pos < end; pos++) {
		if (connection->rx_pos_w == connection->rx_pos_r) {
			return ret;
		}

		*pos = connection->rx_buff[connection->rx_pos_r];
		connection->rx_pos_r++;
		if (connection->rx_pos_r >= AT_CONNECTION_RX_BUFF_SIZE)
			connection->rx_pos_r = 0;

		ret++;
	}

	return ret;
}

static bool at_cipmux_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		switch (value) {
		case 0:
			params->connection_mode = at_single_connection_mode;
			return true;
		case 1:
			params->connection_mode = at_multiple_connections_mode;
			return true;
		}
		break;
	}

	return false;
}

static at_parse_state_t at_cipmux_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos >= 1)
		esp_state->current_state.connection_mode = esp_state->params.connection_mode;

	return at_none;
}

static bool at_cifsr_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		params->symbol_kind = (at_symbol_kind_t)value;
		return true;
	case 2:
		switch (params->symbol_kind)
		{
		case at_symbol_kind_apip:
			if (inet_aton((const char *)value, &params->ap_ip)) {
				return true;
			}
			break;
		case at_symbol_kind_apmac:
			if (ether_aton_r((const char *)value, &params->ap_mac)) {
				return true;
			}
			break;
		case at_symbol_kind_staip:
			if (inet_aton((const char *)value, &params->sta_ip)) {
				return true;
			}
			break;
		case at_symbol_kind_stamac:
			if (ether_aton_r((const char *)value, &params->sta_mac)) {
				return true;
			}
			break;
		}
		break;
	}

	return false;
}

static at_parse_state_t at_cifsr_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos < 2)
		return at_none;

	switch (esp_state->params.symbol_kind)
	{
	case at_symbol_kind_apip:
		esp_state->current_state.ap_ip = esp_state->params.ap_ip;
		break;
	case at_symbol_kind_apmac:
		esp_state->current_state.ap_mac = esp_state->params.ap_mac;
		break;
	case at_symbol_kind_staip:
		esp_state->current_state.sta_ip = esp_state->params.sta_ip;
		break;
	case at_symbol_kind_stamac:
		esp_state->current_state.sta_mac = esp_state->params.sta_mac;
		break;
	}

	return at_none;
}

static bool at_cipap_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		params->symbol_kind = (at_symbol_kind_t)value;
		return true;
	case 2:
		switch (params->symbol_kind)
		{
		case at_symbol_kind_gateway:
			if (inet_aton((const char *)value, &params->ap_gateway)) {
				return true;
			}
			break;
		case at_symbol_kind_netmask:
			if (inet_aton((const char *)value, &params->ap_netmask)) {
				return true;
			}
			break;
		}
		break;
	}

	return false;
}

static at_parse_state_t at_cipap_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos < 2)
		return at_none;

	switch (esp_state->params.symbol_kind)
	{
	case at_symbol_kind_gateway:
		esp_state->current_state.ap_gateway = esp_state->params.ap_gateway;
		break;
	case at_symbol_kind_netmask:
		esp_state->current_state.ap_netmask = esp_state->params.ap_netmask;
		break;
	}

	return at_none;
}

static bool at_cipsntptime_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;
	struct tm date_time = { 0 };

	switch (pos) {
	case 1:
		if (strptime((const char *)value, "%a %b %d %T %Y", &date_time) != NULL) {
			printf("%04d/%02d/%02d %02d:%02d:%02d\r\n",
				date_time.tm_year+1900, date_time.tm_mon+1, date_time.tm_mday,
				date_time.tm_hour, date_time.tm_min, date_time.tm_sec);
			params->date_time = mktime(&date_time);
			return true;
		}
		break;
	}

	return false;
}

static at_parse_state_t at_cipsntptime_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_socket_state_t *params = &esp_state->params;
	struct tm current_time_val = { 0 };
	time_t current_time = params->date_time;

	if (current_time <= MINIMUM_YEAR)
		return at_none;

	esp_state->current_state.date_time = current_time;
	gmtime_r(&current_time, &current_time_val);
	rtc_set_time((struct tm2 *)&current_time_val);

	current_time = time(NULL);
	printf("%s\r\n", ctime(&current_time));

	return at_none;
}

static bool at_cipsta_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		params->symbol_kind = (at_symbol_kind_t)value;
		return true;
	case 2:
		switch (params->symbol_kind)
		{
		case at_symbol_kind_gateway:
			if (inet_aton((const char *)value, &params->sta_gateway)) {
				return true;
			}
			break;
		case at_symbol_kind_netmask:
			if (inet_aton((const char *)value, &params->sta_netmask)) {
				return true;
			}
			break;
		}
		break;
	}

	return false;
}

static at_parse_state_t at_cipsta_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos < 2)
		return at_none;

	switch (esp_state->params.symbol_kind)
	{
	case at_symbol_kind_gateway:
		esp_state->current_state.sta_gateway = esp_state->params.sta_gateway;
		break;
	case at_symbol_kind_netmask:
		esp_state->current_state.sta_netmask = esp_state->params.sta_netmask;
		break;
	}

	return at_none;
}

static bool at_cwjap_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		if (type == at_param_string) {
			strcpy(params->ssid, (const char *)value);
			return true;
		}
		break;
	case 2:
		if (ether_aton_r((const char *)value, &params->baseid)) {
			return true;
		}
		break;
	case 3:
		params->channel = (int)value;
		return true;
	case 4:
		params->rssi = (int)value;
		return true;
		break;
	}

	return false;
}

static at_parse_state_t at_cwjap_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos >= 1)
		strcpy(esp_state->current_state.ssid, esp_state->params.ssid);

	if (pos >= 2)
		esp_state->current_state.baseid = esp_state->params.baseid;

	if (pos >= 3)
		esp_state->current_state.channel = esp_state->params.channel;

	if (pos >= 4)
		esp_state->current_state.rssi = esp_state->params.rssi;

	return at_none;
}

static bool at_cwlap_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	if (esp_state->params.ap_status_count >= sizeof(esp_state->params.ap_status) / sizeof(esp_state->params.ap_status[0]))
		return false;

	esp_ap_status_t *ap_status = &esp_state->params.ap_status[esp_state->params.ap_status_count];

	switch (pos) {
	case 1:
		switch (value) {
		case 0:
			ap_status->encryption_method = at_encryption_method_open;
			return true;
		case 1:
			ap_status->encryption_method = at_encryption_method_wep;
			return true;
		case 2:
			ap_status->encryption_method = at_encryption_method_wpa_psk;
			return true;
		case 3:
			ap_status->encryption_method = at_encryption_method_wpa2_psk;
			return true;
		case 4:
			ap_status->encryption_method = at_encryption_method_wpa_wpa2_psk;
			return true;
		case 5:
			ap_status->encryption_method = at_encryption_method_wpa2_enterprise;
			return true;
		}
		break;
	case 2:
		strcpy(ap_status->ssid, (const char *)value);
		return true;
	case 3:
		ap_status->rssi = (int)value;
		return true;
		break;
	case 4:
		if (ether_aton_r((const char *)value, &ap_status->mac)) {
			return true;
		}
		break;
	case 5:
		ap_status->channel = (int)value;
		return true;
	case 6:
		ap_status->freq_offset = (int)value;
		return true;
	case 7:
		ap_status->freq_calibration = (int)value;
		return true;
	}

	return false;
}

static at_parse_state_t at_cwlap_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->params.ap_status_count++;

	return at_none;
}

static bool at_cwmode_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		switch (value) {
		case 1:
			params->wifi_mode = at_wifi_mode_unknown;
			return true;
		case 2:
			params->wifi_mode = at_wifi_mode_station;
			return true;
		case 3:
			params->wifi_mode = at_wifi_mode_soft_ap;
			return true;
		case 4:
			params->wifi_mode = at_wifi_mode_soft_ap_station;
			return true;
		}
	}

	return false;
}

static at_parse_state_t at_cwmode_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos >= 1)
		esp_state->current_state.wifi_mode = esp_state->params.wifi_mode;

	return at_none;
}

static bool at_cwsap_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_ap_config_t *ap_config = &esp_state->params.config;

	switch (pos) {
	case 1:
		strcpy(ap_config->ssid, (const char *)value);
		return true;
	case 2:
		strcpy(ap_config->pwd, (const char *)value);
		return true;
	case 3:
		ap_config->channel = (int)value;
		return true;
	case 4:
		switch (value) {
		case 0:
			ap_config->encryption_method = at_encryption_method_open;
			return true;
		case 1:
			ap_config->encryption_method = at_encryption_method_wep;
			return true;
		case 2:
			ap_config->encryption_method = at_encryption_method_wpa_psk;
			return true;
		case 3:
			ap_config->encryption_method = at_encryption_method_wpa2_psk;
			return true;
		case 4:
			ap_config->encryption_method = at_encryption_method_wpa_wpa2_psk;
			return true;
		case 5:
			ap_config->encryption_method = at_encryption_method_wpa2_enterprise;
			return true;
		}
		break;
	case 5:
		ap_config->max_connection = (int)value;
		break;
	case 6:
		switch (value) {
		case 0:
			ap_config->ssid_hidden = 0;
			return true;
		case 1:
			ap_config->ssid_hidden = 1;
			return true;
		}
	}

	return false;
}

static at_parse_state_t at_cwsap_exec(esp_serial_state_t *esp_state, int pos)
{
	return at_none;
}

static bool at_ipd_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	esp_socket_state_t *params = &esp_state->params;

	switch (pos) {
	case 1:
		if ((value >= 0) && (value <= 4)) {
			params->link_id = (int)value;
			return true;
		}
		break;
	case 2:
		if ((value >= 1) && (value <= AT_CONNECTION_RX_BUFF_SIZE)) {
			params->length = (int)value;
			return true;
		}
		break;
	case 3:
		if (inet_aton((const char *)value, &params->remote_ip)) {
			return true;
		}
		break;
	case 4:
		if (value >= 0 && value < 65536) {
			params->remote_port = (in_port_t)value;
			return true;
		}
		break;
	}

	return false;
}

static at_parse_state_t at_ipd_exec(esp_serial_state_t *esp_state, int pos)
{
	if (pos >= 1)
		esp_state->current_state.link_id = esp_state->params.link_id;
	if (pos >= 2)
		esp_state->current_state.length = esp_state->params.length;
	if (pos >= 3)
		esp_state->current_state.remote_ip = esp_state->params.remote_ip;
	if (pos >= 4)
		esp_state->current_state.remote_port = esp_state->params.remote_port;

	esp_state->current_state.received = 0;
	esp_state->current_state.lost = 0;
	esp_state->response_kind = at_response_kind_no_set;

	return at_receive_data;
}

static bool at_already_connect_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_already_connect_exec(esp_serial_state_t *esp_state, int pos)
{
	at_connection_t *connection = get_connection(esp_state->params.link_id);
	if (connection != NULL) {
		connection->state = connection_state_connected;
	}

	esp_state->response_kind = at_response_kind_already_connect;

	return at_none;
}

static bool at_error_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_error_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_error;

	return at_none;
}

static bool at_fail_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_fail_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_fail;

	return at_none;
}

static bool at_ok_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_ok_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_ok;

	return at_none;
}

static bool at_send_ok_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_send_ok_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_send_ok;

	return at_none;
}

static bool at_wifi_connected_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_wifi_connected_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->current_state.wifi_state = at_wifi_state_connected;

	return at_none;
}

static bool at_wifi_disconnect_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_wifi_disconnect_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->current_state.wifi_state = at_wifi_state_disconnected;

	return at_none;
}

static bool at_wifi_got_ip_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_wifi_got_ip_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->current_state.wifi_state = at_wifi_state_got_ip;

	return at_none;
}

static bool at_busy_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_busy_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_busy;
	esp_state->current_state.wifi_state = at_wifi_state_busy;

	return at_none;
}

static bool at_ready_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_ready_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_ready;
	esp_state->current_state.wifi_state = at_wifi_state_disconnected;

	return at_none;
}

static bool at_progress_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_progress_exec(esp_serial_state_t *esp_state, int pos)
{
	esp_state->response_kind = at_response_kind_progress;

	return at_none;
}

static bool at_connect_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_connect_exec(esp_serial_state_t *esp_state, int pos)
{
	at_connection_t *connection = get_connection(esp_state->params.link_id);
	if (connection != NULL) {
		connection->state = connection_state_connected;
	}

	esp_state->response_kind = at_response_kind_connect;

	return at_none;
}

static bool at_closed_set_param(esp_serial_state_t *esp_state, int pos, at_param_type_t type, intptr_t value)
{
	// 引数なし
	return false;
}

static at_parse_state_t at_closed_exec(esp_serial_state_t *esp_state, int pos)
{
	at_connection_t *connection = get_connection(esp_state->params.link_id);
	if (connection != NULL) {
		connection->state = connection_state_disconnected;
	}

	esp_state->response_kind = at_response_kind_closed;

	return at_none;
}

const at_response_table_t response_table[] = {
	{ "+CIPMUX", at_cipmux_set_param, at_cipmux_exec },
	{ "+CIFSR", at_cifsr_set_param, at_cifsr_exec },
	{ "+CIPAP", at_cipap_set_param, at_cipap_exec },
	{ "+CIPSNTPTIME", at_cipsntptime_set_param, at_cipsntptime_exec },
	{ "+CIPSTA", at_cipsta_set_param, at_cipsta_exec },
	{ "+CWJAP", at_cwjap_set_param, at_cwjap_exec },
	{ "+CWLAP", at_cwlap_set_param, at_cwlap_exec },
	{ "+CWMODE", at_cwmode_set_param, at_cwmode_exec },
	{ "+CWSAP", at_cwsap_set_param, at_cwsap_exec },
	{ "+IPD", at_ipd_set_param, at_ipd_exec },
	{ "ALREADY CONNECT", at_already_connect_set_param, at_already_connect_exec },
	{ "ERROR", at_error_set_param, at_error_exec },
	{ "FAIL", at_fail_set_param, at_fail_exec },
	{ "OK", at_ok_set_param, at_ok_exec },
	{ "SEND OK", at_send_ok_set_param, at_send_ok_exec },
	{ "WIFI CONNECTED", at_wifi_connected_set_param, at_wifi_connected_exec },
	{ "WIFI DISCONNECT", at_wifi_disconnect_set_param, at_wifi_disconnect_exec },
	{ "WIFI GOT IP", at_wifi_got_ip_set_param, at_wifi_got_ip_exec },
	{ "busy p", at_busy_set_param, at_busy_exec },
	{ "ready", at_ready_set_param, at_ready_exec },
	{ ">", at_progress_set_param, at_progress_exec },
	{ "CONNECT", at_connect_set_param, at_connect_exec},
	{ "CLOSED", at_closed_set_param, at_closed_exec},
};
const int response_table_count = sizeof(response_table) / sizeof(response_table[0]);

int esp_at_rx_handler(void *data, int len)
{
	esp_serial_state_t *esp_state = &esp_serial_state;
	uint8_t *c = (uint8_t *)data;
	ER ret;

	for (uint8_t *end = &c[len]; c < end; c++) {
		if (proc_atc_res(esp_state, *c)) {
			if (esp_state->response_kind != at_response_kind_no_set) {
				ret = tsnd_dtq(esp_state->dtqid, (intptr_t)esp_state->response_kind, 20);
				if (ret != E_OK) {
					printf("snd_dtq error %s\n", itron_strerror(ret));
				}
				esp_state->response_kind = at_response_kind_no_set;
			}
		}
	}

	return (int)((intptr_t)c - (intptr_t)data);
}

void set_ssid_pwd(const char *ssid, const char *pwd)
{
	esp_serial_state_t *esp_state = &esp_serial_state;

	free(esp_state->ssid);
	esp_state->ssid = strdup(ssid);
	free(esp_state->pwd);
	esp_state->pwd = strdup(pwd);
}

bool prepare_esp_at()
{
	esp_serial_state_t *esp_state = &esp_serial_state;
	ER ret;
	char temp[64];
	int tmo = 0;
	at_response_kind_t res_kind = at_response_kind_no_set;

	if (esp_state->current_state.wifi_state != at_wifi_state_got_ip)
		esp_state->esp_state = esp_state_echo_off;

retry:
	ret = esp_serial_clear(esp_state, 100);
	if ((ret != E_OK) && (ret != E_TMOUT))
		return false;

	switch (esp_state->esp_state) {
	case esp_state_reset:
		ret = esp_serial_write(esp_state, "AT+RST\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("reset error %s %d\n", itron_strerror(ret), res_kind);
			return false;
		}

		ret = esp_serial_read(esp_state, &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ready)) {
			printf("ready error %s %d\n", itron_strerror(ret), res_kind);
			return false;
		}
		esp_state->esp_state = esp_state_echo_off;
	case esp_state_echo_off:
		ret = esp_serial_write(esp_state, "ATE0\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("echo off error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_set_station_mode;
	case esp_state_set_station_mode:
		ret = esp_serial_write(esp_state, "AT+CWMODE=1\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("set station mode error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_auto_connect_to_ap_off;
	case esp_state_auto_connect_to_ap_off:
		ret = esp_serial_write(esp_state, "AT+CWAUTOCONN=0\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("auto connect off error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_set_multiple_connections_mode;
	case esp_state_set_multiple_connections_mode:
		ret = esp_serial_write(esp_state, "AT+CIPMUX=1\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("set multiple connections mode error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_show_remote_ip_and_port;
	case esp_state_show_remote_ip_and_port:
		ret = esp_serial_write(esp_state, "AT+CIPDINFO=1\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("show remote ip/port error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_connect_ap;
	case esp_state_connect_ap:
		if ((esp_state->ssid == NULL) || (esp_state->pwd == NULL)) {
			printf("ssid psw error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}

		sprintf_s(temp, sizeof(temp), "AT+CWJAP=\"%s\",\"%s\"\r\n", esp_state->ssid, esp_state->pwd);
		ret = esp_serial_write(esp_state, temp, &res_kind, 20000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("connect wifi error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_update_time;
	case esp_state_update_time:
		sprintf_s(temp, sizeof(temp), "AT+CIPSNTPCFG=%d,%d,\"%s\"\r\n", 1, 9, "ntp.nict.jp");
		ret = esp_serial_write(esp_state, temp, &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("ntp config error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		esp_state->esp_state = esp_state_get_time;
	case esp_state_get_time:
		ret = esp_serial_write(esp_state, "AT+CIPSNTPTIME?\r\n", &res_kind, 10000);
		if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
			printf("get time error %s %d\n", itron_strerror(ret), res_kind);
			break;
		}
		if (esp_state->current_state.date_time <= MINIMUM_YEAR) {
			tmo++;
			if (tmo < 10) {
				printf("retry to get time %s\n", ctime(NULL));
				dly_tsk(1000);
				esp_state->esp_state = esp_state_get_time;
				goto retry;
			}
		}
		esp_state->esp_state = esp_state_done;
	case esp_state_done: {
		time_t ntp = esp_state->current_state.date_time;
		time_t local = time(NULL);
		if ((local > ntp) && ((local - ntp) > 60 * 60)) {
			esp_state->esp_state = esp_state_get_time;
			goto retry;
		}
		return true;
	}
	default:
		esp_state->esp_state = esp_state_echo_off;
		goto retry;
	}

	if (res_kind == at_response_kind_busy){
		esp_state->esp_state = esp_state_reset;
		goto retry;
	}

	printf("prepare error %s %d\n", itron_strerror(ret), res_kind);
	esp_state->esp_state = esp_state_echo_off;
	return false;
}

ESP_AT_SOCKET_HANDLE esp_at_socket_create(bool ssl)
{
	at_connection_t *connection = new_connection();
	if (connection == NULL)
		return NULL;

	connection->ssl = ssl ? "SSL" : "TCP";
	connection->esp_state = &esp_serial_state;

	return (ESP_AT_SOCKET_HANDLE)connection;
}

void esp_at_socket_set_blocking(ESP_AT_SOCKET_HANDLE espAtSocketHandle, bool blocking, unsigned int timeout)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;

	connection->blocking = blocking;
	connection->timeout = timeout;

#ifdef AT_DEBUG
	printf("\033[35mblocking:%s timeout:%d\033[0m\n", blocking ? "true" : "false", timeout);
#endif
}

void esp_at_socket_destroy(ESP_AT_SOCKET_HANDLE espAtSocketHandle)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;

	delete_connection(connection);
}

int esp_at_socket_connect(ESP_AT_SOCKET_HANDLE espAtSocketHandle, const char *host, const int port)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;
	at_response_kind_t res_kind;

	if (connection->state == connection_state_connected) {
		printf("esp_at_socket_connect state error %d\n", connection->state);
		return 0;
	}

	if (!prepare_esp_at()) {
		printf("prepare_esp_at error\n");
		return -MU_FAILURE;
	}

	connection->state = connection_state_connecting;

	ER ret;
	char temp[128];
	sprintf_s(temp, sizeof(temp), "AT+CIPSTART=%d,\"%s\",\"%s\",%d\r\n", connection->link_id, connection->ssl, host, port);
	ret = esp_serial_write(connection->esp_state, temp, &res_kind, 10000);
	if ((ret != E_OK) || (res_kind != at_response_kind_connect)) {
		printf("connect error connect %s %d\n", itron_strerror(ret), res_kind);
		return -MU_FAILURE;
	}
	ret = esp_serial_read(connection->esp_state, &res_kind, 10000);
	if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
		printf("connect error done %s %d\n", itron_strerror(ret), res_kind);
		return -MU_FAILURE;
	}
#ifdef AT_DEBUG
	printf("\033[35mconnected !\033[0m\n");
#endif
	return 0;
}

bool esp_at_socket_is_connected(ESP_AT_SOCKET_HANDLE espAtSocketHandle)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;

	return connection->state == connection_state_connected;
}

void esp_at_socket_close(ESP_AT_SOCKET_HANDLE espAtSocketHandle)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;
	at_response_kind_t res_kind;

	if (connection->state == connection_state_disconnected) {
		printf("esp_at_socket_close state error %d\n", connection->state);
		return;
	}

	connection->state = connection_state_disconnecting;

	ER ret;
	char temp[64];
	sprintf_s(temp, sizeof(temp), "AT+CIPCLOSE=%d\r\n", connection->link_id);
	ret = esp_serial_write(connection->esp_state, temp, &res_kind, 10000);
	if (ret == E_OK) {
		if (res_kind == at_response_kind_closed) {
			ret = esp_serial_read(connection->esp_state, &res_kind, 10000);
		}
		if ((ret == E_OK) && (res_kind == at_response_kind_ok)) {
			return;
		}
	}

	printf("close error %s %d\n", itron_strerror(ret), res_kind);
}

int esp_at_socket_send(ESP_AT_SOCKET_HANDLE espAtSocketHandle, const char *data, int length)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;
	at_response_kind_t res_kind;

	if (connection->state != connection_state_connected) {
		printf("esp_at_socket_send state error %d\n", connection->state);
		return -MU_FAILURE;
	}

	ER ret;
	char temp[64];
	sprintf_s(temp, sizeof(temp), "AT+CIPSEND=%d,%d\r\n", connection->link_id, length);
	ret = esp_serial_write(connection->esp_state, temp, &res_kind, 10000);
	if ((ret != E_OK) || (res_kind != at_response_kind_ok)) {
		printf("send error command %s %d\n", itron_strerror(ret), res_kind);
		return -MU_FAILURE;
	}
	do {
		ret = esp_serial_read(connection->esp_state, &res_kind, 10000);
		if ((ret != E_OK) || ((res_kind != at_response_kind_ok)
			&& (res_kind != at_response_kind_progress))) {
			printf("send error progress %s %d\n", itron_strerror(ret), res_kind);
			return -MU_FAILURE;
		}
	} while(res_kind == at_response_kind_ok);
	ret = serial_wri_dat(AT_PORTID, data, length);
	if (ret < 0) {
		printf("send error write data %s\n", itron_strerror(ret));
		return -MU_FAILURE;
	}
	do {
		ret = esp_serial_read(connection->esp_state, &res_kind, 60000);
		if (ret != E_OK) {
			printf("send error done %s\n", itron_strerror(ret));
			break;
		}
		if (res_kind == at_response_kind_send_ok)
			return length;
	} while ((res_kind == at_response_kind_no_set)
		|| (res_kind == at_response_kind_ok));

	printf("send error done kind=%d\n", res_kind);

	return -MU_FAILURE;
}

int esp_at_socket_receive(ESP_AT_SOCKET_HANDLE espAtSocketHandle, char *data, int length)
{
	at_connection_t *connection = (at_connection_t *)espAtSocketHandle;

	if (connection->state != connection_state_connected) {
		printf("esp_at_socket_receive state error %d\n", connection->state);
		return -MU_FAILURE;
	}

	int ret;
	ret = connection_read_data(connection, data, length);
	if (ret < 0) {
		printf("receive error %s\n", itron_strerror(ret));
		return -MU_FAILURE;
	}
#ifdef AT_DEBUG
	if (ret > 0)
		printf("\033[35mreceived %d\033[0m\n", ret);
#endif
	return ret;
}
