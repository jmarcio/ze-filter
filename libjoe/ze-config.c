/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur     : Jose Marcio Martins da Cruz
 *               jose.marcio.mc@gmail.org
 *
 *  Historique :
 *  Creation     : janvier 2002
 *
 * This program is free software, but with restricted license :
 *
 * - j-chkmail is distributed only to registered users
 * - j-chkmail license is available only non-commercial applications,
 *   this means, you can use j-chkmail if you make no profit with it.
 * - redistribution of j-chkmail in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about j-chkmail license can be found at j-chkmail
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>

#include "ze-filter.h"

#undef  DEBUG
#define DEBUG 0

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define OPT_REC_INITIALIZER \
  {FALSE, 0, FALSE, FALSE, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE}

OPT_REC_T           cf_opt = OPT_REC_INITIALIZER;

static char        *compileConf[] = {
#ifdef _POSIX_SOURCE
  "_POSIX_SOURCE",
#endif
#if STDC_HEADERS
  "STDC_HEADERS",
#endif
#if TIME_WITH_SYS_TIME
  "TIME_WITH_SYS_TIME",
#endif
#if HAVE_INTTYPES_H
  "HAVE_INTTYPES_H",
#endif
#if HAVE_THREAD_H
  "HAVE_THREAD_H",
#endif
#if HAVE_PTHREAD_H
  "HAVE_PTHREAD_H",
#endif
#if HAVE_SYS_PTHREAD_H
  "HAVE_SYS_PTHREAD_H",
#endif
#if HAVE_VALUES_H
  "HAVE_VALUES_H",
#endif
#if HAVE_LIMITS_H
  "HAVE_LIMITS_H",
#endif
#if HAVE_MATH_H
  "HAVE_MATH_H",
#endif
#if HAVE_MEMORY_H
  "HAVE_MEMORY_H",
#endif
#if HAVE_REGEX_H
  "HAVE_REGEX_H",
#endif
#if HAVE_SEARCH_H
  "HAVE_SEARCH_H",
#endif
#if HAVE_STDINT_H
  "HAVE_STDINT_H",
#endif
#if HAVE_STDARG_H
  "HAVE_STDARG_H",
#endif
#if HAVE_STDLIB_H
  "HAVE_STDLIB_H",
#endif
#if HAVE_STDBOOL_H
  "HAVE_STDBOOL_H",
#endif
#if HAVE_STRING_H
  "HAVE_STRING_H",
#endif
#if HAVE_STRINGS_H
  "HAVE_STRINGS_H",
#endif
#if HAVE_SYS_TYPES_H
  "HAVE_SYS_TYPES_H",
#endif
#if HAVE_SYS_STAT_H
  "HAVE_SYS_STAT_H",
#endif
#if HAVE_SYS_TIME_H
  "HAVE_SYS_TIME_H",
#endif
#if HAVE_SYS_WAIT_H
  "HAVE_SYS_WAIT_H",
#endif
#if HAVE_SYS_RESOURCE_H
  "HAVE_SYS_RESOURCE_H",
#endif
#if HAVE_SYS_SOCKET_H
  "HAVE_SYS_SOCKET_H",
#endif
#if HAVE_SYS_MMAN_H
  "HAVE_SYS_MMAN_H",
#endif
#if HAVE_SYS_SYSINFO_H
  "HAVE_SYS_SYSINFO_H",
#endif
#if HAVE_SYS_SYSTEMINFO_H
  "HAVE_SYS_SYSTEMINFO_H",
#endif
#if HAVE_SYS_UTSNAME_H
  "HAVE_SYS_UTSNAME_H",
#endif
#if HAVE_SYS_UN_H
  "HAVE_SYS_UN_H",
#endif
#if HAVE_SYS_SELECT_H
  "HAVE_SYS_SELECT_H",
#endif
#if HAVE_POLL_H
  "HAVE_POLL_H",
#endif
#if HAVE_SYS_POLL_H
  "HAVE_SYS_POLL_H",
#endif
#if HAVE_SYSEXITS_H
  "HAVE_SYSEXITS_H",
#endif
#if HAVE_SYS_CDEFS_H
  "HAVE_SYS_CDEFS_H",
#endif
#if HAVE_SYSLOG_H
  "HAVE_SYSLOG_H",
#endif
#if HAVE_PWD_H
  "HAVE_PWD_H",
#endif
#if HAVE_GRP_H
  "HAVE_GRP_H",
#endif
#if HAVE_UNISTD_H
  "HAVE_UNISTD_H",
#endif
#if HAVE_FCNTL_H
  "HAVE_FCNTL_H",
#endif
#if HAVE_DIRENT_H
  "HAVE_DIRENT_H",
#endif
#if HAVE_SIGNAL_H
  "HAVE_SIGNAL_H",
#endif
#if HAVE_CTYPE_H
  "HAVE_CTYPE_H",
#endif
#if HAVE_NETDB_H
  "HAVE_NETDB_H",
#endif
#if HAVE_NETINET_IN_H
  "HAVE_NETINET_IN_H",
#endif
#if HAVE_ARPA_INET_H
  "HAVE_ARPA_INET_H",
#endif
#if HAVE_ARPA_NAMESER_H
  "HAVE_ARPA_NAMESER_H",
#endif
#if HAVE_RESOLV_H
  "HAVE_RESOLV_H",
#endif
#if HAVE_SYS_LOADAVG_H
  "HAVE_SYS_LOADAVG_H",
#endif
#if HAVE_DLFCN_H
  "HAVE_DLFCN_H",
#endif
#if HAVE_LINK_H
  "HAVE_LINK_H",
#endif
#if HAVE_KVM_H
  "HAVE_KVM_H",
#endif
#if HAVE_SYS_DKSTAT_H
  "HAVE_SYS_DKSTAT_H",
#endif

#if HAVE_DB_H
  "HAVE_DB_H",
#endif
#if HAVE_NDBM_H
  "HAVE_NDBM_H",
#endif
#if HAVE_GDBM_H
  "HAVE_GDBM_H",
#endif
#if HAVE_LIBMILTER_MFDEF_H
  "HAVE_LIBMILTER_MFDEF_H",
#endif
#if HAVE_MFAPI_H
  "HAVE_MFAPI_H",
#endif
#if HAVE_MFDEF_H
  "HAVE_MFDEF_H",
#endif

#if HAVE_BOOL
  "HAVE_BOOL",
#endif
#if HAVE__BOOL
  "HAVE__BOOL",
#endif
#if HAVE_LONG_LONG
  "HAVE_LONG_LONG",
#endif
#if HAVE_UCHAR
  "HAVE_UCHAR",
#endif
#if HAVE_INT8_T
  "HAVE_INT8_T",
#endif
#if HAVE_UINT8_T
  "HAVE_UINT8_T",
#endif
#if HAVE_INT16_T
  "HAVE_INT16_T",
#endif
#if HAVE_UINT16_T
  "HAVE_UINT16_T",
#endif
#if HAVE_INT32_T
  "HAVE_INT32_T",
#endif
#if HAVE_INT64_T
  "HAVE_INT64_T",
#endif
#if HAVE_UINT32_T
  "HAVE_UINT32_T",
#endif
#if HAVE_UINT64_T
  "HAVE_UINT64_T",
#endif
#if HAVE_U_INT32_T
  "HAVE_U_INT32_T",
#endif
#if HAVE_SIZE_T
  "HAVE_SIZE_T",
#endif
#if HAVE_SSIZE_T
  "HAVE_SSIZE_T",
#endif
#if HAVE_SOCKET_T
  "HAVE_SOCKET_T",
#endif
#if HAVE_SOCKLEN_T
  "HAVE_SOCKLEN_T",
#endif
#if HAVE_STRUCT_ADDRINFO
  "HAVE_STRUCT_ADDRINFO",
#endif
#if HAVE_STRUCT_SOCKADDR
  "HAVE_STRUCT_SOCKADDR",
#endif
#if HAVE_STRUCT_SOCKADDR_IN
  "HAVE_STRUCT_SOCKADDR_IN",
#endif
#if HAVE_STRUCT_SOCKADDR_IN6
  "HAVE_STRUCT_SOCKADDR_IN6",
#endif
#if HAVE_STRUCT_SOCKADDR_UN
  "HAVE_STRUCT_SOCKADDR_UN",
#endif
#if HAVE_MODE_T
  "HAVE_MODE_T",
#endif
#if HAVE_OFF_T
  "HAVE_OFF_T",
#endif
#if HAVE_GID_T
  "HAVE_GID_T",
#endif
#if HAVE_UID_T
  "HAVE_UID_T",
#endif
#if HAVE_PID_T
  "HAVE_PID_T",
#endif
#if HAVE_PTHREAD_MUTEX_T
  "HAVE_PTHREAD_MUTEX_T",
#endif
#if HAVE_PTHREAD_RWLOCK_T
  "HAVE_PTHREAD_RWLOCK_T",
#endif
#if HAVE_MUTEX_INITIALIZER
  "HAVE_MUTEX_INITIALIZER",
#endif


#if HAVE_HRTIME_T
  "HAVE_HRTIME_T",
#endif
#if HAVE_IN_ADDR_T
  "HAVE_IN_ADDR_T",
#endif
#if HAVE_IN6_ADDR_T
  "HAVE_IN6_ADDR_T",
#endif
#if HAVE_STRUCT_IN_ADDR
  "HAVE_STRUCT_IN_ADDR",
#endif
#if HAVE_STRUCT_IN6_ADDR
  "HAVE_STRUCT_IN6_ADDR",
#endif
#if RETSIGTYPE
  "RETSIGTYPE",
#endif

#if HAVE_LIBCOMPAT
  "HAVE_LIBCOMPAT",
#endif
#if HAVE_LIBMILTER
  "HAVE_LIBMILTER",
#endif
#if HAVE_LIBMILTER_MFAPI_H
  "HAVE_LIBMILTER_MFAPI_H",
#endif
#if HAVE_LIBNSL
  "HAVE_LIBNSL",
#endif
#if HAVE_LIBPTHREAD
  "HAVE_LIBPTHREAD",
#endif
#if HAVE_LIBRESOLV
  "HAVE_LIBRESOLV",
#endif
#if HAVE_LIBDB
  "HAVE_LIBDB",
#endif
#if HAVE_LIBGDBM
  "HAVE_LIBGDBM",
#endif
#if HAVE_LIBNDBM
  "HAVE_LIBNDBM",
#endif
#if HAVE_LIBSM
  "HAVE_LIBSM",
#endif
#if HAVE_LIBSOCKET
  "HAVE_LIBSOCKET",
#endif
#if HAVE_LIBKSTAT
  "HAVE_LIBKSTAT",
#endif
#if HAVE_LIBRT
  "HAVE_LIBRT",
#endif
#if HAVE_LIBDL
  "HAVE_LIBDL",
#endif
#if HAVE_LIBDLD
  "HAVE_LIBDLD",
#endif
#if HAVE_LIBKVM
  "HAVE_LIBKVM",
#endif


#if HAVE_MAX
  "HAVE_MAX",
#endif
#if HAVE_GETHOSTNAME
  "HAVE_GETHOSTNAME",
#endif
#if HAVE_GETHOSTBYADDR
  "HAVE_GETHOSTBYADDR",
#endif
#if HAVE_GETHOSTBYADDR_R
  "HAVE_GETHOSTBYADDR_R",
#endif
#if HAVE_GETHOSTBYNAME
  "HAVE_GETHOSTBYNAME",
#endif
#if HAVE_GETHOSTBYNAME_R
  "HAVE_GETHOSTBYNAME_R",
#endif
#if HAVE_GETIPNODEBYADDR
  "HAVE_GETIPNODEBYADDR",
#endif
#if HAVE_GETIPNODEBYNAME
  "HAVE_GETIPNODEBYNAME",
#endif
#if HAVE_SYSCTLBYNAME
  "HAVE_SYSCTLBYNAME",
#endif

#if HAVE_RES_NQUERY
  "HAVE_RES_NQUERY",
#endif
#if HAVE_RES_NQUERYDOMAIN
  "HAVE_RES_NQUERYDOMAIN",
#endif
#if HAVE_RES_QUERY
  "HAVE_RES_QUERY",
#endif
#if HAVE_HOSTS_CTL
  "HAVE_HOSTS_CTL",
#endif
#if HAVE_RES_QUERYDOMAIN
  "HAVE_RES_QUERYDOMAIN",
#endif

#if HAVE_FREEADDRINFO
  "HAVE_FREEADDRINFO",
#endif
#if HAVE_GETADDRINFO
  "HAVE_GETADDRINFO",
#endif
#if HAVE_GETNAMEINFO
  "HAVE_GETNAMEINFO",
#endif

#if HAVE_INET_ADDR
  "HAVE_INET_ADDR",
#endif
#if HAVE_INET_NTOA
  "HAVE_INET_NTOA",
#endif
#if HAVE_INET_ATON
  "HAVE_INET_ATON",
#endif
#if HAVE_INET_NTOP
  "HAVE_INET_NTOP",
#endif
#if HAVE_INET_PTON
  "HAVE_INET_PTON",
#endif
#if HAVE_GETRUSAGE
  "HAVE_GETRUSAGE",
#endif
#if HAVE_GETRLIMIT
  "HAVE_GETRLIMIT",
#endif
#if HAVE_SETRLIMIT
  "HAVE_SETRLIMIT",
#endif
#if HAVE_MEMCPY
  "HAVE_MEMCPY",
#endif
#if HAVE_ALLOCA
  "HAVE_ALLOCA",
#endif

#if HAVE_REGCOMP
  "HAVE_REGCOMP",
#endif
#if HAVE_SOCKET
  "HAVE_SOCKET",
#endif
#if HAVE_SELECT
  "HAVE_SELECT",
#endif
#if HAVE_POLL
  "HAVE_POLL",
#endif
#if HAVE_SOCKETPAIR
  "HAVE_SOCKETPAIR",
#endif

#if HAVE_PTHREAD_SETCONCURRENCY
  "HAVE_PTHREAD_SETCONCURRENCY",
#endif
#if HAVE_THR_SETCONCURRENCY
  "HAVE_THR_SETCONCURRENCY",
#endif
#if HAVE_PTHREAD_ATTR_INIT
  "HAVE_PTHREAD_ATTR_INIT",
#endif
#if HAVE_PTHREAD_ATTR_GETSTACKSIZE
  "HAVE_PTHREAD_ATTR_GETSTACKSIZE",
#endif
#if HAVE_PTHREAD_ATTR_SETSTACKSIZE
  "HAVE_PTHREAD_ATTR_SETSTACKSIZE",
#endif
#if HAVE_PTHREAD_RWLOCK_UNLOCK
  "HAVE_PTHREAD_RWLOCK_UNLOCK",
#endif
#if HAVE_PTHREAD_RWLOCK_WRLOCK
  "HAVE_PTHREAD_RWLOCK_WRLOCK",
#endif
#if HAVE_PTHREAD_RWLOCK_RDLOCK
  "HAVE_PTHREAD_RWLOCK_RDLOCK",
#endif

#if HAVE_STRCHR
  "HAVE_STRCHR",
#endif
#if HAVE_STRTOL
  "HAVE_STRTOL",
#endif
#if HAVE_STRCSPN
  "HAVE_STRCSPN",
#endif
#if HAVE_STRDUP
  "HAVE_STRDUP",
#endif
#if HAVE_STRERROR
  "HAVE_STRERROR",
#endif
#if HAVE_STRFTIME
  "HAVE_STRFTIME",
#endif
#if HAVE_CTIME
  "HAVE_CTIME",
#endif
#if HAVE_CTIME_R
  "HAVE_CTIME_R",
#endif
#if HAVE_STRLCPY
  "HAVE_STRLCPY",
#endif
#if HAVE_STRSPN
  "HAVE_STRSPN",
#endif
#if HAVE_STRSTR
  "HAVE_STRSTR",
#endif
#if HAVE_VSNPRINTF
  "HAVE_VSNPRINTF",
#endif
#if HAVE_VSPRINTF
  "HAVE_VSPRINTF",
#endif
#if HAVE_MKSTEMP
  "HAVE_MKSTEMP",
#endif
#if HAVE_MKTEMP
  "HAVE_MKTEMP",
#endif
#if HAVE_OPENDIR
  "HAVE_OPENDIR",
#endif
#if HAVE_READDIR
  "HAVE_READDIR",
#endif
#if HAVE_READDIR_R
  "HAVE_READDIR_R",
#endif
#if HAVE_MMAP
  "HAVE_MMAP",
#endif
#if HAVE_MUNMAP
  "HAVE_MUNMAP",
#endif
#if HAVE_PATH_MAX
  "HAVE_PATH_MAX",
#endif
#if HAVE_SHM_OPEN
  "HAVE_SHM_OPEN",
#endif
#if HAVE_SHM_UNLINK
  "HAVE_SHM_UNLINK",
#endif
#if HAVE_SYSCONF
  "HAVE_SYSCONF",
#endif
#if HAVE_SYSINFO
  "HAVE_SYSINFO",
#endif
#if HAVE_UNAME
  "HAVE_UNAME",
#endif
#if HAVE_FTRUNCATE
  "HAVE_FTRUNCATE",
#endif
#if HAVE_PREAD
  "HAVE_PREAD",
#endif
#if HAVE_PWRITE
  "HAVE_PWRITE",
#endif
#if HAVE_ERF
  "HAVE_ERF",
#endif
#if HAVE_ERFC
  "HAVE_ERFC",
#endif
#if HAVE_ERFI
  "HAVE_ERFI",
#endif
#if NEED_SM_SNPRINTF
  "NEED_SM_SNPRINTF",
#endif
#if HAVE_GETLOADAVG
  "HAVE_GETLOADAVG",
#endif

#if HAVE_KSTAT_CLOSE
  "HAVE_KSTAT_CLOSE",
#endif
#if HAVE_KSTAT_H
  "HAVE_KSTAT_H",
#endif
#if HAVE_KSTAT_OPEN
  "HAVE_KSTAT_OPEN",
#endif
#if HAVE_KSTAT_READ
  "HAVE_KSTAT_READ",
#endif
#if HAVE_KSTAT_LOOKUP
  "HAVE_KSTAT_LOOKUP",
#endif


#if HAVE_LSTAT
  "HAVE_LSTAT",
#endif
#if HAVE_STAT
  "HAVE_STAT",
#endif
#if HAVE_FCHMOD
  "HAVE_FCHMOD",
#endif

#if HAVE_CLOCK
  "HAVE_CLOCK",
#endif
#if HAVE_TIME
  "HAVE_TIME",
#endif
#if HAVE_SLEEP
  "HAVE_SLEEP",
#endif
#if HAVE_USLEEP
  "HAVE_USLEEP",
#endif
#if HAVE_GETHRTIME
  "HAVE_GETHRTIME",
#endif

#if HAVE_LIBPCRE
  "HAVE_LIBPCRE",
#endif
#if HAVE_PCRE_H
  "HAVE_PCRE_H",
#endif

#if HAVE_LIBCLAMAV
  "HAVE_LIBCLAMAV",
#endif
#if HAVE_CLAMAV_H
  "HAVE_CLAMAV_H",
#endif

#if HAVE_SMFI_SETBACKLOG
  "HAVE_SMFI_SETBACKLOG",
#endif
#if HAVE_SMFI_SIGNAL
  "HAVE_SMFI_SIGNAL",
#endif
#if HAVE_SMFI_STOP
  "HAVE_SMFI_STOP",
#endif
#if HAVE_SMFI_OPENSOCKET
  "HAVE_SMFI_OPENSOCKET",
#endif
#if HAVE_SMFI_PROGRESS
  "HAVE_SMFI_PROGRESS",
#endif
#if HAVE_SMFI_QUARANTINE
  "HAVE_SMFI_QUARANTINE",
#endif

#if HAVE_DBM_DB
  "HAVE_DBM_DB",
#endif
#if HAVE_DBM_NDBM
  "HAVE_DBM_NDBM",
#endif
#if HAVE_DBM_GDBM
  "HAVE_DBM_GDBM",
#endif

#if HAVE_DECL_POLLERR
  "HAVE_DECL_POLLERR",
#endif
#if HAVE_DECL_POLLHUP
  "HAVE_DECL_POLLHUP",
#endif
#if HAVE_DECL_POLLIN
  "HAVE_DECL_POLLIN",
#endif
#if HAVE_DECL_POLLNVAL
  "HAVE_DECL_POLLNVAL",
#endif
#if HAVE_DECL_POLLOUT
  "HAVE_DECL_POLLOUT",
#endif
#if HAVE_DECL_POLLPRI
  "HAVE_DECL_POLLPRI",
#endif
#if HAVE_DECL_FCHMOD
  "HAVE_DECL_FCHMOD",
#endif
#if HAVE_DECL_PREAD
  "HAVE_DECL_PREAD",
#endif
#if HAVE_DECL_PWRITE
  "HAVE_DECL_PWRITE",
#endif
#if HAVE_DECL_STRLCPY
  "HAVE_DECL_STRLCPY",
#endif


#if HAVE_DECL___FUNCTION__
  "HAVE_DECL___FUNCTION__",
#endif
#if HAVE_DECL___FUNC__
  "HAVE_DECL___FUNC__",
#endif
#if HAVE_DECL___PRETTY_FUNCTION__
  "HAVE_DECL___PRETTY_FUNCTION__",
#endif

  NULL
};

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
dump_j_conf(fd)
     int                 fd;
{
  char                str[256];
  char              **s;
  struct rlimit       rlp;

#if 0
  if (fd < 0)
    fd = STDOUT_FILENO;
#endif

  FD_PRINTF(fd, "Joe's j-chkmail configuration : %s\n", PACKAGE);

  if (cf_opt.arg_v > 1)
  {

    FD_PRINTF(fd, "\n");
    FD_PRINTF(fd, "---> j-chkmail compile-time configuration\n");
    FD_PRINTF(fd, "     Compiled on %s %s\n", __DATE__, __TIME__);
    FD_PRINTF(fd, "     %s\n\n", UNAME);

    s = compileConf;
    *str = '\0';
    while (*s)
    {
      sprintf(str, "%s %-26s", str, *s);
      if (strlen(str) > 48)
      {
        FD_PRINTF(fd, "      %s\n", str);
        *str = '\0';
      }
      s++;
    }
    FD_PRINTF(fd, "      %s\n\n", str);
    FD_PRINTF(fd, "       RUN_AS_USER  : %s\n", RUN_AS_USER);
    FD_PRINTF(fd, "       RUN_AS_GROUP : %s\n", RUN_AS_GROUP);
    FD_PRINTF(fd, "      \n");
  }

  FD_PRINTF(fd, "--> j-chkmail command line options\n");
  *str = '\0';
  if (cf_opt.arg_h)
  {
    strcat(str, " -h");
  }
  if (cf_opt.arg_v)
  {
    strcat(str, " -v");
  }
  FD_PRINTF(fd, "\n");

  if (cf_opt.arg_p != NULL)
  {
    strcat(str, " -p ");
    strcat(str, cf_opt.arg_p);
  }
  if (cf_opt.arg_i != NULL)
  {
    strcat(str, " -i ");
    strcat(str, cf_opt.arg_i);
  }
  if (cf_opt.arg_u != NULL)
  {
    strcat(str, " -u ");
    strcat(str, cf_opt.arg_u);
  }
  if (cf_opt.arg_c != NULL)
  {
    strcat(str, " -c ");
    strcat(str, cf_opt.arg_c);
  }
  if (cf_opt.arg_l != NULL)
  {
    strcat(str, " -l ");
    strcat(str, cf_opt.arg_l);
  }
  if (strlen(str) > 0)
    FD_PRINTF(fd, " %s\n", str);

  FD_PRINTF(fd, "\n");
  FD_PRINTF(fd, "---> Configuration file options : \n");
  cf_dump(fd, cf_opt.arg_v > 1);

  *str = '\0';
  FD_PRINTF(fd, "    FILE NAME EXTENSIONS :\n");
  list_filename_extensions(fd);
  FD_PRINTF(fd, "\n");

#if HAVE_GETRLIMIT
  if (getrlimit(RLIMIT_NOFILE, &rlp) == 0)
  {
    FD_PRINTF(fd, "       RLIMIT_NOFILE    : %6ld (soft) - %6ld (hard)\n",
              (long) rlp.rlim_cur, (long) rlp.rlim_max);
    FD_PRINTF(fd, "       FD_SETSIZE       : %6ld\n", (long) FD_SETSIZE);
    FD_PRINTF(fd, "       USE_SELECT_LIMIT : %6s\n",
              cf_get_int(CF_USE_SELECT_LIMIT) == OPT_YES ? "YES" : "NO");
  }
#endif
  FD_PRINTF(fd, "\n");
}
