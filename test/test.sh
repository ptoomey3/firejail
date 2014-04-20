#!/bin/bash

echo "TESTING: PID"
./pid.exp

echo "TESTING: profile no permissions"
./profile_noperm.exp

echo "TESTING: profile syntax"
./profile_syntax.exp

echo "TESTING: no network"
./net_none.exp

echo "TESTING: network IP"
./net_none.exp

echo "TESTING: ARP"
./net_arp.exp
