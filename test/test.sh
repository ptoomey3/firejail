#!/bin/bash

echo "*** TESTING system configuration ***"
./chk_config.exp

echo "TESTING: version"
./option_version.exp

echo "TESTING: help"
./option_help.exp

echo "TESTING: man"
./option_man.exp

echo "TESTING: list"
./option_list.exp

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

echo "TESTING: private"
./private.exp

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
