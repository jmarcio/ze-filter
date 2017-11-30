
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

#include <ze-sys.h>
#include <zeLibs.h>
#include "ze-filter.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define    FAST_FD_COUNT        1
#define    CHECK_FD_WITH_POLL   0

#define    DT_CHECK            10

#define    ZE_FD_MAX            -1
#define    ZE_FD_DFL             0

#define    ZE_NOFILE           256

static long         fd_last = 0;
static long         fd_open = 0;
static long         fd_idle = 0;

static rlim_t       fd_max = 0;

static rlim_t       fd_soft = 0;
static rlim_t       fd_hard = 0;

static long         fd_conf = ZE_FD_DFL;
static rlim_t       fd_cur = 0;

static pthread_mutex_t st_mutex = PTHREAD_MUTEX_INITIALIZER;

#if HAVE_POLL
#define POLL_IN_USE  (POLLIN | POLLOUT | POLLPRI | POLLHUP)
#endif

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if HAVE_FDWALK
static int
fdwalk_count(void *count, int fd)
{
  int                *n = (int *) count;

  (*n)++;
  return 0;
}
#endif             /* HAVE_FDWALK */

int
count_file_descriptors()
{
  int                 i = 0;
  struct rlimit       rlp;

  time_t              now = time(NULL);

  long                lsoft, lhard, nb_idle, nb_used;

  lsoft = cf_get_int(CF_FD_FREE_SOFT_LIMIT);
  lhard = cf_get_int(CF_FD_FREE_HARD_LIMIT);

#if HAVE_SETRLIMIT
  if (fd_max == 0) {
    if (getrlimit(RLIMIT_NOFILE, &rlp) == 0) {
      fd_max = rlp.rlim_cur;
    } else {
      fd_max = sysconf(_SC_OPEN_MAX);
    }
  }
#endif

#if 0 && HAVE_FDWALK
  nb_used = nb_idle = 0;

  /*
   * call fdwalk... 
   */
  nb_used = 0;

  (void) fdwalk(fdwalk_count, &nb_used);

  /*
   * fd_last ???????? - this is wrong 
   */
  nb_idle = fd_last - nb_used;

#else              /* HAVE_FDWALK */

  if (fd_last + DT_CHECK > now)
    return fd_open;

  nb_used = nb_idle = 0;
  for (i = fd_max - 1; i >= 0; i--) {
#if CHECK_FD_WITH_POLL == 1
    struct pollfd       ufds;
    int                 r;
#else
    struct stat         buf;
#endif

#if FAST_FD_COUNT
    if (nb_idle > lsoft)
      break;
#endif

#if CHECK_FD_WITH_POLL == 1
    ufds.fd = i;
    ufds.events = POLLIN | POLLOUT;
    ufds.revents = 0;

    r = poll(&ufds, 1, 0);

    if ((r < 0) && (errno == EBADF)) {
      nb_idle++;
      continue;
    }

    if (r < 0) {
      continue;
    }

    if ((r > 0) && ((ufds.revents & POLLNVAL) != 0)) {
      nb_idle++;
      continue;
    }

    if (r >= 0) {
      nb_used++;
      continue;
    }
#else
    if (fstat(i, &buf) == 0)
      nb_used++;
    else
      nb_idle++;
#endif
  }
#endif             /* HAVE_FDWALK */

  MUTEX_LOCK(&st_mutex);
  fd_last = now;
  fd_open = nb_used;
  fd_idle = nb_idle;
  MUTEX_UNLOCK(&st_mutex);

#if 0
  ZE_MessageInfo(10, " FD : open=[%d] idle=[%d] now=[%ld]", fd_open, fd_idle,
                 now % 17);
#endif

  return nb_used;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

int
check_file_descriptors()
{
  long                lsoft, lhard;
  long                nb_idle, nb_open;

  lsoft = cf_get_int(CF_FD_FREE_SOFT_LIMIT);
  lhard = cf_get_int(CF_FD_FREE_HARD_LIMIT);

  count_file_descriptors();

  MUTEX_LOCK(&st_mutex);
  nb_open = fd_open;
  nb_idle = fd_idle;
  MUTEX_UNLOCK(&st_mutex);

  if (fd_max == 0)
    return FD_LEVEL_OK;

#if 1
  if (nb_idle < lhard) {
    ZE_LogMsgWarning(0, "file descriptors being short... (hard limit) %ld/%ld",
                     (long) fd_open, (long) fd_max);
    return FD_LEVEL_HI;
  }
  if (nb_idle < lsoft) {
    ZE_LogMsgWarning(0, "file descriptors being short... (soft limit) %ld/%ld",
                     (long) fd_open, (long) fd_max);
    return FD_LEVEL_SHORT;
  }
#else
  if (nb_open + lhard > fd_max) {
    ZE_LogMsgWarning(0, "file descriptors being short... (hard limit) %ld/%ld",
                     (long) fd_open, (long) fd_max);
    return FD_LEVEL_HI;
  }

  if (nb_open + lsoft > fd_max) {
    ZE_LogMsgWarning(0, "file descriptors being short... (soft limit) %ld/%ld",
                     (long) fd_open, (long) fd_max);
    return FD_LEVEL_SHORT;
  }
#endif

  return FD_LEVEL_OK;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
setup_file_descriptors()
{
#if HAVE_SETRLIMIT
  struct rlimit       rlp;
  char               *strlim = cf_get_str(CF_FILE_DESCRIPTORS);
  long                lsoft, lhard;

  lsoft = cf_get_int(CF_FD_FREE_SOFT_LIMIT);
  lhard = cf_get_int(CF_FD_FREE_HARD_LIMIT);

  if (strlim != NULL)
    ZE_MessageInfo(11, "CF_FILE_DESCRIPTORS = %s", strlim);

  if (getrlimit(RLIMIT_NOFILE, &rlp) == 0) {
    if (fd_soft == 0)
      fd_soft = rlp.rlim_cur;
    if (fd_hard == 0)
      fd_hard = rlp.rlim_max;
  } else {
    ZE_LogSysError("getrlimit(RLIMIT_NOFILES) error");
    return 0;
  }

  if (strlim != NULL) {
    if (strcasecmp(strlim, "DEFAULT") == 0)
      fd_conf = ZE_FD_DFL;
    else {
      if (strcasecmp(strlim, "MAX") == 0) {
#if 0
        fd_conf = ZE_FD_MAX;
#else
        fd_conf = fd_hard;
#endif
      } else {
        long                n;

        n = zeStr2long(strlim, NULL, ZE_FD_DFL);
        if (errno == ERANGE || errno == EINVAL || n <= 0 || n > fd_hard)
          n = ZE_FD_DFL;
        fd_conf = n;
      }
    }
  }

  if (fd_conf > fd_hard) {
    ZE_MessageInfo(0, "Can't set file descriptor limit greater than %ld",
                   (long) fd_hard);
    fd_conf = fd_hard;
  }

  if ((fd_conf > 1) && (fd_conf < ZE_NOFILE)) {
    fd_conf = ZE_NOFILE;
  }

  switch (fd_conf) {
    case ZE_FD_DFL:
      fd_cur = fd_soft;
      break;
    case ZE_FD_MAX:
      fd_cur = fd_hard;
      break;
    default:
      fd_cur = (fd_conf >= ZE_NOFILE ? fd_conf : ZE_NOFILE);
      break;
  }

  ZE_MessageInfo(11, "Will set file descriptor limit to %ld [%ld,%ld]",
                 (long) fd_cur, (long) fd_soft, (long) fd_hard);

  if ((cf_get_int(CF_USE_SELECT_LIMIT) == OPT_YES) && (fd_cur > FD_SETSIZE)) {
    ZE_MessageInfo(11,
                   "May not set file descriptor limit to more than FD_SETSIZE");
    fd_cur = FD_SETSIZE;
  }

  rlp.rlim_cur = fd_cur;
  if (setrlimit(RLIMIT_NOFILE, &rlp) != 0) {
    ZE_LogSysError("setrlimit(RLIMIT_NOFILE) error");
  } else
    ZE_MessageInfo(11, "RLIMIT_NOFILE set to %ld files", (long) rlp.rlim_cur);

  ZE_MessageInfo(9,
                 "File descriptors thresholds set to : soft=[%ld] - hard=[%ld]",
                 (long) (fd_cur - lsoft), (long) (fd_cur - lhard));
#endif

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
enable_coredump(enable)
     bool                enable;
{
#if HAVE_SETRLIMIT
#if 0
  return TRUE;
#else
  struct rlimit       rlp;

  if (getrlimit(RLIMIT_CORE, &rlp) == 0) {
    rlp.rlim_cur = (enable ? 32000000 : 0);

    if (setrlimit(RLIMIT_CORE, &rlp) != 0) {
      ZE_LogSysError("setrlimit(RLIMIT_CORE) error");
      return FALSE;
    } else
      ZE_MessageInfo(9, "RLIMIT_CORE set to %ld bytes", (long) rlp.rlim_cur);
  } else {
    ZE_LogSysError("getrlimit(RLIMIT_CORE) error");
    return FALSE;
  }

  return TRUE;
#endif
#else
  return TRUE;
#endif
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
check_rusage()
{
#if 0
  struct rusage       usage;
  int                 r;

  ZE_MessageInfo(9, "Checking rusage...");
  memset(&usage, 0, sizeof (usage));
  r = getrusage(RUSAGE_SELF, &usage);
  if (r == 0) {
    long                ru_maxrss;  /* maximum resident set size */
    long                ru_ixrss; /* integral shared memory size */
    long                ru_idrss; /* integral unshared data size */
    long                ru_isrss; /* integral unshared stack size */

    ru_maxrss = usage.ru_maxrss;
    ru_ixrss = usage.ru_ixrss;
    ru_idrss = usage.ru_idrss;
    ru_isrss = usage.ru_isrss;

    ZE_MessageInfo(9, "* RESOURCE CHECK : ru_maxrss = %10ld", ru_maxrss);
    ZE_MessageInfo(9, "* RESOURCE CHECK : ru_ixrss  = %10ld", ru_ixrss);
    ZE_MessageInfo(9, "* RESOURCE CHECK : ru_idrss  = %10ld", ru_idrss);
    ZE_MessageInfo(9, "* RESOURCE CHECK : ru_isrss  = %10ld", ru_isrss);
  }
#endif

  return TRUE;
}
