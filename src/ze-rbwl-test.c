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

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          ze_logLevel;

  configure("ze-rbwl-test", conf_file, FALSE);

  zeLog_SetOutput(FALSE, TRUE);
  ze_logLevel = 10;

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
          ze_logLevel++;
          break;
        case 'l':
          ze_logLevel = atoi(optarg);
          break;
        default:
          (void) fprintf(stderr, "-> Unknown command line option : %c\n", c);
          exit(1);
      }
    }

    if (ze_logLevel > 11)
    {
#if 0
#define HOSTNAME_REGEX         "^[a-z0-9.-]+\\.[a-z]{2,6}$"
#else
#define HOSTNAME_REGEX         DOMAINNAME_REGEX
#endif

      ZE_MessageInfo(10, "* HOSTNAME_REGEX %s", HOSTNAME_REGEX);
      if (strexpr(name, HOSTNAME_REGEX, NULL, NULL, TRUE))
        ZE_MessageInfo(10, "* %s is a domain name", name);
      else
        ZE_MessageInfo(10, "* %s isn't a domain name", name);
    }

    netclass = GetClientNetClass(ip, name, NULL, NULL, 0);
    strlcpy(class, NET_CLASS_LABEL(netclass), sizeof (class));

    memset(&rbwl, 0, sizeof (rbwl));
    flag = check_iprbwl_table("00000000.000", ip, name, &rbwl);
    if (flag > 0)
      strlcpy(class, rbwl.netclass, sizeof (class));

    ZE_MessageInfo(0, "Client IP address : %s", ip);
    ZE_MessageInfo(0, "Client hostname   : %s", name);
    ZE_MessageInfo(0, "Client NET class  : %02X %s", netclass, class);

    result = STRNULL(result, "...");

    ZE_MessageInfo(0, "RESULT            = %2d %s", flag, result);
  }

  return 0;
}
