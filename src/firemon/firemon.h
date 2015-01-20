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
#ifndef FIREMON_H
#define FIREMON_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <linux/cn_proc.h>
#include <stdint.h>
#include "../include/pid.h"
#include "../include/common.h"

// clear screen
static inline void firemon_clrscr(void) {
	printf("\033[2J\033[1;1H");
	fflush(0);
}

// firemon.c
int find_child(int id);
void firemon_drop_privs(void);
void firemon_sleep(int st);


// procevent.c
void procevent(pid_t pid);

// usage.c
void usage(void);

// top.c
void top(void);

// list.c
void list(void);

// interface.c
void interface(pid_t pid);

// arp.c
void arp(pid_t pid);

// route.c
void route(pid_t pid);

// caps.c
void caps(void);

// seccomp.c
void seccomp(void);

// cpu.c
void cpu(void);

// cgroup.c
void cgroup(void);

#endif
