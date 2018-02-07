
AC_INIT(ze-filter, 0.0.1, jose.marcio.mc@gmail.com)
AM_INIT_AUTOMAKE([no-define])
AC_COPYRIGHT([Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz])

PACKAGE="ze-filter"

DATE=$(date +%y%m%d)
[ test -f aux/timestamp ] &&  DATE=$(head -1 aux/timestamp)
export DATE

VERSION="VOID-$DATE"
[ test -f aux/release ] && VERSION=$(head -1 aux/release)
export VERSION

COPYRIGHT="Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz"

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(COPYRIGHT)

AC_CONFIG_SRCDIR([src/ze-filter.c])
AM_CONFIG_HEADER([include/config.h])

AC_CONFIG_MACRO_DIRS([m4])
AC_CANONICAL_HOST

AC_PROG_MAKE_SET

dnl ############################################################
dnl
dnl
dnl
AC_CHECK_PROG(ACLOCAL,   aclocal,aclocal,echo)
AC_CHECK_PROG(AUTOMAKE,  automake,automake,echo)
AC_CHECK_PROG(AUTOCONF,  autoconf,autoconf,echo)
AC_CHECK_PROG(AUTOHEADER,autoheader,autoheader,echo)

AC_PATH_PROG(PERL,perl)

dnl ############################################################
dnl
dnl
dnl
CONF_FILE=compile.conf

function getCompileConf()
{
  test -z "$CONF_FILE" && echo "" && return
  grep ^$1 $CONF_FILE 2>/dev/null | \
    sed -e "s/[ ]*=[ ]*/=/" | \
    awk -F= '{print $2}' | \
    tail -1
}

CONF_USER=$(getCompileConf USER)
CONF_GROUP=$(getCompileConf GROUP)

CONF_PREFIX=$(getCompileConf PREFIX)
CONF_CONFDIR=$(getCompileConf CONFDIR)
CONF_WORKROOT=$(getCompileConf WORKROOT)
CONF_SPOOLDIR=$(getCompileConf SPOOLDIR)
CONF_ZEGREYDDIR=$(getCompileConf ZEGREYDDIR)

CONF_RCDIR=$(getCompileConf RCDIR)

CONF_CC=$(getCompileConf CC)
CONF_CFLAGS=$(getCompileConf CFLAGS)
CONF_LDFLAGS=$(getCompileConf LDFLAGS)
CONF_CPPFLAGS=$(getCompileConf CPPFLAGS)

CONF_RSYNC_SERVER=$(getCompileConf RSYNC_SERVER)

dnl ############################################################
dnl
dnl
dnl
RSYNC_SERVER="foss.jose-marcio.org:1873"
if test [ -n "$CONF_RSYNC_SERVER" ];
then
  RSYNC_SERVER="$CONF_RSYNC_SERVER"
fi

AC_SUBST(RSYNC_SERVER)

dnl ############################################################
dnl
dnl
dnl
AC_PREFIX_DEFAULT(/usr)
if test -n "$CONF_PREFIX" ;
then
  ac_default_prefix=$CONF_FREFIX
fi

dnl ### fix unset "--prefix=..." ###
if test "$prefix" = "NONE"
then
  if test -n "$CONF_PREFIX"
  then
    prefix=$CONF_PREFIX
  else
    prefix=$ac_default_prefix
  fi
fi

if test -n "$CONF_CFLAGS"
then
  CFLAGS="$CFLAGS $CONF_CFLAGS"
  export CFLAGS
fi
if test -n "$CONF_CPPFLAGS"
then
  CPPFLAGS="$CPPFLAGS $CONF_CPPFLAGS"
  export CPPFLAGS
fi
if test -n "$CONF_LDFLAGS"
then
  LDFLAGS="$LDFLAGS $CONF_LDFLAGS"
fi

sysconfdir="/etc/ze-filter"
if test -n "$CONF_CONFDIR" ;
then
  sysconfdir=$CONF_CONFDIR
fi

if test -d /var/run
then
  localstatedir="/var/run"
fi

if test "$GCC" = yes; then
  CFLAGS="$CFLAGS -Wall"
  export CFLAGS
else
  CFLAGS="$CFLAGS"
fi

USER=ze-filter
if test -n "$CONF_USER" ;
then
  USER=$CONF_USER
fi
GROUP=ze-filter
if test -n "$CONF_GROUP" ;
then
  GROUP=$CONF_GROUP
fi
SPOOLDIR=/var/spool/ze-filter
if test -n "$CONF_SPOOLDIR" ;
then
  SPOOLDIR=$CONF_SPOOLDIR
fi
CONFDIR=/etc/ze-filter
if test -n "$CONF_CONFDIR" ;
then
  CONFDIR=$CONF_CONFDIR
fi
WORKROOT=/var/ze-filter
if test -n "$CONF_WORKROOT" ;
then
  WORKROOT=$CONF_WORKROOT
fi
ZEGREYDDIR=$WORKROOT/ze-greydb
if test -n "$CONF_ZEGREYDDIR" ;
then
  ZEGREYDDIR=$CONF_ZEGREYDDIR
fi


dnl ############################################################
dnl
dnl
dnl
dnl Checks for programs.
dnl
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LN_S
LT_INIT
AC_PROG_INSTALL

AC_ISC_POSIX

