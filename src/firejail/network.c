#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/route.h>
#include "firejail.h"

int net_ifprint(void) {
	uint32_t ip;
	uint32_t mask;
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1)
		errExit("getifaddrs");

	
	printf("%-20.20s%-20.20s%-20.20s\n",
		"Interface", "IP", "Mask");
	// walk through the linked list
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *si = (struct sockaddr_in *) ifa->ifa_netmask;
			mask = ntohl(si->sin_addr.s_addr);
			si = (struct sockaddr_in *) ifa->ifa_addr;
			ip = ntohl(si->sin_addr.s_addr);

			char ipstr[30];
			sprintf(ipstr, "%d.%d.%d.%d", PRINT_IP(ip));
			char maskstr[30];
			sprintf(maskstr, "%d.%d.%d.%d", PRINT_IP(mask));
			printf("%-20.20s%-20.20s%-20.20s\n",
				ifa->ifa_name, ipstr, maskstr);
		}
	}
	printf("\n");
	freeifaddrs(ifaddr);
}




// return 1 if the bridge was found
int net_bridge_addr(const char *bridge, uint32_t *ip, uint32_t *mask) {
	assert(bridge);
	assert(ip);
	assert(mask);
	int rv = 1;
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1)
		errExit("getifaddrs");

	// walk through the linked list
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


void net_if_up(const char *ifname) {
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0)
		errExit("socket");

	// get the existing interface flags
	struct ifreq ifr;
	strcpy(ifr.ifr_name, ifname);
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
	close(sock);
}


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
}

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
	addr->sin_addr.s_addr = htonl(gw); //inet_addr("192.168.2.1");

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
		return 1;
	}
	
	return 0;

}
