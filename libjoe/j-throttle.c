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

#include "ze-chkmail.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define LOG_LEVEL       11

typedef struct HStat_T
{
  time_t              date;
  int                 nb;
}
HStat_T;



/*
  DIM_HI    number of seconds of connection counting
  DIM_SHFT  log2(DIM_RAP)
  DIM_LO    number of normalised minutes of counting
  DIM_RAP   number of seconds in a normalised minute
*/
#define DIM_HI            1024
#define DIM_SHFT             6
#define DIM_LO            (DIM_HI >> DIM_SHFT)
#define DIM_RAP           (DIM_HI / DIM_LO)


struct ThrottleData
{
  bool                ok;

  pthread_mutex_t     mutex;

  HStat_T             sec[DIM_HI];
  HStat_T             min[DIM_LO];
};

typedef struct ThrottleData ThrottleData;

static ThrottleData hdata = { FALSE, PTHREAD_MUTEX_INITIALIZER };

#define THROTTLE_LOCK() \
  if (pthread_mutex_lock(&hdata.mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_lock"); \
  }

#define THROTTLE_UNLOCK() \
  if (pthread_mutex_unlock(&hdata.mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_unlock"); \
  }



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void                add_throttle_entry(time_t);
void                update_throttle(time_t);

double              poisson_upper_bound(double, double);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
throttle_init()
{
  if (hdata.ok)
    return TRUE;

  THROTTLE_LOCK();

  if (!hdata.ok)
  {
    /* Initialisation of connection rate statistics */
    memset(hdata.sec, 0, sizeof (hdata.sec));
    memset(hdata.min, 0, sizeof (hdata.min));
    hdata.ok = TRUE;
  }

  THROTTLE_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
throttle_free()
{
  THROTTLE_LOCK();


  THROTTLE_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
throttle_resize(sza, szb)
     size_t              sza;
     size_t              szb;
{
  throttle_free();
  return throttle_init();
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

void
add_throttle_entry(t)
     time_t              t;
{
  int                 i;

  throttle_init();

  THROTTLE_LOCK();

  i = t % DIM_HI;

  if (hdata.sec[i].date != t)
  {
    hdata.sec[i].date = t;
    hdata.sec[i].nb = 0;
  }
  hdata.sec[i].nb++;

  t >>= DIM_SHFT;
  i = t % DIM_LO;

  if (hdata.min[i].date != t)
  {
    hdata.min[i].date = t;
    hdata.min[i].nb = 0;
  }
  hdata.min[i].nb++;

  THROTTLE_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static kstats_T     st_sec;
static kstats_T     st_min;
static time_t       st_last = 0;

void
update_throttle(t)
     time_t              t;
{
  int                 i, j;
  time_t              t0, ti;

  if (t < DIM_HI)
    return;

  THROTTLE_LOCK();

  st_last = t;

  memset(hdata.min, 0, sizeof (hdata.min));

  t0 = t + 1 - DIM_HI;

  for (ti = t0; ti < t + 1; ti++)
  {
    i = ti % DIM_HI;
    if (hdata.sec[i].date != ti)
    {
      hdata.sec[i].date = ti;
      hdata.sec[i].nb = 0;
    }
    j = ((ti - t0) >> DIM_SHFT);

    hdata.min[j].nb += hdata.sec[i].nb;
  }
  kstats_reset(&st_sec);
  kstats_reset(&st_min);

  for (i = 0; i < DIM_HI; i++)
    kstats_update(&st_sec, (double) hdata.sec[i].nb);

  for (i = 0; i < DIM_LO; i++)
    kstats_update(&st_min, (double) hdata.min[i].nb);

  MESSAGE_INFO(12,
               "THROTTLE : short=[%7.3f/%7.3f] long=[%7.3f/%7.3f] (mean/std dev)",
               kmean(&st_sec), kstddev(&st_sec), kmean(&st_min),
               kstddev(&st_min));

  THROTTLE_UNLOCK();

  update_throttle_dos();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static bool         dos_current = FALSE;

bool
check_throttle_dos()
{
  return dos_current;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define        HOLD_TIME       120

static time_t       dos_start_time = 0;
static time_t       dos_last_log = 0;

#define  DOS_COEF      5.0

bool
update_throttle_dos()
{
  double              mean, gmean;
  double              stddev;
  double              last_mean = (double) hdata.min[DIM_LO - 1].nb;
  int                 i;
  kstats_T            stats;

  kstats_reset(&stats);

  THROTTLE_LOCK();
  for (i = 0; i < (DIM_LO - 1); i++)
    kstats_update(&stats, (double) hdata.min[i].nb);
  last_mean = (double) hdata.min[DIM_LO - 1].nb;
  THROTTLE_UNLOCK();

  gmean = mean = kmean(&stats);
  stddev = kstddev(&stats);

  /* shall see this again later */
  dos_current = FALSE;

  if (gmean < 10)
    gmean = 10;

  if (last_mean > gmean + DOS_COEF * sqrt(gmean))
  {
    time_t              now = time(NULL);

    if (!dos_current)
      dos_start_time = now;

    if ((dos_last_log == (time_t) 0) && (dos_last_log + HOLD_TIME < now))
    {
      MESSAGE_INFO(LOG_LEVEL, "*** DoS - THROTTLE : %7.3f %7.3f %7.3f",
                    mean, stddev, last_mean);
      dos_last_log = now;
    }
    dos_current = TRUE;
    return TRUE;
  } else
  {
    if (dos_last_log != (time_t) 0)
    {
      MESSAGE_INFO(LOG_LEVEL, "*** DoS - THROTTLE : END");
      dos_last_log = (time_t) 0;
    }
    dos_start_time = (time_t) 0;
  }

  return FALSE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
log_throttle_stats()
{
  double              mean = kmean(&st_min);
  double              stddev = kstddev(&st_min);

  double              last_mean = (double) hdata.min[DIM_LO - 1].nb;

  if (cf_get_int(CF_LOG_THROTTLE) == OPT_YES)
    MESSAGE_INFO(9, "THROTTLE STAT : %7.3f %7.3f %7.3f", mean, stddev,
                 last_mean);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
poisson_upper_bound(lambda, prob)
     double              lambda;
     double              prob;
{
  double              sum, tmp;
  int                 i = 0;

  sum = tmp = exp(-lambda);

  while (sum < prob)
  {
    i++;
    tmp *= lambda / i;
    sum += tmp;
  }
  printf(" i : %3d - sum : %7.5f\n", i, sum);
  return i;
}
