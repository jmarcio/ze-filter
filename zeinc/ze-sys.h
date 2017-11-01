/*
 *
 * ze-filter - Mail Server Filter for sendmail
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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#ifndef __J_SYS_H__


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif             /* HAVE_CONFIG_H */

#if 0
#ifndef _REENTRANT
#define _REENTRANT
#endif             /* _REENTRANT */
#endif

#define OS_OTHER

/*
**     S U N   S O L A R I S
**
*/
#if defined(OS_SOLARIS)

#undef OS_OTHER

#if 0
#ifndef _POSIX_SOURCE
# define _POSIX_SOURCE
#endif             /* _POSIX_SOURCE */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 199506L
#endif             /* _POSIX_C_SOURCE */

#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS
#endif             /* _POSIX_PTHREAD_SEMANTICS */

#define __EXTENSIONS__
#endif

#endif             /* OS_SOLARIS */

/*
**     L I N U X 
**
*/
#if defined(OS_LINUX)

#undef OS_OTHER

/* may delete all this - deprecated */
#if 0

#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199506L
#define __USE_UNIX98
#if HAVE_PTHREAD_H
#define _POSIX_PTHREAD_SEMANTICS
#endif             /* HAVE_PTHREAD_H */
#define _BSD_SOURCE
#define _XOPEN_SOURCE 600
#define _XOPEN_SOURCE_EXTENDED 1
#ifndef __STDC__
#define __STDC__
#endif             /* __STDC__ */
#define __EXTENSIONS__

#endif             /* 0 */

#endif

/*
**     F R E E   B S D
**
*/
#if defined(OS_FREEBSD)

#undef OS_OTHER

#if 0
#ifndef __STDC__
#define __STDC__
#endif             /* __STDC__ */
#define __BSD_VISIBLE 1
#endif

#endif             /* OS_FREEBSD */

/*
**     T R U 6 4
**
*/
#if defined(OS_TRU64)

#undef OS_OTHER

#define _XOPEN_SOURCE 600
#define _XOPEN_SOURCE_EXTENDED 1

#ifndef _POSIX_SOURCE
# define _POSIX_SOURCE
#endif             /* _POSIX_SOURCE */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 199506L
#endif             /* _POSIX_C_SOURCE */

#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS
#endif             /* _POSIX_PTHREAD_SEMANTICS */

#ifndef _POSIX_SHARED_MEMORY_OBJECTS
# define _POSIX_SHARED_MEMORY_OBJECTS
#endif             /* _POSIX_SHARED_MEMORY_OBJECTS */

#if 0
#ifndef __STDC__
#define __STDC__
#endif             /* __STDC__ */
#endif

#if 0
#define _BSD_SOURCE
#endif

#undef HAVE_GETIPNODEBYADDR
#undef HAVE_GETIPNODEBYNAME

#endif             /* OS_TRU64 */

/*
**     H P - U X
**
*/
#if defined(OS_HPUX)

#undef OS_OTHER

#if 0
#ifndef __STDC__
#define __STDC__
#endif             /* __STDC__ */
#define __BSD_VISIBLE 1
#endif

#endif             /* OS_HPUX */

/*
**     N E T     B S D
**
*/
#if defined(OS_NETBSD)

#undef OS_OTHER

#ifndef _POSIX_SOURCE
# define _POSIX_SOURCE
#endif             /* _POSIX_SOURCE */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 199506L
#endif             /* _POSIX_C_SOURCE */

#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS
#endif             /* _POSIX_PTHREAD_SEMANTICS */

#ifndef _POSIX_SHARED_MEMORY_OBJECTS
# define _POSIX_SHARED_MEMORY_OBJECTS
#endif             /* _POSIX_SHARED_MEMORY_OBJECTS */

#endif             /* OS_NETBSD */

/*
**
**
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif             /* HAVE_STDDEF_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif             /* HAVE_UNISTD_H */

#if STDC_HEADERS
# include <string.h>
#else
# if HAVE_STRING_H
#  include <string.h>
# endif            /* HAVE_STRING_H */
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif            /* STDC_STRCHR */
char               *strchr(), *strrchr();

# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif            /* HAVE_MEMCPY */
#endif             /* STDC_HEADERS */

#if HAVE_STRINGS_H
#include <strings.h>
#endif             /* HAVE_STRINGS_H */

#if HAVE_STRING_H
#include <string.h>
#endif             /* HAVE_STRING_H */

#if HAVE_SYSEXITS_H
#include <sysexits.h>
#endif             /* hVE_SYSEXITS_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif             /* HAVE_SYS_TYPES_H */

#if HAVE_LIMITS_H
#include <limits.h>
#else
#if HAVE_VALUES_H
#include <values.h>
#else
Error - dont have limits.h nor values.h
#endif             /* HAVE_VALUES_H */
#endif             /* HAVE_LIMITS_H */


