#!/bin/bash

echo "*** TESTING system configuration ***"
./chk_config.exp

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

echo "TESTING: network no IP"
./net_noip.exp

echo "TESTING: network default gateway"
./net_defaultgw.exp

echo "TESTING: 4 bridges ARP"
./4bridges_arp.exp

echo "TESTING: 4 bridges IP"
./4bridges_ip.exp

echo "TESTING: login SSH"
./login_ssh.exp

echo "TESTING: ARP"
./net_arp.exp
