#!/bin/bash

echo "*********************************TESTING system configuration START***"
./chk_config.exp

echo "TESTING: version"
./option_version.exp

echo "TESTING: help"
./option_help.exp

echo "TESTING: man"
./option_man.exp

echo "TESTING: list"
./option_list.exp

echo "TESTING: tree"
./option_list.exp

echo "TESTING: zsh"
./shell_zsh.exp

echo "TESTING: csh"
./shell_csh.exp

grep "Ubuntu" /etc/os-release
if [ "$?" -eq 0 ];
then
	echo "TESTING: firefox"
	./firefox.exp
fi

echo "TESTING: PID"
./pid.exp

echo "TESTING: profile no permissions"
./profile_noperm.exp

echo "TESTING: profile syntax"
./profile_syntax.exp

echo "TESTING: profile read-only"
./profile_readonly.exp

echo "TESTING: profile tmpfs"
./profile_tmpfs.exp

echo "TESTING: profile applications"
./profile_apps.exp

echo "TESTING: private"
./private.exp `whoami`

grep "openSUSE" /etc/os-release
if [ "$?" -eq 0 ];
then
	echo "TESTING: overlayfs"
	./fs_overlay.exp
fi

grep "Ubuntu" /etc/os-release
if [ "$?" -eq 0 ];
then
	echo "TESTING: overlayfs"
	./fs_overlay.exp
fi

echo "TESTING: seccomp su"
./seccomp-su.exp

echo "TESTING: seccomp ptrace"
./seccomp-ptrace.exp

echo "TESTING: chroot"
./fs_chroot.exp

echo "TESTING: read/write /var/tmp"
./fs_var_tmp.exp

echo "TESTING: read/write /var/lock"
./fs_var_lock.exp

echo "TESTING: read/write /dev/shm"
./fs_dev_shm.exp

echo "TESTING: local network"
./net_local.exp

echo "TESTING: no network"
./net_none.exp

echo "TESTING: network IP"
./net_ip.exp

echo "TESTING: network bad IP"
./net_badip.exp

echo "TESTING: network no IP test 1"
./net_noip.exp

echo "TESTING: network no IP test 2"
./net_noip2.exp

echo "TESTING: network default gateway test 1"
./net_defaultgw.exp

echo "TESTING: network default gateway test 2"
./net_defaultgw2.exp

echo "TESTING: network default gateway test 3"
./net_defaultgw3.exp

echo "TESTING: 4 bridges ARP"
./4bridges_arp.exp

echo "TESTING: 4 bridges IP"
./4bridges_ip.exp

echo "TESTING: login SSH"
./login_ssh.exp

echo "TESTING: ARP"
./net_arp.exp
