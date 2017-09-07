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

int                 v6net_hi = 64;
int                 v6net_lo = 32;
int                 v6net_step = 4;

void
ipv6_init_nets()
{
  char               *env;

  env = getenv("IPV6NET");
  if (env != NULL)
  {
    int                 argc;
    char               *argv[8];

    argc = str2tokens(env, 8, argv, ",");
    if (argc >= 3)
    {
      v6net_lo = atoi(argv[0]);
      v6net_hi = atoi(argv[1]);
      v6net_step = atoi(argv[2]);
    }
  }
}

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          log_level;

  bool                r;

  ipv6_T              ipv6, ip;
  char                buf[64];

  char               *s = NULL;

  configure("j-module-test", conf_file, FALSE);

  set_log_output(FALSE, TRUE);
  log_level = 10;

  ipv6_init_nets();

  printf("Hello, world\n");

  ipv6_str2rec(&ipv6, "1:2:3:4:5:6:7:8:9:10:11:12:13:14:15");
  ipv6_str2rec(&ipv6, "2001:0DB8:0:CD30:123:4567:89AB:CDEF");
  ipv6_str2rec(&ipv6, "2001:0DB8:0:CD30::4567:89AB:CDEF");
  ipv6_str2rec(&ipv6, "::1");
  ipv6_str2rec(&ipv6, "::");

  ipv6_expand(buf, "::1", sizeof (buf));
  ipv6_expand(buf, "2001:0DB8:0:CD30::4567:89AB:CDEF", sizeof (buf));

  s = "2001:660:3312::1::2/64";
  printf(" * IN     %s\n", s);
  if (ipv6_str2rec(&ipv6, s))
  {
    ipv6_rec2str(buf, &ipv6, sizeof (buf));
    printf(" * PARSED %s\n", buf);
  }

  s = "2001:660:33122::1/64";
  printf(" * IN     %s\n", s);
  if (ipv6_str2rec(&ipv6, s))
  {
    ipv6_rec2str(buf, &ipv6, sizeof (buf));
    printf(" * PARSED %s\n", buf);
  }

  s = "2001:660:3312:ffff::1/64";
  printf(" * IN     %s\n", s);
  if (ipv6_str2rec(&ipv6, s))
  {
    int                 i;

    ipv6_rec2str(buf, &ipv6, sizeof (buf));
    printf(" * PARSED %s\n", buf);

    for (i = v6net_hi; i >= v6net_lo; i -= v6net_step)
    {
      ipv6_prefix_str(&ipv6, buf, sizeof (buf), i);

      printf(" * NET    %s\n", buf);
    }
  }
  return 0;
}