dnl AC_DISABLE_SHARED
AC_PROG_LIBTOOL

dnl ############################################################
dnl
dnl
dnl
dnl
dnl Define variables if we're looking for things
dnl at sendmail source tree
dnl
arch=`uname -m`
case $arch
in
  sun4*)        arch=sun4;;
esac
SMOBJ="obj.`uname -s`.`uname -r`.$arch"

dnl
dnl Check if we're running Solaris. Shall set some
dnl variables
AC_MSG_CHECKING(whether we're running Sun Solaris)
os=`uname -s`
AC_MSG_RESULT($os)
AC_MSG_RESULT(     Building for $build...)

UNAME=$(uname -a)
AC_SUBST(UNAME)

AC_PATH_PROG(PSBIN,ps)
AC_SUBST(PSBIN)

PSOPT=""
AC_MSG_CHECKING(whether ps -edf works)
ps -edf > /dev/null 2>&1
if [ test "$?" = "0" ]
then
  res="yes"
  PSOPT="-edf"
else
  res="no"
fi
AC_MSG_RESULT($res)

if test "$res" = "no"
then
  AC_MSG_CHECKING(whether ps -axu works)
  ps -axu > /dev/null 2>&1
  if test "$?" = "0" ; then
    res="yes"
    PSOPT="-axu"
  else
    res="no"
  fi
  AC_MSG_RESULT($res)
fi
AC_SUBST(PSOPT)


if test "$TOTO" = "0" ;
then
dnl ############################################################
dnl
echo "** build          $build"
echo "** host           $host"

echo "** target         $target"
echo "** build_alias    $build_alias"
echo "** host_alias     $host_alias"
echo "** target_alias   $target_alias"

echo "** build_cpu      $build_cpu"
echo "** build_vendor   $build_vendor"
echo "** build_os       $build_os"
echo "** host_cpu       $host_cpu"
echo "** host_vendor    $host_vendor"
echo "** host_os        $host_os"

echo "** target_cpu     $target_cpu"
echo "** target_vendor  $target_vendor"
echo "** target_os      $target_os"
fi

case $host_os 
in
  *linux*)
    AC_DEFINE(OS_LINUX,1,[HOST OS])
    kernel_version=`uname -r`
    case $kernel_version
    in
      2.4.*)
        AC_DEFINE(LINUX_24_KERNEL,1,[OLD LINUX KERNEL])
        ;;
    esac
    ;;
  *solaris*)
    AC_DEFINE(OS_SOLARIS,1,[HOST OS])
    ;;
  *darwin*)
    AC_DEFINE(OS_MACOS,1,[HOST OS])
    ;;
  *freebsd*)
    AC_DEFINE(OS_FREEBSD,1,[HOST OS])
    ;;
  *openbsd*)
    AC_DEFINE(OS_OPENBSD,1,[HOST OS])
    ;;
  *netbsd*)
    AC_DEFINE(OS_NETBSD,1,[HOST OS])
    ;;
  *osf*)
    AC_DEFINE(OS_TRU64,1,[HOST_OS])
    ;;
  *hpux*)
    AC_DEFINE(OS_HPUX,1,[HOST_OS])
    ;;
esac

dnl ############################################################
dnl
dnl
dnl
dnl
dnl Let's read configure arguments


dnl
dnl
dnl USER=ze-filter
AC_ARG_WITH(user,
  AC_HELP_STRING([--with-user=USER],
	               [set ze-filter user name  (default ze-filter)]),
  [USER="$withval"])
AC_SUBST(USER)

dnl
dnl
dnl GROUP=ze-filter
AC_ARG_WITH(group,
  AC_HELP_STRING([--with-group=GROUP],
	               [set ze-filter group name  (default ze-filter)]),
  [GROUP="$withval"])
AC_SUBST(GROUP)

dnl
dnl
dnl WORKROOT=/var/ze-filter
AC_ARG_WITH(work-dir,
  AC_HELP_STRING([--with-work-dir=PATH],
                 [set ze-filter work dir   (default /var/ze-filter)]),
  [WORKROOT="$withval"])
AC_SUBST(WORKROOT)

dnl
dnl
dnl ZEGREYDDIR=/var/ze-filter/ze-greydb
AC_ARG_WITH(ze-greyd-dir,
  AC_HELP_STRING([--with-ze-greyd-dir=PATH],
                 [set ze-greyd work dir   (default /var/ze-filter/ze-greydb)]),
  [ZEGREYDDIR="$withval"])
AC_SUBST(ZEGREYDDIR)

dnl
dnl
dnl SPOOLDIR=/var/spool/ze-filter
AC_ARG_WITH(spool-dir,
  AC_HELP_STRING([--with-spool-dir=PATH],
                 [set ze-filter spool dir (default /var/spool/ze-filter)]),
  [SPOOLDIR="$withval"])
AC_SUBST(SPOOLDIR)

dnl
dnl
AC_MSG_CHECKING([where process state files shall be...])
STATEDIR=/var/run
if test -d $STATEDIR ; then
  STATEDIR=/var/run/ze-filter
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
                 [set ze-filter config dir (default /etc/ze-filter)]),
  [CONFDIR="$withval"])
AC_SUBST(CONFDIR)
sysconfdir=$CONFDIR

