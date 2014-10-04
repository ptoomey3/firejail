/*
 * Copyright (C) 2014 netblue30 (netblue30@yahoo.com)
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef FIREJAIL_H
#define FIREJAIL_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "../include/common.h"

#define USELOCK
#define FIREJAIL_DIR	"/tmp/firejail"
#define RO_DIR	"/tmp/firejail/firejail.ro.dir"
#define RO_FILE	"/tmp/firejail/firejail.ro.file"
#define MNT_DIR	"/tmp/firejail/mnt"



// main.c
typedef struct bridge_t {
	char *dev;		// bridge device name
	uint32_t ip;		// bridge device IP address
	uint32_t mask;		// bridge device mask
	uint32_t ipaddress;	// sandbox interface IP address
	unsigned char configured;
}  Bridge;

typedef struct profile_entry_t {
	struct profile_entry_t *next;
	char *data;
}ProfileEntry;

typedef struct config_t {
	// user data
	char *username;
	char *homedir;
	
	// filesystem
	char **custom_profile;	// loaded profile
	ProfileEntry *profile;
	char *chrootdir;		// chroot directory
	char *cwd;	// current working directory

	// networking
	char *hostname;
	uint32_t defaultgw;	// default gateway
	Bridge bridge0;
	Bridge bridge1;
	Bridge bridge2;
	Bridge bridge3;

	// command line, profile, hostname, chroot dir
	char *command_line;
	char *command_name;
} Config;
extern Config cfg;
static inline int any_bridge_configured(void) {
	if (cfg.bridge3.configured || cfg.bridge2.configured || cfg.bridge1.configured || cfg.bridge0.configured)
		return 1;
	else
		return 0;
}
extern int arg_private;		// mount private /home and /tmp directory
extern int arg_debug;		// print debug messages
extern int arg_nonetwork;	// --net=none
extern int arg_noip;		// --ip=none
extern int arg_command;	// -c
extern int arg_overlay;		// --overlay
extern int arg_zsh;		// use zsh as default shell
extern int arg_csh;		// use csh as default shell
extern int arg_seccomp;	// enable seccomp filter
extern char *arg_seccomp_list;//  optional seccomp list
extern int arg_caps;		// enable capabilities filter
extern int arg_trace;		// syscall tracing support
extern int fds[2];

#define MAX_ARGS 128		// maximum number of command arguments (argc)
extern char *fullargv[MAX_ARGS];
extern int fullargc;

// sandbox.c
int sandbox(void* sandbox_arg);


// network.c
// bring interface up
void net_if_up(const char *ifname);
// configure interface
void net_if_ip(const char *ifname, uint32_t ip, uint32_t mask);
// return -1 if the bridge was not found; if the bridge was found retrun 0 and fill in IP address and mask
int net_bridge_addr(const char *bridge, uint32_t *ip, uint32_t *mask);
// add an IP route, return -1 if error, 0 if OK
int net_add_route(uint32_t dest, uint32_t mask, uint32_t gw);
// print IP addresses for all interfaces
void net_ifprint(void);
// add a veth device to a bridge
void net_bridge_add_interface(const char *bridge, const char *dev);

// fs.c
// build /tmp/firejail directory
void fs_build_firejail_dir(void);
// build /tmp/firejail/mnt directory
void fs_build_mnt_dir(void);
// blacklist files or directoies by mounting empty files on top of them
void fs_blacklist(const char *homedir);
//void fs_blacklist(char **blacklist, const char *homedir);
// remount a directory read-only
void fs_rdonly(const char *dir);
// mount /proc and /sys directories
void fs_proc_sys_dev_boot(void);
// build a basic read-only filesystem
void fs_basic_fs(void);
// mount overlayfs on top of / directory
void fs_overlayfs(void);
// chroot into an existing directory; mount exiting /dev and update /etc/resolv.conf
void fs_chroot(const char *rootdir);

// profile.c
// find and read the profile specified by name from dir directory
void profile_find(const char *name, const char *dir);
// read a profile file
void profile_read(const char *fname);
// check profile line; if line == 0, this was generated from a command line option
void profile_check_line(char *ptr, int lineno);
// add a profile entry in cfg.profile list; use str to populate the list
void profile_add(char *str);

// list.c
void list(void);
void tree(void);
void top(void);
void drop_privs(void);

// usage.c
void usage(void);

// join.c
void join(pid_t pid, const char *homedir);
void join_name(const char *name, const char *homedir);
void shut(pid_t pid);
void shut_name(const char *name);

// restricted_shell.c
extern char *restricted_user;
int restricted_shell(const char *user);

// arp.c
// returns 0 if the address is not in use, -1 otherwise
int arp_check(const char *dev, uint32_t destaddr, uint32_t srcaddr);
// assign a random IP address and check it
uint32_t arp_random(const char *dev, uint32_t ifip, uint32_t ifmask);
// go sequentially trough all IP addresses and assign the first one not in use
uint32_t arp_sequential(const char *dev, uint32_t ifip, uint32_t ifmask);
// assign an IP address using the specified IP address or the ARP mechanism
uint32_t arp_assign(const char *dev, uint32_t ifip, uint32_t ifmask);

// veth.c
int net_create_veth(const char *dev, const char *nsdev, unsigned pid);

// util.c
void logsignal(int s);
void logmsg(const char *msg);
void logargs(int argc, char **argv) ;
void logerr(const char *msg);
int copy_file(const char *srcname, const char *destname);
char *get_link(const char *fname);
int is_dir(const char *fname);
int is_link(const char *fname);
char *line_remove_spaces(const char *buf);
char *split_colon(char *str);

// fs_var.c
void fs_var_log(void);	// mounting /var/log
void fs_var_lib(void);	// various other fixes for software in /var directory
void fs_var_cache(void); // various other fixes for software in /var/cache directory
void fs_var_run(void);
void fs_var_lock(void);
void fs_var_tmp(void);
void dbg_test_dir(const char *dir);

// fs_dev.c
void fs_dev_shm(void);

// fs_home.c
// private mode: mount tmpfs over /home and /tmp
void fs_private(const char *homedir);


// seccomp.c
int seccomp_filter(void);

// caps.c
int caps_filter(void);
void caps_print(void);

// syscall.c
const char *syscall_find_nr(int nr);
// return -1 if error, 0 if no error
int syscall_check_list(const char *slist, void (*callback)(int));
// print all available syscalls
void syscall_print(void);

// fs_trace.c
void fs_trace_preload(void);
void fs_trace(void);


#endif