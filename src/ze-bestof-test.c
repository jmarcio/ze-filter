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
#include <ze-filter.h>
#include <ze-bestof-n.h>

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          ze_logLevel;

  bool                r;

  char                buf[64];

  bestof_T            best;

  configure("ze-module-test", conf_file, FALSE);

  zeLog_SetOutput(FALSE, TRUE);
  ze_logLevel = 10;

  printf("Hello, world\n");

  bestof_init(&best, 5, NULL);

  bestof_add(&best, -5.);
  bestof_add(&best, -4.);
  bestof_add(&best, 0.);
  bestof_add(&best, 4.);
  bestof_add(&best, 5.);

  printf("AVERAGE : %7.3f\n", bestof_average(&best));

  return 0;
}
