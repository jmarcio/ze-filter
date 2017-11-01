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

void
usage(arg)
     char               *arg;
{
  char               *tprefix, *kprefix, *ip, *name, *from, *to, *key;

  tprefix = "GreyCheck";;
  kprefix = "NetClass";

  ip = "10.3.5.5";
  name = "minho.ensmp.fr";
  from = "root@domain.com";
  to = "joe@domain.com";
  key = "10.3.5.5";

  printf("Usage : %s options\n"
         "    -t          : prefix + triplet (default is prefix + key)\n"
         "    -P prefix   : e.g.  -P %s\n"
         "    -K key      : e.g.  -K %s\n"
         "    -I address  : e.g.  -I %s\n"
         "    -H hostname : e.g.  -H %s\n"
         "    -F from     : e.g.  -F %s\n"
         "    -T to       : e.g.  -T %s\n"
         "    -v          : increase log level\n"
         "    -h          : help - this message\n"
         "\n"
         "  Sample queries : \n"
         "    %s -t -P %s -I %s -H %s -F %s -T %s\n"
         "    %s -P %s -K %s\n"
         "\n"
         "  %s\n" "  Compiled on %s\n",
         arg,
         tprefix, key, ip, name, from, to,
         arg, tprefix, ip, name, from, to,
         arg, kprefix, key, PACKAGE, __DATE__ " " __TIME__);
}


int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          log_level;

  char               *prefix, *ip, *name, *from, *to, *key;
  int                 netclass = NET_UNKNOWN;
  bool                triplet = FALSE;

  prefix = "ContentCheck";
  ip = "0.0.0.0";
  name = "localhost";
  from = "nobody@localhost";
  to = "nobody@localdomain";
  key = "0.0.0.0";

  configure("j-policy-test", conf_file, FALSE);

  set_log_output(FALSE, TRUE);
  log_level = 10;

  {
    const char         *args = "P:K:I:H:F:T:htv";
    int                 c;

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
        case 'h':
          usage(argv[0]);
          exit(0);
          break;
        case 'P':
          prefix = optarg;
          break;
        case 'K':
          key = optarg;
          break;
        case 'I':
          ip = optarg;
          break;
        case 'H':
          name = optarg;
          break;
        case 'F':
          from = optarg;
          break;
        case 'T':
          to = optarg;
          break;
        case 't':
          triplet = !triplet;
          break;
        case 'v':
          log_level++;
          break;
        case 'l':
          break;
        default:
          exit(0);
          break;
      }
    }
  }

  if (!policy_init())
  {
    MESSAGE_INFO(0, "Error opening policy database !");
    exit(1);
  }

  if (triplet)
  {
    bool                flag;
    char                class[256];

    memset(class, 0, sizeof (class));
    netclass = GetClientNetClass(ip, name, NULL, class, sizeof (class));

    if (strlen(class) == 0)
      strlcpy(class, NET_CLASS_LABEL(netclass), sizeof (class));
    MESSAGE_INFO(0, "Client IP address : %s", ip);
    MESSAGE_INFO(0, "Client hostname   : %s", name);
    MESSAGE_INFO(0, "Sender            : %s", from);
    MESSAGE_INFO(0, "Recipient         : %s", to);
    MESSAGE_INFO(0, "Client NET class  : %02X %s", netclass, class);
    MESSAGE_INFO(0, "");
    MESSAGE_INFO(0, "Checking = %s %s %s %s %s", prefix, ip, name, from, to);
    flag = check_policy_tuple(prefix, ip, name, class, from, to, TRUE);
    MESSAGE_INFO(0, "RESULT   = %s", STRBOOL(flag, "YES", "NO"));
  } else
  {
    bool                flag;
    char                buf[256];

    MESSAGE_INFO(0, "Checking = %s %s", prefix, key);
    memset(buf, 0, sizeof (buf));
    flag = check_policy(prefix, key, buf, sizeof (buf), TRUE);
    MESSAGE_INFO(0, "RESULT   = %s - %s", buf, STRBOOL(flag, "YES", "NO"));
  }

  policy_close();

  return 0;
}
