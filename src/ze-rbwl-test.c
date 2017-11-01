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

  configure("j-rbwl-test", conf_file, FALSE);

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

    if (log_level > 11)
    {
#if 0
#define HOSTNAME_REGEX         "^[a-z0-9.-]+\\.[a-z]{2,6}$"
#else
#define HOSTNAME_REGEX         DOMAINNAME_REGEX
#endif

      MESSAGE_INFO(10, "* HOSTNAME_REGEX %s", HOSTNAME_REGEX);
      if (strexpr(name, HOSTNAME_REGEX, NULL, NULL, TRUE))
        MESSAGE_INFO(10, "* %s is a domain name", name);
      else
        MESSAGE_INFO(10, "* %s isn't a domain name", name);
    }

    netclass = GetClientNetClass(ip, name, NULL, NULL, 0);
    strlcpy(class, NET_CLASS_LABEL(netclass), sizeof (class));

    memset(&rbwl, 0, sizeof (rbwl));
    flag = check_iprbwl_table("00000000.000", ip, name, &rbwl);
    if (flag > 0)
      strlcpy(class, rbwl.netclass, sizeof (class));

    MESSAGE_INFO(0, "Client IP address : %s", ip);
    MESSAGE_INFO(0, "Client hostname   : %s", name);
    MESSAGE_INFO(0, "Client NET class  : %02X %s", netclass, class);

    result = STRNULL(result, "...");

    MESSAGE_INFO(0, "RESULT            = %2d %s", flag, result);
  }

  return 0;
}
