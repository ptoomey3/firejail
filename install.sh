#!/bin/bash

strip src/firejail/firejail
mkdir -p $1/bin
install -c -m 0755 src/firejail/firejail $1/bin/.
chmod u+s $1/bin/firejail
strip src/firemon/firemon
install -c -m 0755 src/firemon/firemon $1/bin/.
mkdir -p $1/share/doc/firejail
install -c -m 0644 COPYING $1/share/doc/firejail/.
install -c -m 0644 README $1/share/doc/firejail/.
install -c -m 0644 RELNOTES $1/share/doc/firejail/.
mkdir -p /etc/firejail
install -c -m 0644 etc/firefox.profile /etc/firejail/.
install -c -m 0644 etc/firefox.profile /etc/firejail/iceweasel.profile
rm -f firejail.1.gz
gzip -9 firejail.1
mkdir -p $1/share/man/man1
install -c -m 0644 firejail.1.gz $1/share/man/man1/.	
rm -f firejail.1.gz
if [ ! -f /etc/firejail/sshd.users ]
then
	install -c -m 0644 etc/sshd.users /etc/firejail/.
fi
