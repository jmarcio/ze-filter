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

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          log_level;

  configure("j-inet-test", conf_file, FALSE);

  set_log_output(FALSE, TRUE);
  log_level = 10;

  {
    char               *ip, *name;
    int                 netclass;
    char                class[64];

    iprbwl_T            rbwl;

    int                 flag = 0;
    char               *result = NULL;
    int                 c;

    const char         *args = "i:h:t:l:v";

    ip = "0.0.0.0";
    name = "localhost";

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
        case 'i':
          ip = optarg;
          break;
        case 'h':
          name = optarg;
          break;
        case 'v':
          log_level++;
          break;
        case 'l':
          log_level = atoi(optarg);
          break;
        default:
          (void) fprintf(stderr, "-> Unknown command line option : %c\n", c);
          exit(1);
      }
    }
  }

  {
    int                 i;

    argc--;
    argv++;

    for (i = 0; i < argc; i++)
    {
      char                buf[256];

      memset(buf, 0, sizeof (buf));
      if (get_hostbyaddr(argv[i], buf, sizeof (buf)))
      {
        MESSAGE_INFO(10, " %3d TONAME : %-30s %s", i, argv[i], buf);
      }
      memset(buf, 0, sizeof (buf));
      if (get_hostbyname(argv[i], buf, sizeof (buf)))
      {
        MESSAGE_INFO(10, " %3d TOADDR : %-30s %s", i, argv[i], buf);
      }
    }
  }

  return 0;
}
