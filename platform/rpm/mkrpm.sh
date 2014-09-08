#!/bin/bash
VERSION="0.9.12"
rm -fr ~/rpmbuild
rm -f firejail-$VERSION-1.x86_64.rpm

mkdir -p ~/rpmbuild/{RPMS,SRPMS,BUILD,SOURCES,SPECS,tmp}
cat <<EOF >~/.rpmmacros
%_topdir   %(echo $HOME)/rpmbuild
%_tmppath  %{_topdir}/tmp
EOF

cd ~/rpmbuild

mkdir -p firejail-$VERSION/usr/bin
install -m 755 /usr/bin/firejail firejail-$VERSION/usr/bin/.
install -m 755 /usr/bin/firemon firejail-$VERSION/usr/bin/.

mkdir -p firejail-$VERSION/usr/share/man/man1
install -m 644 /usr/share/man/man1/firejail.1.gz firejail-$VERSION/usr/share/man/man1/.
install -m 644 /usr/share/man/man1/firemon.1.gz firejail-$VERSION/usr/share/man/man1/.

mkdir -p firejail-$VERSION/usr/share/doc/packages/firejail
install -m 644 /usr/share/doc/firejail/COPYING firejail-$VERSION/usr/share/doc/packages/firejail/.
install -m 644 /usr/share/doc/firejail/README firejail-$VERSION/usr/share/doc/packages/firejail/.
install -m 644 /usr/share/doc/firejail/RELNOTES firejail-$VERSUIB/usr/share/doc/packages/firejail/.

mkdir -p firejail-$VERSION/etc/firejail
install -m 644 /etc/firejail/firefox.profile firejail-$VERSION/etc/firejail/firefox.profile
install -m 644 /etc/firejail/evince.profile firejail-$VERSION/etc/firejail/evince.profile
install -m 644 /etc/firejail/midori.profile firejail-$VERSION/etc/firejail/midori.profile
install -m 644 /etc/chromium-browser.profile firejail-$VERSION/etc/firejail/.
install -m 644 /etc/chromium.profile firejail-$VERSION/etc/firejail/.
install -m 644 /etc/firejail/login.users firejail-$VERSION/etc/firejail/login.users

mkdir -p firejail-$VERSION/usr/share/bash-completion/completions
install -m 644 /usr/share/bash-completion/completions/firejail  firejail-$VERSION/usr/share/bash-completion/completions/.

tar -czvf firejail-$VERSION.tar.gz firejail-$VERSION

cp firejail-$VERSION.tar.gz SOURCES/.

cat <<EOF > SPECS/firejail.spec
%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: Linux namepaces sandbox program
Name: firejail
Version: $VERSION
Release: 1
License: GPL+
Group: Development/Tools
SOURCE0 : %{name}-%{version}.tar.gz
URL: http://firejail.sourceforege.net

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
Firejail  is  a  SUID sandbox program that reduces the risk of security
breaches by restricting the running environment of untrusted applications
using Linux namespaces. It includes a sandbox profile for Mozilla Firefox.

%prep
%setup -q

%build

%install
rm -rf %{buildroot}
mkdir -p  %{buildroot}

cp -a * %{buildroot}


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/%{name}/firefox.profile
%config(noreplace) %{_sysconfdir}/%{name}/midori.profile
%config(noreplace) %{_sysconfdir}/%{name}/evince.profile
%config(noreplace) %{_sysconfdir}/%{name}/chromium.profile
%config(noreplace) %{_sysconfdir}/%{name}/chromium-browser.profile
%config(noreplace) %{_sysconfdir}/%{name}/login.users
%{_bindir}/*
%{_docdir}/*
%{_mandir}/*
/usr/share/bash-completion/completions/firejail
 
%post
chmod u+s /usr/bin/firejail

%changelog
* Sat Jun 7 2014  netblue30 <netblue30@yahoo.com> 0.9.6-1
 - Mounting tmpfs on top of /var/log, required by several server programs
 - Server fixes for /var/lib and /var/cache
 - Private mode fixes
 - csh and zsh default shell support
 - Chroot mode fixes
 - Added support for lighttpd, isc-dhcp-server, apache2, nginx, snmpd,

* Sun May 5 2014  netblue30 <netblue30@yahoo.com> 0.9.4-1
 - Fixed resolv.conf on Ubuntu systems using DHCP
 - Fixed resolv.conf on Debian systems using resolvconf package
 - Fixed /var/lock directory
 - Fixed /var/tmp directory
 - Fixed symbolic links in profile files
 - Added profiles for evince, midori

* Fri Apr 25 2014  netblue30 <netblue30@yahoo.com> 0.9.2-1
- Checking IP address passed with --ip option using ARP; exit if the address
   is already present
- Using a lock file during ARP address assignment in order to removed a race
   condition.
- Several fixes to --private option; it also mounts a tmpfs filesystem on top
   of /tmp
- Added user access check for profile file
- Added --defaultgw option
- Added support of --noip option; it is necessary for DHCP setups
- Added syslog support
- Added support for "tmpfs" and "read-only" profile commands
- Added an expect-based testing framework for the project
- Added bash completion support
- Added support for multiple networks

* Sat Apr 12 2014  netblue30 <netblue30@yahoo.com> 0.9-1
- First Build

EOF

rpmbuild -ba SPECS/firejail.spec
rpm -qpl RPMS/x86_64/firejail-$VERSION-1.x86_64.rpm
cd ..
rm -f firejail-$VERSION-1.x86_64.rpm
cp rpmbuild/RPMS/x86_64/firejail-$VERSION-1.x86_64.rpm .