/* ---------- */


#ifndef __bool_true_false_are_defined
# if !defined (HAVE_BOOL)
typedef int         bool;
# endif /* HAVE_BOOL */
# define  __bool_true_false_are_defined 1
#endif /* __bool_true_false_are_defined */

#ifndef true
# define true   1
# define false  0
#endif


#if HAVE_STDARG_H
#include <stdarg.h>
#endif             /* HAVE_STDARG_H */

#if HAVE_MATH_H
#include <math.h>
#endif             /* HAVE_MATH_H */

#include <errno.h>

#if TIME_WITH_SYS_TIME
# include <time.h>
# include <sys/time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif            /* HAVE_SYS_TIME_H */
#endif             /* TIME_WITH_SYS_TIME */

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif             /* HAVE_SYS_RESOURCE_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif             /* HAVE_SIGNAL_H */

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif             /* HAVE_SYS_WAIT_H */
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned int)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif             /* HAVE_FCNTL_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif             /* HAVE_SYS_STAT_H */

#if HAVE_DIRENT_H
#include <dirent.h>
#endif             /* HAVE_DIRENT_H */

#if USE_PCRE
# if !defined(HAVE_PCRE_H) || !defined(HAVE_LIBPCRE)
#  undef USE_PCRE
# endif            /* !defined(HAVE_PCRE_H) || !defined(HAVE_LIBPCRE) */
#endif             /* USE_PCRE */

#if USE_PCRE
# if HAVE_PCRE_H && HAVE_LIBPCRE
#  include <pcre.h>
# else             /* HAVE_PCRE_H && HAVE_LIBPCREX */
#  ERROR : PCRE
# endif            /* HAVE_PCRE_H && HAVE_LIBPCREX */
#endif             /* USE_PCRE */

#if HAVE_REGEX_H
#include <regex.h>
#endif             /* HAVE_REGEX_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif             /* HAVE_CTYPE_H */

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif             /* HAVE_SYS_SOCKET_H */

#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif             /* HAVE_SYS_UN_H */

#if HAVE_POLL_H
#include <poll.h>
#endif             /* HAVE_POLL_H */

#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif             /* HAVE_SYS_POLL_H */

#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif             /* HAVE_SYS_SELECT_H */

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif             /* HAVE_NETINET_IN_H */

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif             /* HAVE_ARPA_INET_H */

#if HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif             /* HAVE_ARPA_NAMESER_H */

#if HAVE_NETDB_H
#include <netdb.h>
#endif             /* HAVE_NETDB_H */

#if HAVE_RESOLV_H
#include <resolv.h>
#endif             /* HAVE_RESOLV_H */

#if HAVE_SYSLOG_H
#include <syslog.h>
#endif             /* HAVE_SYSLOG_H */

#if HAVE_THREAD_H
#include <thread.h>
#endif             /* HAVE_THREAD_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#else
#if HAVE_SYS_PTHREAD_H
#include <sys/pthread.h>
#endif             /* HAVE_SYS_PTHREAD_H */
#endif             /* HAVE_PTHREAD_H */

#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif             /* HAVE_DLFCN_H */

#if HAVE_PWD_H
#include <pwd.h>
#endif             /* HAVE_PWD_H */

#if HAVE_GRP_H
#include <grp.h>
#endif             /* HAVE_GRP_H */

#ifdef HAVE_KSTAT_H
#include <kstat.h>
#endif             /* HAVE_KSTAT_H */

#ifdef HAVE_KVM_H
#include <kvm.h>
#endif             /* HAVE_KVM_H */

#ifdef HAVE_SYS_DKSTAT_H
#include <sys/dkstat.h>
#endif             /* HAVE_SYS_DKSTAT_H */

#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h>
#endif             /* HAVE_SYS_LOADAVG_H */

#ifdef HAVE_SYS_SYSTEMINFO_H
#include <sys/systeminfo.h>
#endif             /* HAVE_SYS_SYSTEMINFO_H */

#ifdef HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif             /* HAVE_SYS_SYSINFO_H */

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif             /* HAVE_SYS_UTSNAME_H */

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif             /* HAVE_SYS_MMAN_H */

#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif /* HAVE_ASSERT_H */

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif             /* HAVE_LIBGEN_H */

#ifndef isblank
# define isblank(c)     ((c) == ' ' || (c) == '\t')
#endif

#if USE_BerkeleyDB
#if HAVE_DB_H
#include <db.h>
#endif             /* HAVE_DB_H */
#endif             /* USE_BerkeleyDB */

