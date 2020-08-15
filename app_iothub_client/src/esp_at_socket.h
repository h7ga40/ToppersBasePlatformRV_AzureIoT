#ifndef _ESP_AT_SOCKET_H_
#define _ESP_AT_SOCKET_H_

#include <stdint.h>
#include <stdbool.h>

#define ETH_ALEN	6
struct ether_addr {
	uint8_t ether_addr_octet[ETH_ALEN];
};

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
	in_addr_t s_addr;
};

struct ether_addr *ether_aton_r(const char *x, struct ether_addr *p_a);
int inet_aton(const char *s0, struct in_addr *dest);

void init_esp_at();

/*ESPのバッファサイズ*/
#define AT_CONNECTION_RX_BUFF_SIZE 4096

#endif /* _ESP_AT_SOCKET_H_ */
