/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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


#ifndef __ZE_MACROS_H

#define ASSERT(a)		assert((a))

#if !OS_LINUX
# define WAIT_NOHANG(pid,stat)    waitpid(pid, stat, WNOHANG)
# define WAIT_HANG(pid,stat)      waitpid(pid, stat, 0)
#else
# define WAIT_NOHANG(pid,stat)    waitpid(pid, stat, WNOHANG | __WALL)
# define WAIT_HANG(pid,stat)      waitpid(pid, stat, __WALL)
#endif

#define FREE(x) \
  do { \
    if (x != NULL) \
      free(x); \
    x = NULL; \
  } while (0)


#define  FD_PRINTF(fdp, ...)					    \
  do {								    \
    if (fdp >= 0) {						    \
      char fdp_str[1024];					    \
      (void ) snprintf(fdp_str, sizeof(fdp_str), __VA_ARGS__);	    \
      if (write (fdp, fdp_str, strlen(fdp_str)) != strlen(fdp_str)) \
        ZE_LogSysError("error on FD_PRINTF");			    \
    } else							    \
      printf(__VA_ARGS__);					    \
  } while (0)

#define  SD_PRINTF(fdp, ...)					    \
  do {								    \
    if (fdp >= 0) {						    \
      char fdp_str[1024];					    \
      (void ) snprintf(fdp_str, sizeof(fdp_str), __VA_ARGS__);	    \
      if (sendto(fdp, fdp_str, strlen(fdp_str), 0, NULL, 0) != strlen(fdp_str)) \
        ZE_LogSysError("error on SD_PRINTF");			    \
    } else							    \
      printf(__VA_ARGS__);					    \
  } while (0)

#if 0
#define STRLCPY(d,s,sz)       snprintf(d,sz,"%s",s)
#endif

#if 1
#define STRCASEEQUAL(a,b)						\
  ((a) != NULL && (b) != NULL ? strcasecmp((a),(b)) == 0 : ((a) == (b)))

#define STRNCASEEQUAL(a,b,n)						\
  ((a) != NULL && (b) != NULL ? strncasecmp((a),(b),(n)) == 0 : ((a) == (b)))

#define STREQUAL(a,b)							\
  ((a) != NULL && (b) != NULL ? strcmp((a),(b)) == 0 : ((a) == (b)))

#define STRNULL(x,r)             ((x) != NULL ? (x) : (r))
#define STREMPTY(x,r)            ((x) != NULL && strlen(x) > 0 ? (x) : (r))

#define ISSTRNULL(x)             ((x) == NULL)
#define ISSTREMPTY(x)            ((x) == NULL || strlen(x) == 0)

#define STRBOOL(x,t,f)           ((x) ? t : f)

#endif

#define SIGN(x)                 ((x) < 0 ? - 1 : 1)

#define MUTEX_LOCK(mutex)                                    \
  {                                                          \
    int r = 0;                                               \
    if ((r = pthread_mutex_lock(mutex)) != 0) {              \
      ZE_LogSysError("pthread_mutex_lock : %s", strerror(r)); \
    }                                                        \
  }

#define MUTEX_UNLOCK(mutex)                                    \
  {                                                            \
    int r = 0;                                                 \
    if ((r = pthread_mutex_unlock(mutex)) != 0) {              \
      ZE_LogSysError("pthread_mutex_unlock : %s", strerror(r)); \
    }                                                          \
  }

#define RWLOCK_RDLOCK(lock) \
  if (pthread_rwlock_rdlock(lock) != 0) { \
    ZE_LogSysError("pthread_rwlock_rdlock"); \
  }

#define RWLOCK_WRLOCK(lock) \
  if (pthread_rwlock_wrlock(lock) != 0) { \
    ZE_LogSysError("pthread_rwlock_rdlock"); \
  }

#define RWLOCK_UNLOCK(lock) \
  if (pthread_rwlock_unlock(lock) != 0) { \
    ZE_LogSysError("pthread_rwlock_rdlock"); \
  }



