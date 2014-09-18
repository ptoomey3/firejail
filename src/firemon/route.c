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

typedef struct  iflist_t {
	struct iflist_t *next;
	uint32_t ip;
} IfList;
static IfList *ifs = NULL;
static char last_start[MAXBUF];

static IfList *list_find(uint32_t ip, uint32_t mask) {
	IfList *ptr = ifs;
	while (ptr) {
		if ((ptr->ip & mask) == (ip & mask))
			return ptr;
		ptr = ptr->next;
	}
	
	return NULL;
}

static void extract_if(const char *fname) {
	// clear interface list
	while (ifs) {
		IfList *tmp = ifs->next;
		free(ifs);
		ifs = tmp;
	}
	assert(ifs == NULL);	
	
	FILE *fp = fopen(fname, "r");
	if (!fp)
		return;
	
	char buf[MAXBUF];
	int state = 0;	// 0 -wait for Local
			// 
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
			
		if (state == 0) {
			if (strncmp(buf, "Local:", 6) == 0) {
				state = 1;
				continue;
			}
		}
		else if (state == 1) {
			// remove broadcast addresses
			if (strstr(start,"BROADCAST"))
				continue;
			else if (*start == '+')
				continue;
			else if (*start == '|') {
				strcpy(last_start, start);
				continue;
			}
			else if (strstr(buf, "LOCAL")) {
//				printf("%s %s\n", last_start, start);
				unsigned mbits;
				sscanf(start, "/%u", &mbits);
				if (mbits != 32)
					continue;

				unsigned a, b, c, d;
				if (sscanf(last_start, "|-- %u.%u.%u.%u", &a, &b, &c, &d) != 4 || a > 255 || b > 255 || c > 255 || d > 255)
					continue;

				IfList *newif = malloc(sizeof(IfList));
				if (!newif)
					errExit("malloc");
				newif->ip = a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
				newif->next = ifs;
				ifs = newif;
			}
		}
	}
	
	fclose(fp);


}

static void print_route(const char *fname) {
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
		//Iface	Destination	Gateway 	Flags	RefCnt	Use	Metric	Mask		MTU	Window	IRTT
		if (strncmp(start, "Iface", 5) == 0)
			continue;

		// extract data
		char ifname[64];
		char destination[64];
		char gateway[64];
		char flags[64];
		char refcnt[64];
		char use[64];
		char metric[64];
		char mask[64];
		int rv = sscanf(start, "%s %s %s %s %s %s %s %s\n", ifname, destination, gateway, flags, refcnt, use, metric, mask);
		if (rv != 8)
			continue;
		
		// destination ip
		uint32_t destip;
		sscanf(destination, "%x", &destip);
		destip = ntohl(destip);
		uint32_t destmask;
		sscanf(mask, "%x", &destmask);
		destmask = ntohl(destmask);
		uint32_t gw;
		sscanf(gateway, "%x", &gw);
		gw = ntohl(gw);
		
//		printf("#%s# #%s# #%s# #%s# #%s# #%s# #%s# #%s#\n", ifname, destination, gateway, flags, refcnt, use, metric, mask);
		if (gw != 0)
			printf("  %u.%u.%u.%u/%u via %u.%u.%u.%u, dev %s, metric %s\n",
				PRINT_IP(destip), mask2bits(destmask),
				PRINT_IP(gw),
				ifname,
				metric);
		else { // this is an interface
			IfList *ifentry = list_find(destip, destmask);
			if (ifentry) {
				printf("  %u.%u.%u.%u/%u, dev %s, scope link src %d.%d.%d.%d\n",
					PRINT_IP(destip), mask2bits(destmask),
					ifname,
					PRINT_IP(ifentry->ip));
			}
			else
				printf("here %d\n", __LINE__);
		}
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


void route(void) {
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
				if (!asprintf(&fname, "/proc/%d/net/fib_trie", child) == -1)
					errExit("asprintf");
				extract_if(fname);
				free(fname);

				if (!asprintf(&fname, "/proc/%d/net/route", child) == -1)
					errExit("asprintf");
				print_route(fname);
				free(fname);
				printf("\n");
			}
		}
	}
}


