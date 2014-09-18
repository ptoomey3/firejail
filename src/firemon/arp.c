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
#include "firemon.h"
#include "../include/pid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#define MAXBUF 4096

static void print_arp(const char *fname) {
	FILE *fp = fopen(fname, "r");
	if (!fp)
		return;
	
	char buf[MAXBUF];
	while (fgets(buf, MAXBUF, fp)) {
		// remove blanks, \n
		char *ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		char *start = ptr;
		if (*start == '\0')
			continue;
		ptr = strchr(ptr, '\n');
		if (ptr)
			*ptr = '\0';

		// remove table header
		//IP address       HW type     Flags       HW address            Mask     Device
		if (strncmp(start, "IP address", 10) == 0)
			continue;

		// extract data
		char ip[64];
		char type[64];
		char flags[64];
		char mac[64];
		char mask[64];
		char device[64];
		int rv = sscanf(start, "%s %s %s %s %s %s\n", ip, type, flags, mac, mask, device);
		if (rv != 6)
			continue;
		
		// destination ip
		unsigned a, b, c, d;
		if (sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d) != 4 || a > 255 || b > 255 || c > 255 || d > 255)
			continue;
		uint32_t destip = a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
		if (strcmp(flags, "0x0") == 0)
			printf("  %d.%d.%d.%d dev %s FAILED\n",
				PRINT_IP(destip), device);
		else
			printf("  %d.%d.%d.%d dev %s lladdr %s REACHABLE\n",
				PRINT_IP(destip), device, mac);
	}
	
	fclose(fp);

}

// return -1 if not found
static int find_child(int id) {
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 2 && pids[i].parent == id)
			return i;
	}
	
	return -1;
}


void arp(void) {
	if (getuid() == 0)
		firemon_drop_privs();
	
	pid_read(0);	// include all processes
	
	// print processes
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 1) {
			pid_print_list(i, 0);
			int child = find_child(i);
			if (child != -1) {
				char *fname;
				if (!asprintf(&fname, "/proc/%d/net/arp", child) == -1)
					errExit("asprintf");
				print_arp(fname);
				free(fname);
				printf("\n");
			}
		}
	}
}


