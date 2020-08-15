#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "main.h"
#include "kernel_cfg.h"
#include "esp_at_socket.h"

void __wrap_exit(int status)
{
	ext_ker();
}

int _kill(int pid, int sig)
{
	return 0;
}

int _getpid(int n)
{
	return 1;
}

int custom_rand_generate_seed(uint8_t *output, uint32_t sz)
{
	SYSTIM now;
	int32_t i;

	get_tim(&now);
	srand(now);

	for (i = 0; i < sz; i++)
		output[i] = rand();

	return 0;
}

// musl-libc

int inet_aton(const char *s0, struct in_addr *dest)
{
	const char *s = s0;
	unsigned char *d = (unsigned char *)dest;
	unsigned long a[4] = { 0 };
	char *z;
	int i;

	for (i = 0; i < 4; i++) {
		a[i] = strtoul(s, &z, 0);
		if (z == s || (*z && *z != '.') || !isdigit(*s))
			return 0;
		if (!*z) break;
		s = z + 1;
	}
	if (i == 4) return 0;
	switch (i) {
	case 0:
		a[1] = a[0] & 0xffffff;
		a[0] >>= 24;
	case 1:
		a[2] = a[1] & 0xffff;
		a[1] >>= 16;
	case 2:
		a[3] = a[2] & 0xff;
		a[2] >>= 8;
	}
	for (i = 0; i < 4; i++) {
		if (a[i] > 255) return 0;
		d[i] = a[i];
	}
	return 1;
}

struct ether_addr *ether_aton_r(const char *x, struct ether_addr *p_a)
{
	struct ether_addr a;
	char *y;
	for (int ii = 0; ii < 6; ii++) {
		unsigned long int n;
		if (ii != 0) {
			if (x[0] != ':') return 0; /* bad format */
			else x++;
		}
		n = strtoul(x, &y, 16);
		x = y;
		if (n > 0xFF) return 0; /* bad byte */
		a.ether_addr_octet[ii] = n;
	}
	if (x[0] != 0) return 0; /* bad format */
	*p_a = a;
	return p_a;
}
