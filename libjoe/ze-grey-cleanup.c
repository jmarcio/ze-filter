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
 *  Creation     : Tue Apr 12 14:14:49 CEST 2005
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
#include <ze-grey-cleanup.h>

static double       threshold = -1;
static bool         threshok = FALSE;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
set_grey_dewhitelist_threshold(val)
     double              val;
{
  threshold = val;
  threshok = TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
set_threshold_from_env()
{
  char               *env = NULL;

  if (threshok)
    return;

  env = getenv("GREY_DEWHITELIST_THRESHOLD");
  if (env != NULL)
  {
    double              t = str2double(env, NULL, 0);

    if (errno != EINVAL && errno != ERANGE)
      threshold = t;
  }
  threshok = TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_check_bad_smtp_client(ip, flags)
     char               *ip;
     uint32_t            flags;
{
  int                 conn, msgs, score;
  int                 spamtraps, badrcpt, badmx;

  double              res = 0;

  if (!threshok)
    set_threshold_from_env();

  if (flags == GREY_DW_NONE)
    return FALSE;

  if (threshold <= 0)
    return FALSE;

  conn = smtprate_check(RATE_CONN, ip, 10 MINUTES);

#if 0
  {
    int                 xfiles;

    xfiles = smtprate_check(RATE_XFILES, ip, 10 MINUTES);
    res += xfiles / 3.;
  }
#endif

  if ((flags & GREY_DW_SPAMTRAP) != GREY_DW_NONE)
  {
    spamtraps = livehistory_check_host(ip, 4 HOURS, LH_SPAMTRAP);
    res += spamtraps / 5.;
  }

  if ((flags & GREY_DW_BAD_RCPT) != GREY_DW_NONE)
  {
    badrcpt = livehistory_check_host(ip, 4 HOURS, LH_BADRCPT);
    res += badrcpt / 3.;
  }

  if ((flags & GREY_DW_BAD_MX) != GREY_DW_NONE)
  {
    badmx = livehistory_check_host(ip, 4 HOURS, LH_BADMX);
    res += badmx / 3.;
  }

  if ((flags & GREY_DW_BAD_CLIENT) != GREY_DW_NONE)
  {
    msgs = smtprate_check(RATE_MSGS, ip, 10 MINUTES);
    score = smtprate_check(RATE_SCORE, ip, 10 MINUTES);
    if (msgs > 1)
      res += (1. * score) / msgs;
  }

  if (threshold > 0.)
    return res > threshold;

  return FALSE;
}
