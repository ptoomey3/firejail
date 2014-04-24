#!/bin/bash

# firejail executable
strip src/firejail/firejail
mkdir -p $1/bin
install -c -m 0755 src/firejail/firejail $1/bin/.
chmod u+s $1/bin/firejail

# firemon executable
strip src/firemon/firemon
install -c -m 0755 src/firemon/firemon $1/bin/.

# documents
mkdir -p $1/share/doc/firejail
install -c -m 0644 COPYING $1/share/doc/firejail/.
install -c -m 0644 README $1/share/doc/firejail/.
install -c -m 0644 RELNOTES $1/share/doc/firejail/.

# etc files
mkdir -p /etc/firejail
install -c -m 0644 etc/firefox.profile /etc/firejail/.
install -c -m 0644 etc/firefox.profile /etc/firejail/iceweasel.profile
if [ ! -f /etc/firejail/login.users ]
then
	install -c -m 0644 etc/login.users /etc/firejail/.
fi

# man pages
rm -f firejail.1.gz
gzip -9 firejail.1
rm -f firemon.1.gz
gzip -9 firemon.1
mkdir -p $1/share/man/man1
install -c -m 0644 firejail.1.gz $1/share/man/man1/.	
install -c -m 0644 firemon.1.gz $1/share/man/man1/.	
rm -f firejail.1.gz firemon.1.gz

# bash completion
mkdir -p $1/share/bash-completion/completions
install -c -m 0644 etc/firejail.bash_completion $1/share/bash-completion/completions/firejail