dnl
dnl
dnl
AC_MSG_CHECKING(if system has systemd)
HAS_SYSTEMD=no
[ test -d /etc/systemd/system ] && HAS_SYSTEMD=yes
AC_MSG_RESULT($HAS_SYSTEMD)

AC_MSG_CHECKING([where default start-up scripts are installed])

if test -n "$CONF_RCDIR" ;
then
  RCDIR=$CONF_RCDIR
else
  RCDIR=/etc/init.d
  for dir in /etc/init.d /etc/rc.d /etc/rc.d/init.d /sbin/init.d /usr/local/etc/rc.d
  do
    if test -d $dir
    then
      RCDIR=$dir
      break
    fi
  done
  case $build
  in
    *freebsd*)
      RCDIR="/usr/local/etc/rc.d"
      ;;
    *)
      ;;
  esac
fi
AC_MSG_RESULT($RCDIR)
 
dnl
dnl
AC_ARG_WITH(rc-dir,
  AC_HELP_STRING([--with-rc-dir=PATH],
                 [set system start-up dir (default /etc/init.d or /etc/rc.d)]),
  [RCDIR="$withval"])
AC_SUBST(RCDIR)

case $build
in
  *freebsd*)
    RCSCRIPT="ze-filter.sh"
    RCGREYD="ze-greyd.sh"
    ;;
  *)
    RCSCRIPT="ze-filter"
    RCGREYD="ze-greyd"
    ;;
esac
AC_SUBST(RCSCRIPT)
AC_SUBST(RCGREYD)

AC_MSG_CHECKING([file extensions defined at xfiles.def file])
XFILES_DEF=""
cp /dev/null > include/xfiles.inc > /dev/null 2>&1
cp /dev/null > include/xfilesdef.h > /dev/null 2>&1
if [ test -f tools/xfiles.def -a -x tools/mk-xfilesdef ] ;
then
  XFILES_DEF=`tools/mk-xfilesdef tools/xfiles.def`
  $PERL tools/mk-xfilesdef -l tools/xfiles.def > include/xfiles.inc
  $PERL tools/mk-xfilesdef -i tools/xfiles.def > include/xfilesdef.h
fi
AC_MSG_RESULT(ok)
AC_SUBST(XFILES_DEF)


dnl
dnl
LANGSPEC=EN
AC_SUBST(LANGSPEC)

dnl ############################################################
dnl

ACX_PTHREAD()

CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"


dnl ############################################################
dnl
dnl
dnl
dnl
dnl Let's check for libraries
dnl
case $host_os 
in
  *linux*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,1,[for normal OSs])
    AC_DEFINE(_DEFAULT_SOURCE,1,[DEFAULT_SOURCE])
    AC_DEFINE(_BSD_SOURCE,1,[BSD_SOURCE])
    AC_DEFINE(_POSIX_SOURCE,,[POSIX_SOURCE])
    AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    dnl AC_DEFINE(_XOPEN_SOURCE,500,[BSD_SOURCE])
    dnl AC_DEFINE(_XOPEN_SOURCE_EXTENDED,1,[BSD_SOURCE])

    #CPPFLAGS="-D_REENTRANT $CPPFLAGS"
    ;;
  *solaris*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,1,[for normal OSs])
    dnl AC_DEFINE(_BSD_SOURCE,1,[BSD_SOURCE])
    AC_DEFINE(_POSIX_SOURCE,,[POSIX_SOURCE])
    AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    AC_DEFINE(__EXTENSIONS__,1,[__EXTENSIONS])
    dnl AC_DEFINE(_XOPEN_SOURCE,600,[_XOPEN_SOURCE])
    dnl AC_DEFINE(_XOPEN_SOURCE_EXTENDED,1,[_XOPEN_SOURCE_EXTENDED])

    #CPPFLAGS="-D_XPG6 $CPPFLAGS"
    #if test "$ac_cv_c_compiler_gnu" = "yes"
    #then
    #  CFLAGS="$CFLAGS"
    #else
    #  CPPFLAGS="-D_XPG6 $CPPFLAGS"
    #  CFLAGS="-xc99 $CFLAGS"
    #fi
    ;;
  *freebsd*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,1,[for normal OSs])
    AC_DEFINE(_BSD_SOURCE,1,[BSD_SOURCE])
    dnl AC_DEFINE(_POSIX_SOURCE,,[POSIX_SOURCE])
    dnl AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    AC_DEFINE(__EXTENSIONS__,,[__EXTENSIONS])
    #AC_DEFINE(_XOPEN_SOURCE,600,[BSD_SOURCE])
    ;;
  *darwin*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,1,[for normal OSs])
    AC_DEFINE(BIND_8_COMPAT,1,[needed by ze-dns])
    dnl dnl AC_DEFINE(_BSD_SOURCE,1,[BSD_SOURCE])
    dnl AC_DEFINE(_POSIX_SOURCE,,[POSIX_SOURCE])
    dnl AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    dnl AC_DEFINE(__EXTENSIONS__,1,[__EXTENSIONS])
    dnl dnl AC_DEFINE(_XOPEN_SOURCE,600,[_XOPEN_SOURCE])
    dnl dnl AC_DEFINE(_XOPEN_SOURCE_EXTENDED,1,[_XOPEN_SOURCE_EXTENDED])
    ;;
  *openbsd*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,1,[for normal OSs])
    AC_DEFINE(_BSD_SOURCE,1,[BSD_SOURCE])
    dnl AC_DEFINE(_POSIX_SOURCE,,[POSIX_SOURCE])
    dnl AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    AC_DEFINE(__EXTENSIONS__,,[__EXTENSIONS])
    #AC_DEFINE(_XOPEN_SOURCE,600,[BSD_SOURCE])
    ;;
  *netbsd*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,,[for normal OSs])
    ;;
  *osf*)
    AC_DEFINE(_POSIX_PTHREAD_SEMANTICS,1,[for normal OSs])
    AC_DEFINE(_OSF_SOURCE,1,[OSF_SOURCE])
    dnl AC_DEFINE(_POSIX_SOURCE,,[POSIX_SOURCE])
    dnl AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    AC_DEFINE(_XOPEN_SOURCE,600,[BSD_SOURCE])
    AC_DEFINE(_POSIX_SHARED_MEMORY_OBJECTS,1,[POSIX...])
    AC_DEFINE(__EXTENSIONS__,1,[__EXTENSIONS])
    ;;
  *hpux*)
    AC_DEFINE(_PTHREADS_DRAFT4,1,[for HU-UX])
    dnl AC_DEFINE(_BSD_SOURCE,1,[BSD_SOURCE])
    AC_DEFINE(_POSIX_SOURCE,1,[POSIX_SOURCE])
    AC_DEFINE(_POSIX_C_SOURCE,199506L,[POSIX_SOURCE])
    AC_DEFINE(__EXTENSIONS__,1,[__EXTENSIONS])
    AC_DEFINE(_XOPEN_SOURCE,600,[BSD_SOURCE])
    AC_DEFINE(_XOPEN_SOURCE_EXTENDED,1,[BSD_SOURCE])
    ;;
