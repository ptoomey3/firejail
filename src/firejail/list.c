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
#include "firejail.h"
#include "../include/pid.h"

static void drop_privs(void) {
	// drop privileges
	if (setuid(getuid()) < 0)
		errExit("setuid/getuid");
	if (setgid(getgid()) < 0)
		errExit("setgid/getgid");
}

void list(void) {
	drop_privs();
	pid_read(0);	// include all processes
	
	// print processes
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 1)
			pid_print_tree(i, 0, 0);
	}
}

void list_mem(void) {
	drop_privs();
	pid_read(0);	// include all processes
	pid_print_mem_header();
	
	// print processes
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 1)
			pid_print_mem(i, 0);
	}
}

void list_cpu(void) {
	drop_privs();
	pid_read(0);	// include all processes
		
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		unsigned utime;
		unsigned stime;
		if (pids[i].level == 1)
			pid_store_cpu(i, 0, &utime, &stime);
	}
	sleep(5);
	pid_print_cpu_header();
	for (i = 0; i < MAX_PIDS; i++) {
		unsigned utime;
		unsigned stime;
		if (pids[i].level == 1)
			pid_print_cpu(i, 0, &utime, &stime, 5);
	}
}
