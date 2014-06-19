#!/bin/bash

echo "*** TESTING system configuration ***"
./chk_config.exp

echo "TESTING: servers rsyslogd, sshd, nginx"
./servers.exp

if [ -f /etc/init.d/snmpd ]
then
	echo "TESTING: servers snmpd"
	./servers2.exp
fi

if [ -f /etc/init.d/apache2 ]
then
	echo "TESTING: servers apache2"
	./servers3.exp
fi

if [ -f /etc/init.d/isc-dhcp-server ]
then
	echo "TESTING: servers isc dhcp server"
	./servers4.exp
fi