esac

dnl ############################################################
dnl
dnl
dnl
dnl
dnl Let's check for libraries
dnl


dnl
dnl Now let's really check them
dnl Replace `main' with a function in -lmilter:
dnl Debian Linux install libmilter at /usr/lib/libmilter while
dnl everybody do it at /usr/lib.
if test ! -f /usr/lib/libmilter$SMLMV.a ;
then
  if test -f /usr/lib/libmilter/libmilter.a ;
  then
    LDFLAGS="-L/usr/lib/libmilter $LDFLAGS"
  fi
fi

dnl Replace `main' with a function in -lpthread:
AC_CHECK_LIB(pthread, main,, LIBS="$LIBS -pthread")

case $host_os 
in
  *linux*)
    ;;
  *solaris*)
    ;;
  *freebsd*)
    ;;
  *netbsd*)
    #AC_CHECK_LIB(compat, lfind)
    ;;
  *osf*)
    ;;
esac


AC_CHECK_LIB(nsl, main)
AC_CHECK_LIB(socket, main)
AC_CHECK_LIB(ldap, main)

#dnl
#dnl Decide if shall use bundled libbind
#dnl
dnl
dnl libresolv shall be checked only if not using libbind
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
  freebsd5.3*|freebsd5.4*|freebsd5.5*|freebsd5.6*|freebsd5.7*)
    AC_DEFINE(BSD_RES_THREAD_SAFE,1,[Thread safe resolver])
    ;;
  freebsd6*|freebsd7*|freebsd8*)
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

#
# was --with-libxml2 specified?
#
AC_MSG_CHECKING(for libxml2 library)
AC_ARG_WITH(libxml2,
[  --with-libxml2[=PATH]   Build with libxml2 library [yes|no|path]],
    use_libxml2="$withval", use_libxml2="auto")

case "$use_libxml2" in
        no)
                DST_LIBXML2_INC=""
                ;;
        auto|yes)
                case X`(xml2-config --version) 2>/dev/null` in
                X2.[[67]].*)
                        libxml2_libs=`xml2-config --libs`
                        libxml2_cflags=`xml2-config --cflags`
                        ;;
                *)
                        libxml2_libs=
                        libxml2_cflags=
                        ;;
                esac
                ;;
        *)
                if test -f "$use_libxml2/bin/xml2-config" ; then
                        libxml2_libs=`$use_libxml2/bin/xml2-config --libs`
                        libxml2_cflags=`$use_libxml2/bin/xml2-config --cflags`
                fi
                ;;
esac

if test "X$libxml2_libs" != "X"
then
        AC_MSG_RESULT(yes)
        CFLAGS="$CFLAGS $libxml2_cflags"
        LIBS="$LIBS $libxml2_libs"
        AC_DEFINE(HAVE_LIBXML2, 1, [Define if libxml2 was found])
else
        AC_MSG_RESULT(no)
fi



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
dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT 
AC_HEADER_STDBOOL
AC_HEADER_STAT

AC_CHECK_HEADERS([assert.h])

AC_CHECK_HEADERS([\
  sys/cdefs.h \
  sys/types.h \
  stdint.h \
  sys/stat.h \
  syslog.h \
  signal.h \
  stddef.h \
  unistd.h \
  sysexits.h \
  regex.h \
  ctype.h \
  sys/time.h])


AC_CHECK_HEADERS([\
  poll.h \
  sys/poll.h \
  sys/select.h \
  fcntl.h \
  dirent.h])

