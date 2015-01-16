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

void usage(void) {
	printf("firejail - version %s\n\n", VERSION);
	printf("Firejail is a SUID sandbox program that reduces the risk of security breaches by\n");
	printf("restricting the running environment of untrusted applications using Linux\n");
	printf("namespaces. It includes a sandbox profile for Mozilla Firefox.\n\n");
	printf("Usage: firejail [options] [program and arguments]\n\n");
	printf("Without any options, the sandbox consists of a filesystem chroot build from the\n");
	printf("current system directories  mounted  read-only,  and  new PID and IPC\n");
	printf("namespaces. If no program is specified as an argument, /bin/bash is started by\n");
	printf("default in the sandbox.\n\n");
	printf("Options:\n");
	printf("\t--bind=dirname1,dirname2 - mount-bind dirname1 on top of dirname2.\n");
	printf("\t--bind=filename1,dirname2 - mount-bind filename1 on top of filename2.\n");
	printf("\t--blacklist=dirname_or_filename - blacklist directory or file.\n");
	printf("\t-c - execute command and exit.\n");
	printf("\t--caps - enable Linux capabilities filter.\n");
	printf("\t--cgroup=tasks-file - place the sandbox in the specified control group.\n");
	printf("\t\ttasks-file is the full path of cgroup tasks file.\n");
	printf("\t\tExample: --cgroup=/sys/fs/cgroup/g1/tasks\n");
	printf("\t--chroot=dirname - chroot into dirname directory.\n");
	printf("\t--cpu=cpu-number,cpu-number - set cpu affinity.\n");
	printf("\t\tExample: cpu=0,1,2\n");
	printf("\t--csh - use /bin/csh as default shell.\n");
	printf("\t--debug - print sandbox debug messages.\n");
	printf("\t--debug-caps - print capabilities for the current session and exit.\n");
	printf("\t--debug-syscalls - print all recognized system calls in the current\n");
	printf("\t\tFirejail software build and exit.\n");
	printf("\t--defaultgw=address - use this address as default gateway in the new\n");
	printf("\t\tnetwork namespace.\n");
	printf("\t--help, -? - this help screen.\n");
	printf("\t--ip=address - use this IP address in the new network namespace.\n");
	printf("\t--ipc-namespace - enable a new IPC namespace if the sandbox was started\n");
	printf("\t\tas a regular user. IPC namespace is enabled by default only if\n");
	printf("\t\tthe sandbox is started as root.\n");
	printf("\t--join=name - join the sandbox started using --name option.\n");
	printf("\t--join=pid - join the sandbox of the specified process.\n");
	printf("\t--list - list all sandboxes.\n");
	printf("\t--name=name - set sandbox hostname.\n");
	printf("\t--net=bridgename - enable network namespaces and connect to this bridge\n");
	printf("\t\tdevice. Unless specified with option --ip and --defaultgw, an\n");
	printf("\t\tIP address and a default gateway will be assigned automatically\n");
	printf("\t\tto the sandbox. The IP address is checked using ARP before\n");
	printf("\t\tassignment. The IP address assigned as default gateway is the\n");
	printf("\t\tbridge device IP address. Up to four --net bridge devices can\n");
	printf("\t\tbe defined.\n");
	printf("\t--net=none - enable a new, unconnected network namespace.\n");
	printf("\t--noip - no IP address and no default gateway address are configured in\n");
	printf("\t\tthe new network namespace. Use this option in case you intend\n");
	printf("\t\tto start a DHCP client in the sandbox.\n");
	printf("\t--overlay - mount a filesystem overlay on top of the current filesystem.\n");
	printf("\t\t(OverlayFS support is required in Linux kernel for this option\n");
	printf("\t\tto work)\n");    
	printf("\t--private - mount new /root and /home/user directories.\n");
	printf("\t--private=directory - use directory as user home.\n");
	printf("\t--profile=filename - use a custom profile.\n");
	printf("\t--read-only=dirname_or_filename - set directory or file read-only.\n");
	printf("\t--rlimit-fsize=number - set the maximum file size that can be created\n");
	printf("\t\tby a process.\n");
	printf("\t--rlimit-nofile=number - set the maximum number of files that can be\n");
	printf("\t\topened by a process.\n");
	printf("\t--rlimit-nproc=number - set the maximum number of processes that can be\n");
	printf("\t\tcreated for the real user ID of the calling process.\n");
	printf("\t--rlimit-sigpending=number - set the maximum number of pending signals\n");
	printf("\t\tfor a process.\n");
#ifdef HAVE_SECCOMP
	printf("\t--seccomp - enable seccomp filter and disable the syscalls in the\n");
	printf("\t\tlist. The default list is as follows: mount, umount2,\n");
	printf("\t\tptrace, kexec_load, open_by_handle_at, init_module,\n");
	printf("\t\tfinit_module, delete_module, iopl, ioperm, swapon, swapoff\n");
	printf("\t\tand syslog.\n");
	printf("\t--seccomp=syscall,syscall,syscall - enable seccomp filter, apply the\n");
	printf("\t\tdefault syscall list and the syscalls specified by the command.\n");
#endif
	printf("\t--shell=program - set default user shell.\n");
	printf("\t--shutdown=name - shutdown the sandbox started using --name option.\n");
	printf("\t--shutdown=pid - shutdown the sandbox specified by pid.\n");
	printf("\t--tmpfs=dirname - mount a tmpfs filesystem on directory dirname.\n");
	printf("\t--top - monitor the most CPU-intensive sandboxes.\n");
	printf("\t--trace - trace open, access and connect system calls.\n");
	printf("\t--tree - print a tree of all sandboxed processes.\n");
	printf("\t--version - print program version and exit.\n");
	printf("\t--zsh - use /usr/bin/zsh as default shell.\n");
	printf("\n");

	printf("Monitoring\n\n");

	printf("Option --list prints a list of all sandboxes. The format for each entry is as\n");
	printf("follows:\n\n");
	printf("\tPID:USER:Command\n\n");

	printf("Option --tree prints the tree of processes running in the sandbox. The format\n");
	printf("for each process entry is as follows:\n\n");
	printf("\tPID:USER:Command\n\n");

	printf("Option --top is similar to the UNIX top command, however it applies only to\n");
	printf("sandboxes. Listed below are the available fields (columns) in alphabetical\n");
	printf("order:\n\n");
	printf("\tCommand - command used to start the sandbox.\n");
	printf("\tCPU%% - CPU usage, the sandbox share of the elapsed CPU time since the\n");
	printf("\t       last screen update\n");
	printf("\tPID - Unique process ID for the task controlling the sandbox.\n");
	printf("\tPrcs - number of processes running in sandbox, including the controlling\n");
	printf("\t       process.\n");
	printf("\tRES - Resident Memory Size (KiB), sandbox non-swapped physical memory.\n");
	printf("\t      It is a sum of the RES values for all processes running in the\n");
	printf("\t      sandbox.\n");
	printf("\tSHR - Shared Memory Size (KiB), it reflects memory shared with other\n");
	printf("\t      processes. It is a sum of the SHR values for all processes running\n");
	printf("\t      in the sandbox, including the controlling process.\n");
	printf("\tUptime - sandbox running time in hours:minutes:seconds format.\n");
	printf("\tUser - The owner of the sandbox.\n");
	printf("\n");
	printf("Profile files\n\n");
	printf("Several command line configuration options can be passed to the program using\n");
	printf("profile files. Default Firejail profile files are stored in /etc/firejail\n");
	printf("directory, user profile files are stored in ~/.config/firejail directory. See\n");
	printf("man 5 firejail-profile for more information.\n\n");
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
	printf("   $ firejail --seccomp firefox\n");
	printf("          start Mozilla Firefox in a seccomp sandbox\n");
	printf("   $ firejail --caps firefox\n");
	printf("          start Mozilla Firefox in a Linux capabilities sandbox\n");
	printf("   $ firejail --debug firefox\n");
	printf("          debug Firefox sandbox\n");
	printf("   $ firejail --private\n");
	printf("          start a /bin/bash session with a new tmpfs home directory\n");
	printf("   $ firejail --net=br0 ip=10.10.20.10\n");
	printf("          start a /bin/bash session in a new network namespace; the session is\n");
	printf("          connected to the main network using br0 bridge device, an IP address\n");
	printf("          of 10.10.20.10 is assigned to the sandbox\n");
	printf("   $ firejail --net=br0 --net=br1 --net=br2\n");
	printf("          start a /bin/bash session in a new network namespace and connect it\n");
	printf("          to br0, br1, and br2 host bridge devices\n");
	printf("   $ firejail --list\n");
	printf("          list all running sandboxes\n");
	printf("\n");
	printf("License GPL version 2 or later\n");
	printf("Homepage: http://firejail.sourceforge.net\n");
	printf("\n");
}
