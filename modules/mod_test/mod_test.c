/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2007 - Ecole des Mines de Paris
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 Jose-Marcio.Martins@ensmp.fr
 *
 *  Historique   :
 *  Creation     : Sat Jun  2 22:11:39 CEST 2007
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

#include <ze-sys.h>
#include <ze-filter.h>
#include <mod_test.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static mod_info_T   info = {
  "mod_test",
  "Jose-Marcio Martins da Cruz",
  "0.1.0 Alpha"
};

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_init(version, callbacks, args)
     int                 version;
     uint32_t           *callbacks;
     char               *args;
{
  /* check version */
  if (version != MOD_VERSION)
    printf("API Version mismatch : %08X %08X\n", version, MOD_VERSION);
  printf("NAME    : %s\n", info.name);
  printf("AUTHOR  : %s\n", info.author);
  printf("VERSION : %s\n", info.version);
  printf("VERSION : %08X\n", version);
  printf("ARGS    : %s\n", STRNULL(args,"null"));

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_info(infop)
     mod_info_T         *infop;
{
  *infop = info;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_call(callback, step, arg)
     int                callback;
     int                step;
     mod_args_T         *arg;
{

  if (arg == NULL)
    goto fin;

  printf("     Checking  : %-10s %-16s %s\n", arg->id, arg->addr, arg->name);

  switch (callback)
  {
    default:
      printf("     Callback  : %-12s %2d\n", CALLBACK_LABEL(callback), step);
      break;
  }

fin:
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_close()
{
  printf("  * Closing module %s\n", info.name);

  return TRUE;
}

