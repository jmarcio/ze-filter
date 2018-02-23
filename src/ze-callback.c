
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Sat Dec 19 23:42:11 CET 2009
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
#include <ze-filter.h>
#include <ze-callback.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

static kstats_T     callback_st[CALLBACK_LAST + 1];
static kstats_T     callback_gst;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static bool         st_ok = FALSE;


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void
stats_init()
{
  int                 i;

  MUTEX_LOCK(&mutex);

  for (i = CALLBACK_FIRST; i <= CALLBACK_LAST; i++)
    zeKStatsReset(&callback_st[i]);
  zeKStatsReset(&callback_gst);
  st_ok = TRUE;

  MUTEX_UNLOCK(&mutex);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
callback_stats_update(callback, dt)
     int                 callback;
     timems_T            dt;
{
  if (callback < CALLBACK_FIRST || callback > CALLBACK_LAST)
    return FALSE;

  if (!st_ok)
    stats_init();

  MUTEX_LOCK(&mutex);
  zeKStatsUpdate(&(callback_st[callback]), (double) dt);
  zeKStatsUpdate(&callback_gst, (double) dt);
  MUTEX_UNLOCK(&mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
callback_stats_dump(fd, line)
     int                 fd;
     bool                line;
{
  int                 i;

  MUTEX_LOCK(&mutex);

  if (line) {
    char                buf[2048];
    time_t              now = time(NULL);
    char                str[256];

    memset(buf, 0, sizeof (buf));
    memset(str, 0, sizeof (str));

    snprintf(str, sizeof (str), "DATA TIMESTAMP=(%12lld) ", (long long) now);
    strlcat(buf, str, sizeof (buf));
    for (i = CALLBACK_FIRST; i <= CALLBACK_LAST; i++) {
      snprintf(str, sizeof (str), "%s=(%.1f) ", CALLBACK_LABEL(i),
               zeKMean(&callback_st[i]));
      strlcat(buf, str, sizeof (buf));
    }
    FD_PRINTF(fd, "%s\n", buf);
  } else {
    if (fd >= 0) {
      FD_PRINTF(fd, "   Callback Handling Times Statistics\n");
      FD_PRINTF(fd, "*  %-15s : %8s %10s %10s %10s %10s\n",
                "Callback", "Count", "Minimum", "Maximum", "Mean", "Std. Dev.");
      FD_PRINTF(fd, "*  %-15s : %8s %10s %10s %10s %10s\n",
                "********", "********", "*********", "*********",
                "*********", "*********");
    } else {
      ZE_MessageInfo(9, "   Callback Handling Times Statistics");
      ZE_MessageInfo(9, "*  %-15s : %8s %10s %10s %10s %10s",
                     "Callback", "Count", "Minimum", "Maximum", "Mean",
                     "Std. Dev.");
      ZE_MessageInfo(9, "*  %-15s : %8s %10s %10s %10s %10s", "********",
                     "********", "*********", "*********", "*********",
                     "*********");
    }
    for (i = CALLBACK_FIRST; i <= CALLBACK_LAST; i++) {
      if (fd >= 0)
        FD_PRINTF(fd, "*  %-15s : %8ld %10.1f %10.1f %10.1f %10.1f  ms\n",
                  CALLBACK_LABEL(i),
                  (long int) zeKCount(&callback_st[i]),
                  zeKMin(&callback_st[i]),
                  zeKMax(&callback_st[i]),
                  zeKMean(&callback_st[i]), zeKStdDev(&callback_st[i]));
      else
        ZE_MessageInfo(9, "*  %-15s : %8ld %10.1f %10.1f %10.1f %10.1f  ms",
                       CALLBACK_LABEL(i),
                       (long int) zeKCount(&callback_st[i]),
                       zeKMin(&callback_st[i]),
                       zeKMax(&callback_st[i]),
                       zeKMean(&callback_st[i]), zeKStdDev(&callback_st[i]));
    }
    if (fd >= 0) {
      FD_PRINTF(fd, "*  %-15s : %8s %10s %10s %10s %10s\n",
                "********", "********", "*********", "*********",
                "*********", "*********");
      FD_PRINTF(fd, "*  %-15s : %8ld %10.1f %10.1f %10.1f %10.1f  ms\n",
                "Global",
                (long int) zeKCount(&callback_gst),
                zeKMin(&callback_gst),
                zeKMax(&callback_gst),
                zeKMean(&callback_gst), zeKStdDev(&callback_gst));
    } else {
      ZE_MessageInfo(9, "*  %-15s : %8s %10s %10s %10s %10s",
                     "********", "********", "*********", "*********",
                     "*********", "*********");
      ZE_MessageInfo(9, "*  %-15s : %8ld %10.1f %10.1f %10.1f %10.1f  ms",
                     "Global",
                     (long int) zeKCount(&callback_gst),
                     zeKMin(&callback_gst),
                     zeKMax(&callback_gst),
                     zeKMean(&callback_gst), zeKStdDev(&callback_gst));
    }
  }

  MUTEX_UNLOCK(&mutex);

  return TRUE;
}