#if defined(HAVE_DECL___FUNC__) && (HAVE_DECL___FUNC__ == 1)
# define J_FUNCTION         __func__
# define J_PRETTY_FUNCTION  __func__
#else
# if defined(HAVE_DECL___FUNCTION__) && (HAVE_DECL___FUNCTION__ == 1)
#  define J_FUNCTION         __FUNCTION__
#  define J_PRETTY_FUNCTION  __PRETTY_FUNCTION__
# else
#  define J_FUNCTION         __FILE__
#  define J_PRETTY_FUNCTION  __FILE__
# endif
#endif

#if !defined(HAVE_UCHAR)
typedef unsigned char uchar;
#endif             /* HAVE_UCHAR */

#if !defined(HAVE_INT16_T)
typedef short       int16_t;
#endif             /* HAVE_INT16_T */

#if !defined(HAVE_UINT16_T)
typedef unsigned short uint16_t;
#endif

#if !HAVE_INT32_T
typedef long        uint32_t;
#endif

#if !HAVE_UINT32_T
typedef unsigned long uint32_t;
#endif

#if !defined(HAVE_INT64_T)
# if HAVE_LONG_LONG
#  if  SIZEOF_LONG_LONG == 8
typedef long long int64_t;
#   define  HAVE_INT64_T 1
#  endif /* SIZEOF_LONG_LONG */
# endif /* HAVE_LONG_LONG */
#endif /* HAVE_INT64_T */

#if !defined(HAVE_UINT64_T)
# if HAVE_UNSIGNED_LONG_LONG
#  if  SIZEOF_UNSIGNED_LONG_LONG == 8
typedef unsigned long long uint64_t;
#   define  HAVE_UINT64_T 1
#  endif /* SIZEOF_UNSIGNED_LONG_LONG */
# endif /* HAVE_UNSIGNED_LONG_LONG */
#endif /* HAVE_UINT64_T */

#if !defined(HAVE_OFF_T)
typedef unsigned long off_t;
#endif

#if !defined(HAVE_MODE_T)
typedef unsigned int mode_t;
#endif

#if !defined(HAVE_SSIZE_T)
typedef long        ssize_t;
#endif

#if !defined(HAVE_SOCKET_T)
typedef int         socket_t;
#endif

#ifndef HAVE_SOCKLEN_T
#if HAVE_SIZE_T
typedef size_t      socklen_t;
#else
typedef int         socklen_t;
#endif
#endif


/* #undef HAVE_IN_ADDR_T */
#if !defined(HAVE_IN_ADDR_T)
typedef uint32_t    in_addr_t;
#endif

typedef union
{
  struct sockaddr     sa;
#if  HAVE_STRUCT_SOCKADDR_IN
  struct sockaddr_in  sin;
#endif                          /* HAVE_STRUCT_SOCKADDR_IN */
#if  ENABLE_IPV6
# if  HAVE_STRUCT_SOCKADDR_IN6
  struct sockaddr_in6 sin6;
# endif                         /* HAVE_STRUCT_SOCKADDR_IN6 */
#endif                          /* ENABLE_IPV6 */
#if  HAVE_STRUCT_SOCKADDR_UN && 0
  struct sockaddr_un  sun;
#endif                          /* HAVE_STRUCT_SOCKADDR_UN */
} JSOCKADDR_T;

#if !HAVE_FLOCK_T && HAVE_STRUCT_FLOCK
typedef struct flock flock_t;
#endif


#if !HAVE_DECL_STRLCPY
size_t              strlcpy(char *, const char *, size_t);
#endif

#if !HAVE_DECL_STRLCAT
size_t              strlcat(char *, const char *, size_t);
#endif

#if HAVE_FCHMOD && (HAVE_DECL_FCHMOD == 0)
int                 fchmod(int, mode_t);
#endif

#if HAVE_PREAD && (HAVE_DECL_PREAD == 0)
ssize_t             pread(int, void *, size_t, off_t);
#endif

#if HAVE_PWRITE && (HAVE_DECL_PWRITE == 0)
ssize_t             pwrite(int, const void *, size_t, off_t);
#endif

#ifndef 	_POSIX2_LINE_MAX
#define	_POSIX2_LINE_MAX	2048
#endif  /* 	_POSIX2_LINE_MAX **/


#if HAVE_MI_STOP
int  mi_stop (void);
#endif /* HAVE_MI_STOP */

#ifdef _POSIX_PTHREAD_SEMANTICS
# define CTIME_R(t,s)         ctime_r(t,s)
#else
# define CTIME_R(t,s)         ctime_r(t,s,sizeof(s))
#endif

#if !HAVE_STRTOULL
# if HAVE_STRTOLL
#  define strtoull strtoll
# else
#  define strtoull strtoul
# endif /* HAVE_STRTOLL */
#endif  /* HAVE_STRTOULL */


#define __J_SYS_H__

#endif
