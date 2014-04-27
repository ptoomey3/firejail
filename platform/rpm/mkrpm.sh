#!/bin/bash
rm -fr rpmbuild

mkdir -p ~/rpmbuild/{RPMS,SRPMS,BUILD,SOURCES,SPECS,tmp}
cat <<EOF >~/.rpmmacros
%_topdir   %(echo $HOME)/rpmbuild
%_tmppath  %{_topdir}/tmp
EOF

cd ~/rpmbuild

mkdir -p firejail-0.9.2/usr/bin
install -m 755 /usr/bin/firejail firejail-0.9.2/usr/bin/.
install -m 755 /usr/bin/firemon firejail-0.9.2/usr/bin/.

mkdir -p firejail-0.9.2/usr/share/man/man1
install -m 644 /usr/share/man/man1/firejail.1.gz firejail-0.9.2/usr/share/man/man1/.
install -m 644 /usr/share/man/man1/firemon.1.gz firejail-0.9.2/usr/share/man/man1/.

mkdir -p firejail-0.9.2/usr/share/doc/packages/firejail
install -m 644 /usr/share/doc/firejail/COPYING firejail-0.9.2/usr/share/doc/packages/firejail/.
install -m 644 /usr/share/doc/firejail/README firejail-0.9.2/usr/share/doc/packages/firejail/.
install -m 644 /usr/share/doc/firejail/RELNOTES firejail-0.9.2/usr/share/doc/packages/firejail/.

mkdir -p firejail-0.9.2/etc/firejail
install -m 644 /etc/firejail/firefox.profile firejail-0.9.2/etc/firejail/firefox.profile
install -m 644 /etc/firejail/login.users firejail-0.9.2/etc/firejail/login.users

mkdir -p firejail-0.9.2//usr/share/bash-completion/completions
install -m 644 /usr/share/bash-completion/completions/firejail  firejail-0.9.2//usr/share/bash-completion/completions/.

tar -czvf firejail-0.9.2.tar.gz firejail-0.9.2

cp firejail-0.9.2.tar.gz SOURCES/.

cat <<EOF > SPECS/firejail.spec
%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: Linux namepaces sandbox program
Name: firejail
Version: 0.9.2
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
%config(noreplace) %{_sysconfdir}/%{name}/login.users
%{_bindir}/*
%{_docdir}/*
%{_mandir}/*
/usr/share/bash-completion/completions/firejail
 
%post
chmod u+s /usr/bin/firejail

%changelog
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
rpm -qpl RPMS/x86_64/firejail-0.9.2-1.x86_64.rpm
cd ..
rm -f firejail-0.9.2-1.x86_64.rpm
cp rpmbuild/RPMS/x86_64/firejail-0.9.2-1.x86_64.rpm .