AC_CHECK_HEADERS([ \
  sys/socket.h \
  sys/un.h \
  netinet/in.h \
  arpa/inet.h \
  netdb.h \
  arpa/nameser.h \
  resolv.h],,,
  [
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#if HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
  ])


AC_CHECK_HEADERS([\
  stdarg.h \
  strings.h \
  string.h \
  math.h])

AC_CHECK_HEADERS([\
  limits.h \
  values.h])

AC_CHECK_HEADERS([\
  thread.h \
  pthread.h \
  sys/pthread.h \
  sys/resource.h],,,
  [
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
  ])

AC_CHECK_HEADERS([\
  pwd.h \
  grp.h])

AC_CHECK_HEADERS([\
  kstat.h \
  sys/loadavg.h \
  sys/systeminfo.h \
  sys/sysinfo.h \
  sys/utsname.h],,,
  [
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
  ])

AC_CHECK_HEADERS([\
  sys/mman.h])

AC_CHECK_HEADERS([dlfcn.h link.h])

AC_CHECK_HEADERS([lber.h ldap.h])

case $host_os 
in
  *linux*)
    ;;
  *solaris*)
    ;;
  *freebsd*)
    AC_CHECK_HEADERS([kvm.h])
    AC_CHECK_HEADERS([sys/dkstat.h],,,
    [
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
    ]
    )
    ;;
  *netbsd*)
    AC_CHECK_LIB(compat, lfind)
    ;;
  *osf*)
    ;;
esac

AC_CHECK_HEADERS([libgen.h])

AC_CHECK_HEADERS([\
  libmilter/mfapi.h\
  libmilter/mfdef.h])

if test "$ac_cv_header_libmilter_mfapi_h" = "no" ; then
  AC_CHECK_HEADERS([mfapi.h mfdef.h])

  if test "$ac_cv_header_mfapi_h" = "yes" ; then
  echo "***   "
  echo "*** It seems that the libmilter installed is older than 8.12.3 version"
  echo "***   "
  echo "*** If this is the case, you should do the following (as root)"
  echo "***    cd /usr/include"
  echo "***    mkdir libmilter"
  echo "***    mv mfapi.h mfdef.h libmilter"
  echo "***   "
  else
  echo "***   "
  echo "*** It seems that you don't have libmilter header files"
  echo "*** You need to reinstall libmilter library"
  echo "***   "
  fi
  exit 1
fi

AC_LANG_C


AC_MSG_CHECKING(smfiDesc has xxfi_unknown)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <libmilter/mfapi.h>
#ifndef NULL
#define NULL 0L
#endif
]],
[[struct smfiDesc x;
x.xxfi_unknown = NULL;
]])],
	[r=yes ],
	[r=no])
AC_MSG_RESULT($r)
if test [ "$r" = "yes" ];
then
  AC_DEFINE(HAVE_XXFI_UNKNOWN,1,[have xxfi_unknown])
fi

AC_MSG_CHECKING(smfiDesc has xxfi_data)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <libmilter/mfapi.h>
#ifndef NULL
#define NULL 0L
#endif
]],
[[struct smfiDesc x;
x.xxfi_data = NULL;
]])],
	[r=yes ],
	[r=no])
AC_MSG_RESULT($r)
if test [ "$r" = "yes" ];
then
  AC_DEFINE(HAVE_XXFI_DATA,1,[have xxfi_data])
fi

AC_MSG_CHECKING(smfiDesc has xxfi_negotiate)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <libmilter/mfapi.h>
#ifndef NULL
#define NULL 0L
#endif
]],
[[struct smfiDesc x;
x.xxfi_negotiate = NULL;
]])],
	[r=yes ],
	[r=no])
AC_MSG_RESULT($r)
if test [ "$r" = "yes" ];
then
  AC_DEFINE(HAVE_XXFI_NEGOTIATE,1,[have xxfi_negotiate])
fi

AC_MSG_CHECKING(smfiDesc has xxfi_signal)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <libmilter/mfapi.h>
#ifndef NULL
#define NULL 0L
#endif
]],
[[struct smfiDesc x;
x.xxfi_signal = NULL;
]])],
	[r=yes ],
	[r=no])
AC_MSG_RESULT($r)
if test [ "$r" = "yes" ];
then
  AC_DEFINE(HAVE_XXFI_SIGNAL,1,[have xxfi_signal])
fi


dnl ############################################################
dnl
dnl
dnl

dnl CPPFLAGS="$CPPFLAGS -I../include"

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

AC_C_CONST
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(unsigned long long)
AC_TYPE_SIGNAL

AC_CHECK_TYPES([bool, uchar])
AC_CHECK_TYPES([long long, unsigned long long])
AC_CHECK_TYPES([uid_t, gid_t, pid_t])
AC_CHECK_TYPES([int64_t, uint64_t, int32_t, uint32_t, u_int32_t],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif
])
AC_CHECK_TYPES([int16_t, uint16_t, int8_t, uint8_t],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif
])
AC_CHECK_TYPES([size_t, ssize_t, off_t, mode_t])
AC_CHECK_TYPES([hrtime_t])
AC_CHECK_TYPES([in_addr_t,in6_addr_t,struct in_addr,struct in6_addr],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_SYS_SOCKET_H 
# include <sys/socket.h>
#endif
])

