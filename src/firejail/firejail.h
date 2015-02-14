/*
 * Copyright (C) 2014, 2015 netblue30 (netblue30@yahoo.com)
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
#include "../include/common.h"

#define USELOCK
#define FIREJAIL_DIR	"/tmp/firejail"
#define RO_DIR	"/tmp/firejail/firejail.ro.dir"
#define RO_FILE	"/tmp/firejail/firejail.ro.file"
#define MNT_DIR	"/tmp/firejail/mnt"
#define OVERLAY_DIR	"/tmp/firejail/overlay"
#define MAX_INCLUDE_LEVEL 6



// main.c
typedef struct bridge_t {
	// on the host
	char *dev;		// interface device name: bridge or regular ethernet
	uint32_t ip;		// interface device IP address
	uint32_t mask;		// interface device mask
	
	// inside the sandbox
	char *devsandbox;	// name of the device inside the sandbox
	uint32_t ipsandbox;	// ipaddress inside the sandbox
	
	// flags
	uint8_t arg_ip_none;	// --ip=none
	uint8_t macvlan;	// set by --net=eth0 (or eth1, ...); reset by --net=br0 (or br1, ...)
	uint8_t configured;
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
	ProfileEntry *profile;
	char *chrootdir;	// chroot directory
	char *home_private;	// private home directory
	char *cwd;		// current working directory

	// networking
	char *hostname;
	uint32_t defaultgw;	// default gateway
	Bridge bridge0;
	Bridge bridge1;
	Bridge bridge2;
	Bridge bridge3;

	// rlimits
	unsigned rlimit_nofile;
	unsigned rlimit_nproc;
	unsigned rlimit_fsize;
	unsigned rlimit_sigpending;
	
	// cpu affinity and control groups
	uint32_t cpus;
	char *cgroup;
	

	// command line
	char *command_line;
	char *command_name;
	char *shell;
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
extern int arg_command;	// -c
extern int arg_overlay;		// --overlay
extern int arg_zsh;		// use zsh as default shell
extern int arg_csh;		// use csh as default shell
extern int arg_seccomp;	// enable seccomp filter
extern char *arg_seccomp_list;//  optional seccomp list
extern int arg_seccomp_empty;// start with an empty syscall list
extern int arg_caps_default_filter; // enable capabilities default filter
extern int arg_caps_drop_all;	// drop all capabilities
extern uint64_t arg_caps_custom_filter;	// set custom capabilities filter
extern int arg_trace;		// syscall tracing support
extern int arg_rlimit_nofile;	// rlimit nofile
extern int arg_rlimit_nproc;	// rlimit nproc
extern int arg_rlimit_fsize;	// rlimit fsize
extern int arg_rlimit_sigpending;// rlimit sigpending
extern int arg_nox11;		// kill the program if x11 unix domain socket is accessed
extern int arg_nodbus;		// kill the program if D-Bus is accessed
extern int arg_nogroups;	// disable supplementary groups
extern int fds[2];

#define MAX_ARGS 128		// maximum number of command arguments (argc)
extern char *fullargv[MAX_ARGS];
extern int fullargc;

// sandbox.c
int sandbox(void* sandbox_arg);

// network_main.c
void net_configure_bridge(Bridge *br, char *dev_name);
void net_configure_sandbox_ip(Bridge *br);
void net_configure_veth_pair(Bridge *br, const char *ifname, pid_t child);
void net_bridge_wait_ip(Bridge *br);
void net_check_cfg(void);

// network.c
void net_if_up(const char *ifname);
void net_if_ip(const char *ifname, uint32_t ip, uint32_t mask);
int net_get_bridge_addr(const char *bridge, uint32_t *ip, uint32_t *mask);
int net_add_route(uint32_t dest, uint32_t mask, uint32_t gw);
void net_ifprint(void);
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
int profile_find(const char *name, const char *dir);
// read a profile file
void profile_read(const char *fname);
// check profile line; if line == 0, this was generated from a command line option
// return 1 if the command is to be added to the linked list of profile commands
// return 0 if the command was already executed inside the function
int profile_check_line(char *ptr, int lineno);
// add a profile entry in cfg.profile list; use str to populate the list
void profile_add(char *str);

// list.c
void list(void);
void tree(void);
void top(void);

// usage.c
void usage(void);

// join.c
void join(pid_t pid, const char *homedir, int argc, char **argv, int index);
void join_name(const char *name, const char *homedir, int argc, char **argv, int index);
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
int net_create_macvlan(const char *dev, const char *parent, unsigned pid);

// util.c
void drop_privs(int nogroups);
void extract_command_name(const char *str);
void logsignal(int s);
void logmsg(const char *msg);
void logargs(int argc, char **argv) ;
void logerr(const char *msg);
int copy_file(const char *srcname, const char *destname);
char *get_link(const char *fname);
int is_dir(const char *fname);
int is_link(const char *fname);
char *line_remove_spaces(const char *buf);
char *split_comma(char *str);
int not_unsigned(const char *str);
int find_child(pid_t parent, pid_t *child);
void check_private_dir(void);

// fs_var.c
void fs_var_log(void);	// mounting /var/log
void fs_var_lib(void);	// various other fixes for software in /var directory
void fs_var_cache(void); // various other fixes for software in /var/cache directory
void fs_var_run(void);
void fs_var_lock(void);
void fs_var_tmp(void);
void fs_var_utmp(void);
void dbg_test_dir(const char *dir);

// fs_dev.c
void fs_dev_shm(void);

// fs_home.c
// private mode: mount tmpfs over /home and /tmp
void fs_private(void);
void fs_private_home(void);


// seccomp.c
int seccomp_filter(void);
void seccomp_set(void);

// caps.c
int caps_default_default(void);
void caps_print(void);
void caps_drop_all(void);
void caps_set(uint64_t caps);

// syscall.c
const char *syscall_find_nr(int nr);
// return -1 if error, 0 if no error
int syscall_check_list(const char *slist, void (*callback)(int));
// print all available syscalls
void syscall_print(void);

// fs_trace.c
void fs_trace_preload(void);
void fs_trace(void);

// fs_hostname.c
void fs_hostname(const char *hostname);

// rlimit.c
void set_rlimits(void);

// cpu.c
void read_cpu_list(const char *str);
void set_cpu_affinity(void);
void load_cpu(const char *fname);
void save_cpu(void);

// cgroup.c
void save_cgroup(void);
void load_cgroup(const char *fname);
void set_cgroup(const char *path);

// output.c
void check_output(int argc, char **argv);

#endif