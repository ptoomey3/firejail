#ifndef FIREJAIL_H
#define FIREJAIL_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define errExit(msg)    do { perror(msg); exit(1);} while (0)
#define PRINT_IP(A) \
((int) (((A) >> 24) & 0xFF)),  ((int) (((A) >> 16) & 0xFF)), ((int) (((A) >> 8) & 0xFF)), ((int) ( (A) & 0xFF))

static inline uint8_t mask2bits(uint32_t mask) {
	uint32_t tmp = 0x80000000;
	int i;
	uint8_t rv = 0;

	for (i = 0; i < 32; i++, tmp >>= 1) {
		if (tmp & mask)
			rv++;
		else
			break;
	}
	return rv;
}

static inline int atoip(const char *str, uint32_t *ip) {
	unsigned a, b, c, d;

	if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4 || a > 255 || b > 255 || c > 255 || d > 255)
		return 1;
		
	*ip = a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
	return 0;
}

// main.c
extern int arg_debug;
extern int arg_command;

// network.c
// bring interface up
void net_if_up(const char *ifname);
// configure interface
void net_if_ip(const char *ifname, uint32_t ip, uint32_t mask);
// find bridge address and mask, return 1 if error
int net_bridge_addr(const char *bridge, uint32_t *ip, uint32_t *mask);
// add an IP route, return 1 if error
int net_add_route(uint32_t dest, uint32_t mask, uint32_t gw);

// fs.c
void set_exit_parent(pid_t pid);
void bye_parent(void);
void mnt_blacklist(char **blacklist, const char *homedir);
void mnt_rdonly(const char *dir);
void mnt_proc_sys(void);
void mnt_basic_fs(void);
void mnt_home(const char *homedir);
void mnt_overlayfs(void);
void mnt_chroot(const char *rootdir);

// profile.c
void get_profile(const char *name, const char *dir);
void read_profile(const char *fname);

// main.c
extern char **custom_profile;

// list.c
void list(void);

// usage.c
void usage(void);

// join
void join(pid_t pid);
#endif