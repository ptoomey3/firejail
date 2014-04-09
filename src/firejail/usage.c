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

void usage(void) {
	printf("firejail - version %s\n\n", VERSION);
	printf("Firejail is a SUID sandbox program that reduces the risk of security breaches by\n");
	printf("restricting the running environment of untrusted applications using Linux\n");
	printf("namespaces. It includes a sandbox profile for Mozilla Firefox.\n\n");
	printf("Usage: firejail [options] [program and arguments]\n");
	printf("Without any options, the sandbox consists of a filesystem chroot build from the\n");
	printf("current system directories  mounted  read-only,  and  new PID and IPC\n");
	printf("namespaces.\n\n");
	printf("If no program is specified as an argument, /bin/bash is started by default in\n");
	printf("the sandbox.\n\n");
	printf("Options:\n");
	printf("\t-c - execute command and exit\n");
	printf("\t--chroot=dirname - chroot into dirname directory\n");
	printf("\t--debug - print sandbox debug messages\n");
	printf("\t--help, -? - this help screen\n");
	printf("\t--ip=ipaddress - use this IP address in the new network namespace\n");
	printf("\t--join=pid - join the sandbox of the specified process\n");
	printf("\t--name=name - set sandbox hostname\n");
	printf("\t--net=bridgename - enable network namespaces and connect to this bridge\n");
	printf("\t\tdevice\n");
	printf("\t--net=none - enable a new, unconnected network namespace\n");
	printf("\t--overlay - mount a filesystem overlay on top of the current filesystem\n");
	printf("\t            (OverlayFS support is required in Linux kernel for this\n");
	printf("\t            option to work)\n");    
	printf("\t--private - mount a new /home/user directory\n");
	printf("\t--profile=filename - use a custom profile\n");
	printf("\t--version - print program version and exit\n");
	printf("\n");
	printf("Profile files\n\n");
	printf("The profile files define a chroot filesystem built on top of the existing\n");
	printf("filesystem. Each line describes a file element that is removed from\n");
	printf("the filesystem, for example:\n");
	printf("\n");
	printf("# this is a comment\n");
	printf("blacklist /etc/password # remove /etc/password file\n");
	printf("blacklist /usr/bin # remove /usr/bin directory\n");
	printf("blacklist /usr/bin/gcc* # remove all gcc files in /usr/bin (file globbing)\n");
	printf("blacklist ${PATH}/ifconfig # remove ifconfig from the regular path directories\n");
	printf("blacklist ${HOME}/.ssh # remove .ssh directory from user home directory\n");
	printf("\n");
	printf("Default Firejail profile files are stored in /etc/firejail directory, user\n");
	printf("profile files are stored in ~/.config/firejail directory. See\n");
	printf("/etc/firejail/firefox.profile for more examples. \n\n");
	printf("Restricted shell\n\n");
	printf("To  configure a restricted shell, replace /bin/bash with /usr/bin/firejail i\n");
	printf("/etc/password file for each user that needs to  be  restricted.\n");
	printf("Alternatively, you can specify /usr/bin/firejail  in adduser command:\n\n");
	printf("   adduser --shell /usr/bin/firejail username\n\n");
	printf("Arguments to be passed to firejail executable upon login are  declared  in\n");
	printf("/etc/firejail/login.users file.\n\n");
	printf("Examples:\n\n");
	printf("   $ firejail\n");
	printf("          start a regular /bin/bash session in sandbox\n");
	printf("   $ firejail firefox\n");
	printf("          start Mozilla Firefox\n");
	printf("   $ firejail --debug firefox\n");
	printf("          debug Firefox sandbox\n");
	printf("   $ firejail --private\n");
	printf("          start a /bin/bash session with a new tmpfs home directory\n");
	printf("   $ firejail --net=br0 ip=10.10.20.10\n");
	printf("          start a /bin/bash session in a new network namespace; the session is\n");
	printf("          connected to the main network using br0 bridge device, an IP address\n");
	printf("          of 10.10.20.10 is assigned to the sandbox\n");
	printf("   $ firejail --list\n");
	printf("          list all running sandboxes\n");
	printf("\n");
	printf("Copyright @ 2014 netblue30@yahoo.com\n");
	printf("License GPL version 2 or later\n");
	printf("Homepage: http://firejail.sourceforge.net\n");
	printf("\n");
}
