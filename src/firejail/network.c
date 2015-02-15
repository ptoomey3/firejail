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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/route.h>
#include <linux/if_bridge.h>

// scan interfaces in current namespace and print IP address/mask for each interface
void net_ifprint(void) {
	uint32_t ip;
	uint32_t mask;
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1)
		errExit("getifaddrs");

	printf("%-20.20s%-20.20s%-20.20s%-20.20s\n",
		"Interface", "IP", "Mask", "Status");
	// walk through the linked list
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *si = (struct sockaddr_in *) ifa->ifa_netmask;
			mask = ntohl(si->sin_addr.s_addr);
			si = (struct sockaddr_in *) ifa->ifa_addr;
			ip = ntohl(si->sin_addr.s_addr);

			char *status;
			if (ifa->ifa_flags & IFF_RUNNING && ifa->ifa_flags & IFF_UP)
				status = "UP";
			else
				status = "DOWN";

			char ipstr[30];
			sprintf(ipstr, "%d.%d.%d.%d", PRINT_IP(ip));
			char maskstr[30];
			sprintf(maskstr, "%d.%d.%d.%d", PRINT_IP(mask));
			printf("%-20.20s%-20.20s%-20.20s%-20.20s\n",
				ifa->ifa_name, ipstr, maskstr, status);
		}
	}
	printf("\n");
	freeifaddrs(ifaddr);
}


// return -1 if the bridge was not found; if the bridge was found retrun 0 and fill in IP address and mask
int net_get_bridge_addr(const char *bridge, uint32_t *ip, uint32_t *mask) {
	assert(bridge);
	assert(ip);
	assert(mask);
	int rv = -1;
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1)
		errExit("getifaddrs");

	// walk through the linked list; if the interface is found, extract IP address and mask
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		if (strcmp(ifa->ifa_name, bridge) != 0)
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *si = (struct sockaddr_in *) ifa->ifa_netmask;
			*mask = ntohl(si->sin_addr.s_addr);
			si = (struct sockaddr_in *) ifa->ifa_addr;
			*ip = ntohl(si->sin_addr.s_addr);
			rv = 0;
			break;
		}
	}

	freeifaddrs(ifaddr);
	return rv;
}

// bring interface up
void net_if_up(const char *ifname) {
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
		errExit("socket");

	// get the existing interface flags
	struct ifreq ifr;
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;

	// read the existing flags
	if (ioctl(sock, SIOCGIFFLAGS, &ifr ) < 0) {
		close(sock);
		errExit("ioctl");
	}

	ifr.ifr_flags |= IFF_UP;

	// set the new flags
	if (ioctl( sock, SIOCSIFFLAGS, &ifr ) < 0) {
		close(sock);
		errExit("ioctl");
	}

	// checking
	// read the existing flags
	if (ioctl(sock, SIOCGIFFLAGS, &ifr ) < 0) {
		close(sock);
		errExit("ioctl");
	}

	// wait not more than 500ms for the interface to come up
	int cnt = 0;
	while (cnt < 50) {
		usleep(10000);			  // sleep 10ms

		// read the existing flags
		if (ioctl(sock, SIOCGIFFLAGS, &ifr ) < 0) {
			close(sock);
			errExit("ioctl");
		}
		if (ifr.ifr_flags & IFF_RUNNING)
			break;
		cnt++;
	}

	close(sock);
}

// configure interface
void net_if_ip( const char *ifname, uint32_t ip, uint32_t mask) {
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
		errExit("socket");

	struct ifreq ifr;
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;

	((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = htonl(ip);
	if (ioctl( sock, SIOCSIFADDR, &ifr ) < 0) {
		close(sock);
		errExit("ioctl");
	}

	if (ip != 0) {
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr =  htonl(mask);
		if (ioctl( sock, SIOCSIFNETMASK, &ifr ) < 0) {
			close(sock);
			errExit("ioctl");
		}
	}

	close(sock);
	usleep(10000);				  // sleep 10ms
}


// add an IP route, return -1 if error, 0 if the route was added
int net_add_route(uint32_t ip, uint32_t mask, uint32_t gw) {
	int sock;
	struct rtentry route;
	struct sockaddr_in *addr;
	int err = 0;

	// create the socket
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errExit("socket");

	memset(&route, 0, sizeof(route));

	addr = (struct sockaddr_in*) &route.rt_gateway;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(gw);

	addr = (struct sockaddr_in*) &route.rt_dst;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(ip);

	addr = (struct sockaddr_in*) &route.rt_genmask;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(mask);

	route.rt_flags = RTF_UP | RTF_GATEWAY;
	route.rt_metric = 0;
	if ((err = ioctl(sock, SIOCADDRT, &route)) != 0) {
		close(sock);
		return -1;
	}

	return 0;
}


// add a veth device to a bridge
void net_bridge_add_interface(const char *bridge, const char *dev) {
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);

	if (ifindex <= 0)
		errExit("if_nametoindex");

	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
              	errExit("socket");

	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRADDIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(sock, SIOCBRADDIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };

		ifr.ifr_data = (char *) args;
		err = ioctl(sock, SIOCDEVPRIVATE, &ifr);
	}
	(void) err;
}

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

