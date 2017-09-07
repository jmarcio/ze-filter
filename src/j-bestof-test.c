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

#include <j-sys.h>
#include <j-chkmail.h>
#include <j-bestof-n.h>

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          log_level;

  bool                r;

  char                buf[64];

  bestof_T            best;

  configure("j-module-test", conf_file, FALSE);

  set_log_output(FALSE, TRUE);
  log_level = 10;

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
