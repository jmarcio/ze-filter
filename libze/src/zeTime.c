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
 *  Creation     : Wed May 11 13:06:37 CEST 2005
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
#include <libze.h>
#include <zeTime.h>

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
uint64_t
zeTime_ms()
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
zeSleep_ms(ms)
     time_t              ms;
{
  struct timeval      tv;

  tv.tv_sec = 0;
  tv.tv_usec = ms * 1000;
  select(0, NULL, NULL, NULL, &tv);

  return 0;
}