AC_CHECK_TYPES([
struct sockaddr,struct addrinfo,struct sockaddr_un,struct sockaddr_in,struct sockaddr_in6],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
#if HAVE_SYS_SOCKET_H 
# include <sys/socket.h>
#endif
#if HAVE_SYS_UN_H 
# include <sys/un.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])



AC_CHECK_TYPES([
socket_t,socklen_t],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_SYS_SOCKET_H 
# include <sys/socket.h>
#endif
])

AC_CHECK_TYPES([pthread_mutex_t,pthread_rwlock_t],,,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_PTHREAD_H
# include <pthread.h>
#endif
])

AC_CHECK_TYPES([struct flock,flock_t],,,
[
#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
])


dnl Checks for library functions.
AC_CHECK_FUNCS([regcomp \
  alloca \
  strsep \
  strcspn \
  strdup \
  strndup \
  strspn \
  strstr \
  strlcpy \
  strlcat \
  strchr \
  strtol \
  strtoul \
  strtoll \
  strtoull \
  strtod \
  memcpy \
  memcmp \
  ffs \
  strerror \
  vsprintf \
  vsnprintf \
  vsyslog \
  mkstemp \
  mktemp])

AC_CHECK_FUNCS([gethrtime \
  clock \
  time \
  ctime \
  ctime_r \
  sleep \
  usleep])

AC_CHECK_FUNCS([getrusage \
  getrlimit \
  setrlimit \
  getloadavg \
  sysinfo \
  sysconf \
  uname \
  gethostname \
  sysctlbyname \
  kstat_open \
  kstat_close \
  kstat_read \
  kstat_lookup])

AC_CHECK_FUNCS([fdwalk])

AC_CHECK_FUNCS([mmap \
  munmap \
  shm_open \
  shm_unlink])

AC_CHECK_FUNCS([stat \
  lstat \
  read \
  fchmod \
  pread \
  pwrite \
  ftruncate \
  opendir \
  readdir \
  readdir_r])

AC_CHECK_FUNCS([socket \
  select \
  poll \
  socketpair \
  inet_ntop \
  inet_pton \
  inet_ntoa \
  inet_aton \
  inet_addr \
  getaddrinfo \
  getnameinfo \
  freeaddrinfo \
  gethostbyname \
  gethostbyname_r \
  gethostbyaddr \
  gethostbyaddr_r])

AC_CHECK_FUNCS([\
    res_init \
    res_close \
    res_query \
    res_search \
    res_querydomain \
    res_ninit \
    res_nclose \
    res_nquery \
    res_nsearch \
    res_nquerydomain \
    __res_ninit \
    __res_nclose \
    __res_nquery \
    __res_nsearch \
    __res_nquerydomain])

AC_CHECK_FUNCS([getipnodebyname \
    getipnodebyaddr])

AC_CHECK_FUNCS([pthread_attr_init \
  pthread_attr_getstacksize \
  pthread_attr_setstacksize \
  pthread_rwlock_rdlock \
  pthread_rwlock_wrlock \
  pthread_rwlock_unlock \
  pthread_setconcurrency])

AC_CHECK_FUNCS([ldap_init \
  ldap_bind \
  ldap_search \
  ldap_error \
  ldap_result])

AC_CHECK_FUNCS([smfi_setbacklog \
  smfi_stop \
  smfi_settimeout \
  smfi_quarantine \
  smfi_opensocket \
  smfi_progress \
  smfi_insheader \
  smfi_setsymlist \
  smfi_chgfrom \
  smfi_addrcpt_par \
  smfi_setminworkers])

AC_CHECK_FUNCS([erf erfc])

AC_FUNC_STRFTIME 

AC_CHECK_DECLS([POLLIN, POLLOUT, POLLPRI, POLLNVAL, POLLERR, POLLHUP],,
,[
#if HAVE_POLL_H
#include <poll.h>
#endif
#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
])

AC_CHECK_DECLS([strlcpy, strlcat, fchmod, pread, pwrite])


dnl ############################################################
dnl
dnl
dnl
AC_MSG_CHECKING([wheather sysinfo returns nodename (Solaris)])
AC_TRY_LINK([
#if HAVE_SYS_SYSTEMINFO_H
#include <sys/systeminfo.h>
#endif
],[
  char  host[256];

  sysinfo(SI_HOSTNAME,host,sizeof(host));
],\
  [jm_have_si_hostname=yes],\
  [jm_have_si_hostname=no])

AC_MSG_RESULT($jm_have_si_hostname)

if test "$jm_have_si_hostname" = "yes" ; then
  AC_DEFINE(SYSINFO_NODENAME,1,[si_hostname])
fi

AC_MSG_CHECKING([wheather sysinfo returns system load (Linux)])
AC_TRY_LINK([
#if HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif
#include <stdio.h>
#include <linux/unistd.h>
],[
  struct sysinfo s_info;
  int error;

  error = sysinfo(&s_info);
  printf("code error = %d\n", error);
  printf("Uptime = %ds\nLoad: 1 min %d / 5 min %d / 15 min %d\n"
         "RAM: total %d / free %d / shared %d\n"
         "Memory in buffers = %d\nSwap: total %d / free %d\n"
         "Number of processes = %d\n",
          s_info.uptime, s_info.loads[0],
          s_info.loads[1], s_info.loads[2],
          s_info.totalram, s_info.freeram,
          s_info.sharedram, s_info.bufferram,
          s_info.totalswap, s_info.freeswap,
          s_info.procs);
  return(0);
],\
  [jm_sysinfo_sysload=yes],\
  [jm_sysinfo_sysload=no])

