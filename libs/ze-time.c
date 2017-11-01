/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Wed May 11 13:06:37 CEST 2005
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
#include <ze-libjc.h>
#include <ze-time.h>

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
uint64_t
time_ms()
{
#if HAVE_GETHRTIME
  return (uint64_t) (gethrtime() / 1000000);
#else
  struct timeval      tv;

  if (gettimeofday(&tv, NULL) != 0)
    return 0;
  return (uint64_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
time_t
sleep_ms(ms)
     time_t              ms;
{
  struct timeval      tv;

  tv.tv_sec = 0;
  tv.tv_usec = ms * 1000;
  select(0, NULL, NULL, NULL, &tv);

  return 0;
}
