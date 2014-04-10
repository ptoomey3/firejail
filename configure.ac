AC_PREREQ([2.68])
AC_INIT(firejail, 0.9, netblue30@yahoo.com, , http://firejail.sourceforge.net)
AC_CONFIG_SRCDIR([src/firejail/main.c])
#AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

# checking pthread library
AC_CHECK_LIB([pthread], [main], [], AC_MSG_ERROR([*** POSIX thread support not installed ***]))
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR([*** POSIX thread support not installed ***]))

# testing ip
IP=`which ip`
if test "$IP" == ""
then
        AC_MSG_ERROR([ip command is missing, please install iproute2 package.])
fi
echo "Found $IP"

# testing brctl
BRCTL=`which brctl`
if test "$BRCTL" == ""
then
        AC_CHECK_FILE(/sbin/brctl,[BRCTL="/sbin/brctl"],
        	[AC_MSG_ERROR([brctl commnad is missing, please install bridge-utils package.])])
fi
echo "Found $BRCTL"

AC_OUTPUT(Makefile)
AC_OUTPUT(src/lib/Makefile)
AC_OUTPUT(src/firejail/Makefile)
AC_OUTPUT(src/firemon/Makefile)