AC_MSG_RESULT($jm_sysinfo_sysload)

if test "$jm_sysinfo_sysload" = "yes" ; then
  AC_DEFINE(SYSINFO_SYSLOAD,1,[sysinfo_sysload])
fi
 
 
dnl ############################################################
dnl
dnl
dnl
AC_CHECK_DECLS([__func__, __FUNCTION__, __PRETTY_FUNCTION__])


dnl ############################################################
dnl
dnl
dnl
dnl
AC_MSG_CHECKING([wheather compiler defines PATH_MAX])
AC_TRY_LINK([
#include <stdio.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
],[
  printf("%d\n",PATH_MAX);
],\
  [jm_have_path_max=yes],\
  [jm_have_path_max=no])

AC_MSG_RESULT($jm_have_path_max)

if test "$jm_have_path_max" = "yes" ; then
  AC_DEFINE(HAVE_PATH_MAX,1,[PATH_MAX])
fi



dnl ############################################################
dnl
dnl
dnl
dnl Let's see what user wants...
dnl
dnl
dnl P C R E
dnl

echo "*** Checking how PCRE shall be integrated..."
PCRE_PATH=""
USE_LIBPCRE="bundled"

PCREDIR="libpcre"
AC_CONFIG_SUBDIRS(libpcre)
dnl BDBLIBS="../$BDBDIR/libpcre.a"
dnl BDBINCS="-I../$BDBDIR/"

LDLIBPCRE=""

AC_DEFINE(USE_PCRE,1,[pcre])
AC_DEFINE(HAVE_PCRE_H,1,[pcre])
AC_DEFINE(HAVE_LIBPCRE,1,[pcre])
CPPFLAGS="$CPPFLAGS -I../$PCREDIR"
LDLIBPCRE="../$PCREDIR/.libs/libpcre.a"
dnl LDLIBPCRE="../$PCREDIR/libpcre.la"

AC_SUBST(PCREDIR)
AC_SUBST(LDLIBPCRE)

echo "*** $USE_LIBPCRE : $PCREDIR"
echo ""

dnl
dnl B e r k e l e y   D B
dnl
echo "*** Checking how Berkeley DB shall be integrated..."

WITH_BDB=bundled
BDBDIR="libdb"
AC_CONFIG_SUBDIRS(libdb)
BDBLIBS="-L../$BDBDIR -ldb"
dnl BDBLIBS="../$BDBDIR/libdb.a"
BDBINCS="-I../$BDBDIR/"
USE_LIBDB="bundled"
CPPFLAGS="$CPPFLAGS -I. -I../$BDBDIR"
AC_DEFINE(USE_BerkeleyDB,1,[BerkeleyDB])
AC_DEFINE(HAVE_LIBDB,1,[db library])
AC_DEFINE(HAVE_DB_H,1,[db include])

AC_SUBST(BDBDIR)
AC_SUBST(BDBLIBS)
AC_SUBST(BDBINCS)

echo "*** $WITH_BDB : $BDBDIR"
echo ""

dnl
dnl
echo ""
dnl
dnl

dnl ############################################################
dnl
dnl XXX
dnl Still needed ???
dnl
if test "$ac_cv_lib_resolv_res_nquery" = "yes" ;
then
  AC_DEFINE(HAVE_RES_NQUERY,1,[libbind res_nquery])
  AC_DEFINE(HAVE_RES_NQUERYDOMAIN,1,[libbind res_nquerydomain])
  AC_DEFINE(HAVE_RES_NSEARCH,1,[libbind res_nsearch])
  AC_DEFINE(HAVE_ARPA_INET_H,1,[arpa/inet.h])
  AC_DEFINE(HAVE_NETDB_H,1,[netdb.h])
  AC_DEFINE(HAVE_ARPA_NAMESER_H,1,[arpa/nameser.h])
  AC_DEFINE(HAVE_RESOLV_H,1,[resolv.h])
fi

dnl ############################################################
MODULES=""
dnl MODS=`cat modules/modules.txt | grep -v #`
MODS=`cat modules/modules.txt `
MODMAKE=""
MODDIRS=""
for mod in $MODS
do
  MODULES="$MODULES $mod"
  MODMAKE="$MODMAKE modules/$mod/Makefile"
  MODDIRS="$MODDIRS modules/$mod"
done

AC_SUBST(MODDIRS)
AC_SUBST(MODULES)

dnl ############################################################
dnl
CFARGS=`echo $ac_configure_args | sed s/\'//g`
AC_SUBST(CFARGS)

dnl ############################################################
dnl
TOPDIR=`pwd`
AC_SUBST(TOPDIR)

ZEINCS="-I$TOPDIR/libze/include -I$TOPDIR/libzechk/include -I$TOPDIR/libml/include -I$TOPDIR/libzefilter/include -I$TOPDIR/libpcre -I$TOPDIR/libdb"
AC_SUBST(ZEINCS)

