#!/bin/bash
rm -fr rpmbuild

mkdir -p ~/rpmbuild/{RPMS,SRPMS,BUILD,SOURCES,SPECS,tmp}
cat <<EOF >~/.rpmmacros
%_topdir   %(echo $HOME)/rpmbuild
%_tmppath  %{_topdir}/tmp
EOF

cd ~/rpmbuild

mkdir -p firejail-0.9/usr/bin
install -m 755 /usr/bin/firejail firejail-0.9/usr/bin/.
install -m 755 /usr/bin/firemon firejail-0.9/usr/bin/.

mkdir -p firejail-0.9/usr/share/man/man1
install -m 644 /usr/share/man/man1/firejail.1.gz firejail-0.9/usr/share/man/man1/.
install -m 644 /usr/share/man/man1/firemon.1.gz firejail-0.9/usr/share/man/man1/.

mkdir -p firejail-0.9/usr/share/doc/packages/firejail
install -m 644 /usr/share/doc/firejail/COPYING firejail-0.9/usr/share/doc/packages/firejail/.
install -m 644 /usr/share/doc/firejail/README firejail-0.9/usr/share/doc/packages/firejail/.
install -m 644 /usr/share/doc/firejail/RELNOTES firejail-0.9/usr/share/doc/packages/firejail/.

mkdir -p firejail-0.9/etc/firejail
install -m 644 /etc/firejail/firefox.profile firejail-0.9/etc/firejail/firefox.profile
install -m 644 /etc/firejail/login.users firejail-0.9/etc/firejail/login.users

tar -czvf firejail-0.9.tar.gz firejail-0.9

cp firejail-0.9.tar.gz SOURCES/.

cat <<EOF > SPECS/firejail.spec
%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: Linux namepaces sandbox program
Name: firejail
Version: 0.9
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

%post
chmod u+s /usr/bin/firejail

%changelog
* Wed Apr 02 2014  netblue30 <netblue30@yahoo.com> 0.9-1
- First Build

EOF

rpmbuild -ba SPECS/firejail.spec
rpm -qpl RPMS/x86_64/firejail-0.9-1.x86_64.rpm
cd ..
rm -f firejail-0.9-1.x86_64.rpm
cp rpmbuild/RPMS/x86_64/firejail-0.9-1.x86_64.rpm .
