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

static int          add_scores(char *);

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          ze_logLevel;

  bool                r;

  configure("ze-module-test", conf_file, FALSE);

  zeLog_SetOutput(FALSE, TRUE);
  ze_logLevel = 10;

  printf("Hello, world\n");

  load_all_modules("/etc/ze-filter", "ze-modules", "/usr/lib/ze-filter");

  module_info();

  {
    mod_ctx_T           arg;

    memset(&arg, 0, sizeof (arg));

    arg.id = "NOID";
    arg.claddr = "10.3.5.5";
    arg.clname = "minho.ensmp.fr";

    arg.callback = CALLBACK_CONNECT;
    module_call(CALLBACK_CONNECT, rand() % 9, &arg);

    arg.callback = CALLBACK_EOM;
    module_call(CALLBACK_EOM, rand() % 9, &arg);
  }

  return 0;
}
