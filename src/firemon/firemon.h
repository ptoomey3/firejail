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
#include "../include/pid.h"

#define BUFLEN 4096

#define errExit(msg)    do { char msgout[500]; sprintf(msgout, "Error %s %s %d", msg, __FUNCTION__, __LINE__); perror(msgout); exit(1);} while (0)

// clear screen
static inline firemon_clrscr(void) {
	printf("\033[2J\033[1;1H");
	fflush(0);
}

// firemon.c
void firemon_drop_privs(void);
void firemon_sleep(int st);


// procevent.c
void procevent(pid_t pid);

// usage.c
void usage(void);

// list.c
void list(pid_t pid);
void list_mem(pid_t pid);
void list_cpu(pid_t pid);
void list_uptime(pid_t pid);

// top.c
void top(void);

#endif
