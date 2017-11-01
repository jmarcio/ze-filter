/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Sat Dec 19 23:42:11 CET 2009
 *
 * This program is free software, but with restricted license :
 *
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#include <j-sys.h>
#include <ze-filter.h>
#include <j-callback.h>

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

static kstats_T     callback_st[CALLBACK_LAST + 1];
static kstats_T     callback_gst;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static bool         st_ok = FALSE;


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void
stats_init()
{
  int                 i;

  MUTEX_LOCK(&mutex);

  for (i = CALLBACK_FIRST; i <= CALLBACK_LAST; i++)
    kstats_reset(&callback_st[i]);
  kstats_reset(&callback_gst);
  st_ok = TRUE;

  MUTEX_UNLOCK(&mutex);
}


/******************************************************************************
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
  kstats_update(&(callback_st[callback]), (double) dt);
  kstats_update(&callback_gst, (double) dt);
  MUTEX_UNLOCK(&mutex);

  return TRUE;
}

/******************************************************************************
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

  if (line)
  {
    char                buf[2048];
    time_t              now = time(NULL);
    char                str[256];

    memset(buf, 0, sizeof (buf));
    memset(str, 0, sizeof (str));

    snprintf(str, sizeof (str), "DATA TIMESTAMP=(%12lld) ", (long long) now);
    strlcat(buf, str, sizeof (buf));
    for (i = CALLBACK_FIRST; i <= CALLBACK_LAST; i++)
    {
      snprintf(str, sizeof(str), "%s=(%.1f) ", CALLBACK_LABEL(i), kmean(&callback_st[i]));
      strlcat(buf, str, sizeof (buf));
    }
    FD_PRINTF(fd, "%s\n", buf);
  } else
  {
    if (fd >= 0)
    {
      FD_PRINTF(fd, "   Callback Handling Times Statistics\n");
      FD_PRINTF(fd, "*  %-15s : %8s %10s %10s %10s %10s\n",
                "Callback", "Count", "Minimum", "Maximum", "Mean", "Std. Dev.");
      FD_PRINTF(fd, "*  %-15s : %8s %10s %10s %10s %10s\n",
                "********", "********", "*********", "*********",
                "*********", "*********");
    } else
    {
      MESSAGE_INFO(9, "   Callback Handling Times Statistics");
      MESSAGE_INFO(9, "*  %-15s : %8s %10s %10s %10s %10s",
                   "Callback", "Count", "Minimum", "Maximum", "Mean",
                   "Std. Dev.");
      MESSAGE_INFO(9, "*  %-15s : %8s %10s %10s %10s %10s", "********",
                   "********", "*********", "*********", "*********",
                   "*********");
    }
    for (i = CALLBACK_FIRST; i <= CALLBACK_LAST; i++)
    {
      if (fd >= 0)
        FD_PRINTF(fd, "*  %-15s : %8d %10.1f %10.1f %10.1f %10.1f  ms\n",
                  CALLBACK_LABEL(i),
                  kcount(&callback_st[i]),
                  kmin(&callback_st[i]),
                  kmax(&callback_st[i]),
                  kmean(&callback_st[i]), kstddev(&callback_st[i]));
      else
        MESSAGE_INFO(9, "*  %-15s : %8d %10.1f %10.1f %10.1f %10.1f  ms",
                     CALLBACK_LABEL(i),
                     kcount(&callback_st[i]),
                     kmin(&callback_st[i]),
                     kmax(&callback_st[i]),
                     kmean(&callback_st[i]), kstddev(&callback_st[i]));
    }
    if (fd >= 0)
    {
      FD_PRINTF(fd, "*  %-15s : %8s %10s %10s %10s %10s\n",
                "********", "********", "*********", "*********",
                "*********", "*********");
      FD_PRINTF(fd, "*  %-15s : %8d %10.1f %10.1f %10.1f %10.1f  ms\n",
                "Global",
                kcount(&callback_gst),
                kmin(&callback_gst),
                kmax(&callback_gst),
                kmean(&callback_gst), kstddev(&callback_gst));
    } else
    {
      MESSAGE_INFO(9, "*  %-15s : %8s %10s %10s %10s %10s",
                   "********", "********", "*********", "*********",
                   "*********", "*********");
      MESSAGE_INFO(9, "*  %-15s : %8d %10.1f %10.1f %10.1f %10.1f  ms",
                   "Global",
                   kcount(&callback_gst),
                   kmin(&callback_gst),
                   kmax(&callback_gst),
                   kmean(&callback_gst), kstddev(&callback_gst));
    }
  }

  MUTEX_UNLOCK(&mutex);

  return TRUE;
}