#ifndef HAVE_MAX
#define max(a,b)                 ((a) > (b) ? (a) : (b))
#define min(a,b)                 ((a) < (b) ? (a) : (b))
#endif

#ifdef   MAX
# undef  MAX
#endif

#ifdef   MIN
# undef  MIN
#endif

#define MAX(a,b)              ((a) > (b) ? (a) : (b))
#define MIN(a,b)              ((a) < (b) ? (a) : (b))

#define SECONDS
#define MINUTES                  * 60
#define HOURS                    * 60 MINUTES
#define DAYS                     * 24 HOURS
#define WEEKS                    * 7 DAYS
#define MONTHS                   * 30 DAYS
#define YEARS                    * 365 DAYS

#define BYTES
#define KBYTES                   * 1024
#define MBYTES                   * 1024 KBYTES
#define GBYTES                   * 1024 MBYTES


#ifndef TRUE
# define TRUE   1
#endif             /* ! TRUE */
#ifndef FALSE
# define FALSE  0
#endif             /* ! FALSE */




#define SET_BIT(p, i)           ((p) |= (1 << (i)))
#define CLR_BIT(p, i)           ((p) &= ~(1 << (i)))
#define GET_BIT(p, i)           (((p) & (1 << (i))) != 0 ? TRUE : FALSE)


#define SKIP_SPACES(s)				\
  do {						\
    if (s != NULL)				\
      while (*s != '\0' && isspace(*s))		\
	s++;					\
  } while (0)

#define SKIP_ALPHAS(s)				\
  do {						\
    if (s != NULL)				\
      while (*s != '\0' && isalpha(*s))		\
	s++;					\
  } while (0)

#define SKIP_DIGITS(s)				\
  do {						\
    if (s != NULL)				\
      while (*s != '\0' && isdigit(*s))		\
	s++;					\
  } while (0)

#define SKIP_ALPHANUM(s)			\
  do {						\
    if (s != NULL)				\
      while (*s != '\0' && isalnum(*s))		\
	s++;					\
  } while (0)

#define SKIP_KEYCHARS(s)				\
  do {							\
    if (s != NULL)					\
      while (*s != '\0' && (isalnum(*s) || *s == '-'))	\
	s++;						\
  } while (0)

#define STRIP_END_SPACES(s)				\
  do {						\
    if (s != NULL)				\
      while (*s != '\0' && isspace(*s))		\
	s++;					\
  } while (0)


#define PATH_REGEX               "^(/[-a-z0-9.]+)+$"

#define DOMAINNAME_REGEX         "^[a-z0-9._-]+\\.[a-z]{2,6}$"

#define IPV4_ADDR_REGEX          "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$"
#define IPV6_ADDR_REGEX          "(ipv6:)?.*:"

#define IPV4_ADDR_REGEX_BRACKET  "[\\[][0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+[\\]]"


#define CRLF                     "\r\n"

#if defined(NULLSENDER)
# undef NULLSENDER
#endif
#define NULLSENDER "<>"

#define POSTMASTER_OK        1
#define POSTMASTER_FORGED   -1
#define NOT_POSTMASTER       0


#define ADJUST_FILENAME(path, fname, cfdir, defval)		\
  do {									\
    char *name = NULL;							\
    name = (fname != NULL && strlen(fname) > 0) ? fname : defval;	\
    strcpy(path, "");							\
    if (*name == '/' || *name == '.')					\
      strlcpy(path, name, sizeof(path));				\
    else								\
      snprintf(path, sizeof(path), "%s/%s", cfdir, name);		\
    zeLog_MessageInfo(15, "Adjusted path is %s", path);			\
  } while (FALSE);


#define SHOW_CURSOR(zero)		       	\
  do {					       	\
    static char xCURSOR[] = "|/-\\";	       	\
    static int n = 0, i = 0;		       	\
    if (zero)				       	\
      n = 0;				       	\
    if (++n % 10 == 0) {		       	\
      char c = xCURSOR[i++ % 4];			\
      fprintf (stderr, "* %c %6d\r", c, n);	\
    }					       	\
  } while (FALSE)

#define __ZE_MACROS_H
#endif
