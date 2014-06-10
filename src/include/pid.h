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
#ifndef PID_H
#define PID_H
#define MAX_PIDS 32769

#include <sys/types.h>
#include <unistd.h>
typedef struct {
	short level;  // -1 not a firejail process, 0 not investigated yet, 1 firejail process, > 1 firejail child
	unsigned char zombie;
	pid_t parent;
	uid_t uid;
	char *user;
	char *cmd;
} Process;
extern Process pids[MAX_PIDS];

uid_t pid_get_uid(pid_t pid);
char *pid_proc_cmdline(const pid_t pid);
char *pid_get_user_name(uid_t uid);
int pid_is_firejail(pid_t pid);
void pid_print_tree(unsigned index, unsigned parent, int nowrap);
void pid_read(void);

#endif