dnl from line 963
CPPFLAGS="$CPPFLAGS -I../include $ZEINCS"

ZELIBS=" -L$TOPDIR/libjoe -ljoe"
ZELIBS="$ZELIBS -L$TOPDIR/libml/src -lml"
ZELIBS="$ZELIBS -L$TOPDIR/libzechk/src -lzechk"
ZELIBS="$ZELIBS -L$TOPDIR/libs -lcommon"
ZELIBS="$ZELIBS -L$TOPDIR/libze/src -lze"


AC_SUBST(ZELIBS)
dnl
dnl

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(include/version.h include/defs.h)

AC_CONFIG_FILES(libjoe/Makefile)
AC_CONFIG_FILES(libs/Makefile)
AC_CONFIG_FILES(libze/src/Makefile)
AC_CONFIG_FILES(libzechk/src/Makefile)
AC_CONFIG_FILES(libml/src/Makefile)

AC_CONFIG_FILES(src/Makefile)
AC_CONFIG_FILES(scratch/Makefile)

AC_CONFIG_FILES(ze-bayes-tbx/Makefile)
AC_CONFIG_FILES(ze-logreg/Makefile)
AC_CONFIG_FILES(ze-makemap/Makefile)
AC_CONFIG_FILES(ze-scanfile/Makefile)
AC_CONFIG_FILES(ze-printstats/Makefile)
AC_CONFIG_FILES(ze-greyd/Makefile)
AC_CONFIG_FILES(ze-logserver/Makefile)


AC_CONFIG_FILES(config/etc/Makefile)
AC_CONFIG_FILES(config/cdb/Makefile)
AC_CONFIG_FILES(config/cdb/Makefile.org)
AC_CONFIG_FILES(config/cdb/update-db.sh.org)
AC_CONFIG_FILES(config/startup/ze-filter.init)
AC_CONFIG_FILES(config/startup/ze-greyd.init)
AC_CONFIG_FILES(config/startup/ze-filter.init.rhel)
AC_CONFIG_FILES(config/startup/ze-filter.service)
AC_CONFIG_FILES(config/startup/ze-greyd.service)
AC_CONFIG_FILES(config/startup/Makefile)

dnl AC_CONFIG_FILES([$MODMAKE])

AC_CONFIG_COMMANDS([default],[[chmod 755 bin/ze-filter.init]
			      [chmod 755 bin/ze-filter.init.rhel]
			      [chmod 755 bin/ze-greyd.init]
                              [echo ""]
                              [echo "*** Configuring PCRE"]],
                              [[]])

AC_OUTPUT

echo ""

dnl ############################################################
dnl
dnl
dnl

log_conf ()
{
  echo $1 >> config.results
}

BDBVER=
if test -f libdb/bdb.version 
then
  BDBVER=`cat libdb/bdb.version | head -1`
  BDBVER=`basename $BDBVER`
fi

PCREVER=
if test -f libpcre/pcre.version
then
  PCREVER=`cat libpcre/pcre.version | head -1`
  PCREVER=`basename $PCREVER`
fi

cp /dev/null config.results

echo "" >> config.results
echo "Configuration results : `date`" >> config.results
echo "-----------------------" >> config.results
echo "  configure arguments      : $CFARGS" >> config.results
echo "" >> config.results
echo "  Source directory         : $TOPDIR" >> config.results
echo "  Package                  : $PACKAGE" >> config.results
echo "  Version                  : $VERSION" >> config.results
echo "  Host type                : $host" >> config.results
echo "  Host OS                  : $host_os" >> config.results
echo "  Host CPU                 : $host_cpu" >> config.results
echo "  ze-filter user           : $USER" >> config.results
echo "  ze-filter group          : $GROUP" >> config.results
echo "  prefix dir               : $prefix" >> config.results
echo "  start-up script dir      : $RCDIR" >> config.results
echo "  start-up scripts         : ze-filter$RCSUFFIX ze-greyd$RCSUFFIX" >> config.results
echo "  configuration files dir  : $CONFDIR" >> config.results
echo "  work dir                 : $WORKROOT" >> config.results
echo "  state dir                : $STATEDIR" >> config.results
echo "  spool dir                : $SPOOLDIR" >> config.results
echo "  ze-greyd dir             : $ZEGREYDDIR" >> config.results
echo "  " >> config.results
echo "  USE PCRE                 : $USE_LIBPCRE - $PCREVER" >> config.results
echo "  USE BerkeleyDB           : $USE_LIBDB - $BDBVER" >> config.results
echo "  " >> config.results
echo "  MODULES                  : $MODULES" >> config.results
echo "  " >> config.results
echo "  CC                       : $CC" >> config.results
echo "  CPPFLAGS                 : $CPPFLAGS" >> config.results
echo "  CFLAGS                   : $CFLAGS" >> config.results
echo "  LIBS                     : $LIBS" >> config.results
echo "  LDFLAGS                  : $LDFLAGS" >> config.results
echo "  ZELIBS                   : $ZELIBS" >> config.results
echo "  " >> config.results

if test -f aux/configure.footer
then
  cat aux/configure.footer >> config.results
fi

if [ test "x$enable_shared" = "xyes" -a "x$enable_static" = "xno" ]
then
  cat aux/warning.static >> config.results
fi

cat config.results


