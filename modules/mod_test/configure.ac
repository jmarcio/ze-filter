
AC_INIT(ze-filter, 1.12.0, Jose-Marcio.Martins@ensmp.fr)

AC_COPYRIGHT([Copyright (c) 2002-2007 Ecole des Mines de Paris])
dnl AC_REVISION([$Id: configure.ac,v 1.129 2004/10/14 16:50:22 Exp $])
AC_CONFIG_SRCDIR([./mod_test.c])
dnl AC_CONFIG_AUX_DIR(build)

PACKAGE="ze-filter"
VERSION="1.11.1-$DATE"

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AC_CANONICAL_HOST

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_PROG_MAKE_SET

dnl ############################################################
dnl
dnl
dnl
AC_CHECK_PROG(ACLOCAL,aclocal,aclocal,echo)
AC_CHECK_PROG(AUTOMAKE,automake,automake,echo)
AC_CHECK_PROG(AUTOCONF,autoconf,autoconf,echo)
AC_CHECK_PROG(AUTOHEADER,autoheader,autoheader,echo)

AC_PATH_PROG(PERL,perl)

dnl ############################################################
dnl
dnl
dnl
AC_PREFIX_DEFAULT(/usr)
if test -n "$CONF_PREFIX" ;
then
  ac_default_prefix=$CONF_FREFIX
fi

sysconfdir="/etc/mail/jchkmail"
if test -d /var/run
then
  localstatedir="/var/run"
fi

if test "$GCC" = yes; then
  CFLAGS="$CFLAGS -Wall"
else
  CFLAGS="$CFLAGS"
fi

SPOOLDIR=/var/spool/jchkmail
CONFDIR=/etc/mail/jchkmail
WORKROOT=/var/jchkmail
ZEGREYDDIR=$WORKROOT/ze-greydb

dnl ############################################################
dnl
dnl
dnl
dnl Checks for programs.
dnl
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_ISC_POSIX

dnl AC_DISABLE_SHARED
AC_PROG_LIBTOOL

MODULES="modules/mod_test"
dnl AC_CONFIG_SUBDIRS($(MODULES))

dnl ############################################################
dnl
dnl
dnl
dnl
dnl Define variables if we're looking for things
dnl at sendmail source tree
dnl
UNAME="`uname -a`"
AC_SUBST(UNAME)

dnl ############################################################
dnl

dnl ############################################################
dnl
dnl
dnl
dnl
dnl WORKROOT=/var/jchkmail
AC_ARG_WITH(work-dir,
  AC_HELP_STRING([--with-work-dir=PATH],
                 [set ze-filter work dir   (default /var/jchkmail)]),
  [WORKROOT="$withval"])
AC_SUBST(WORKROOT)

dnl
dnl
dnl SPOOLDIR=/var/spool/jchkmail
AC_ARG_WITH(spool-dir,
  AC_HELP_STRING([--with-spool-dir=PATH],
                 [set ze-filter spool dir (default /var/spool/jchkmail)]),
  [SPOOLDIR="$withval"])
AC_SUBST(SPOOLDIR)

dnl
dnl
AC_MSG_CHECKING([where process state files shall be...])
STATEDIR=/var/run
if test -d $STATEDIR ; then
  STATEDIR=/var/run/jchkmail
else
  STATEDIR=$WORKROOT
fi
AC_MSG_RESULT($STATEDIR)
AC_SUBST(STATEDIR)
dnl For the moment, let STATEDIR be equal to WORKROOT... 
dnl STATEDIR=$WORKROOT

dnl
dnl
dnl CONFDIR=$sysconfdir
AC_ARG_WITH(conf-dir,
  AC_HELP_STRING([--with-conf-dir=PATH],
                 [set ze-filter config dir (default /etc/mail/jchkmail)]),
  [CONFDIR="$withval"])
AC_SUBST(CONFDIR)
sysconfdir=$CONFDIR

dnl
dnl
dnl ############################################################
dnl

ACX_PTHREAD()

CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"


dnl ############################################################
dnl
dnl
dnl
dnl Replace `main' with a function in -lpthread:
AC_CHECK_LIB(pthread, main,, LIBS="$LIBS -pthread")

AC_CHECK_LIB(nsl, main)
AC_CHECK_LIB(socket, main)

dnl
AC_CHECK_LIB(resolv, res_nquery)
AC_CHECK_LIB(resolv, __res_nquery)
AC_CHECK_LIB(c, res_nquery)
AC_CHECK_LIB(c, __res_nquery)
if test "$ac_cv_lib_resolv___res_nquery" = "yes" ;
then
  ac_cv_lib_resolv_res_nquery=yes
fi

USE_LIBBIND="no"

AC_CHECK_LIB(resolv, main)


case $host_os 
in
  freebsd4*)
    ;;
  freebsd5.0*|freebsd5.1*|freebsd5.2*)
    ;;
  freebsd5.3*|freebsd5.4*|freebsd5.5*|freebsd5.6*|freebsd5.7*|freebsd6*)
    AC_DEFINE(BSD_RES_THREAD_SAFE,1,[Thread safe resolver])
    ;;
esac


AC_CHECK_LIB(m, main)
AC_CHECK_LIB(kstat, main)
AC_CHECK_LIB(rt, main)

AC_CHECK_LIB(dl, main)

case $host_os 
in
  *hpux*)
    AC_CHECK_LIB(dld, main)
    ;;
  *freebsd*)
    AC_CHECK_LIB(kvm, main)
    ;;
esac

dnl
dnl Add code here to enable ze-filter building without libmilter
dnl Surely, when doing this, ze-filter build will be done without
dnl online filter
dnl
SAVELIBS=$LIBS
AC_CHECK_LIB(milter, main,,\
	AC_MSG_ERROR(Can't compile ze-filter without libmilter...))


AC_CHECK_LIB(milter,mi_stop,AC_DEFINE([HAVE_MI_STOP], 1, [MI_STOP]))


dnl ############################################################
dnl
dnl
dnl
dnl
TOPDIR=`pwd`
AC_SUBST(TOPDIR)

dnl
dnl
dnl Finally, what we're really want
AC_CONFIG_FILES([Makefile])

AC_CONFIG_COMMANDS([default],[],[[]])
AC_OUTPUT
echo ""

dnl ############################################################
dnl
dnl
dnl

