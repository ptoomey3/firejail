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

#include "firejail.h"

#define BUFSIZE 1024
uint32_t network_get_defaultgw(void) {
	FILE *fp = fopen("/proc/self/net/route", "r");
	if (!fp)
		errExit("fopen");
	
	char buf[BUFSIZE];
	uint32_t retval = 0;
	while (fgets(buf, BUFSIZE, fp)) {
		if (strncmp(buf, "Iface", 5) == 0)
			continue;
		
		char *ptr = buf;
		while (*ptr != ' ' && *ptr != '\t')
			ptr++;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
			
		unsigned dest;
		unsigned gw;
		int rv = sscanf(ptr, "%x %x", &dest, &gw);
		if (rv == 2 && dest == 0) {
			retval = ntohl(gw);
			break;
		}
	}

	fclose(fp);
	return retval;
}

		
		
