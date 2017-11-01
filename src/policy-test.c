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

#include <j-avclient.h>

bool                shall_notify_user(char *, bool);

void
print_str_bounds(s, i, f)
     char               *s;
     int                 i;
     int                 f;
{
  char                buf[256];

  memset(buf, 0, sizeof (buf));
  strcpy(buf, s);
  printf("  -> %s\n", buf);

  memset(buf, 0, sizeof (buf));
  strset(buf, ' ', strlen(s) + 1);
  buf[i] = buf[f] = '+';
  printf("  -> %s\n", buf);
}


int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  char                out[256];
  int                 res = 0;
  char               *in = "/tmp/Yaha.P";

  char                question[2048];
  char                answer[2048];
  char                msg[2048];
  int                 avres;

  extern int          log_level;

  configure("j-policy-test", conf_file, FALSE);

  memset(answer, 0, sizeof (answer));

  set_log_output(TRUE, TRUE);
  log_level = 10;

  {
    char               *ip;

    if (argc > 1)
      ip = argv[1];
    else
      ip = "194.214.158.176";

  }

  if (1)
  {
    char               *prefix, *ip, *name, *from, *to;
    bool                flag;

    prefix = ip = name = from = to = NULL;

    prefix = STRBOOL(argc > 1, argv[1], "-");
    ip = STRBOOL(argc > 2, argv[2], "-");
    name = STRBOOL(argc > 3, argv[3], "-");
    from = STRBOOL(argc > 4, argv[4], "-");
    to = STRBOOL(argc > 5, argv[5], "-");

    MESSAGE_INFO(0, "Checking = %s %s %s %s", ip, name, from, to);

    if (db_policy_open(TRUE))
    {
      flag = check_policy_tuple(prefix, ip, name, NULL, from, to, FALSE);

      MESSAGE_INFO(0, "RESULT = %s", STRBOOL(flag, "YES", "NO"));

      db_policy_close();
    } else
      MESSAGE_INFO(0, "Error opening policy database !");

    return 0;

  }
#if 0
  {
    bool                res = FALSE;

    char               *ip, *from, *to, hostname[256];

    printf("ARGC = %d\n", argc);

    gethostname(hostname, sizeof (hostname));
    /*strcpy(hostname, "minho"); */
    ip = STRBOOL(argc > 1, argv[1], "1.2.3.4");
    from = STRBOOL(argc > 2, argv[2], "martins@ensmp.fr");
    to = STRBOOL(argc > 3, argv[3], "joe@ensmp.fr");

    res = grey_init(FALSE);
    MESSAGE_INFO(0, "GREY_INIT : %s %s %s %s : %s",
                 STRNULL(ip, "IP"),
                 STRNULL(from, "FROM"),
                 STRNULL(to, "TO"),
                 STRNULL(hostname, "HOSTNAME"), STRBOOL(res, "TRUE", "FALSE"));

    res = grey_check(ip, from, to, hostname);

    MESSAGE_INFO(0, "GREY_CHECK : %s %s %s : %s",
                 STRNULL(ip, "IP"),
                 STRNULL(from, "FROM"),
                 STRNULL(to, "TO"), STRBOOL(res, "TRUE", "FALSE"));

    MESSAGE_INFO(0, "TIME %ld", time(NULL));
    grey_close();

    return 0;
  }
#endif

  if (db_policy_open(TRUE))
  {
    long                pi, pf;
    char               *s, *expr;
    bool                found;

    char               *prefix;
    char               *key;

    char                buf[256];

    if (argc > 1)
      prefix = argv[1];

    if (argc > 2)
      key = argv[2];

#if 1
    prefix = STRNULL(prefix, "");
    key = STRNULL(key, "");
#else
    prefix = "NetClass";
    key = "194.214.158.200";
#endif
    memset(buf, 0, sizeof (buf));
    check_policy(prefix, key, buf, sizeof (buf), TRUE);
    printf("POLICY : %s(%s) = %s\n", prefix, key, buf);
    printf("\n");
  }

  {
    bool                ok;
    char               *s;

    s = "joe@ensmp.fr";
    ok = shall_notify_user(s, FALSE);
    printf(" -> %-30s %s\n", s, STRBOOL(ok, "TRUE", "FALSE"));

    s = "martins@ensmp.fr";
    ok = shall_notify_user(s, FALSE);
    printf(" -> %-30s %s\n", s, STRBOOL(ok, "TRUE", "FALSE"));

    s = "joe@ensmp.com";
    ok = shall_notify_user(s, FALSE);
    printf(" -> %-30s %s\n", s, STRBOOL(ok, "TRUE", "FALSE"));

    s = "martins@joe";
    ok = shall_notify_user(s, FALSE);
    printf(" -> %-30s %s\n", s, STRBOOL(ok, "TRUE", "FALSE"));

    s = "martins@fr.joe";
    ok = shall_notify_user(s, FALSE);
    printf(" -> %-30s %s\n", s, STRBOOL(ok, "TRUE", "FALSE"));

    s = "charles@free.fr";
    ok = shall_notify_user(s, FALSE);
    printf(" -> %-30s %s\n", s, STRBOOL(ok, "TRUE", "FALSE"));
  }
}

bool
shall_notify_user(user_addr, to)
     char               *user_addr;
     bool                to;
{
  bool                result = FALSE;

  char                buf[256];

  memset(buf, 0, sizeof (buf));
  if (check_policy("NotifyUser", user_addr, buf, sizeof (buf), FALSE))
  {
    if (strcasecmp(buf, "NO") == 0)
      return FALSE;
    if (strcasecmp(buf, "YES") == 0)
      return TRUE;
    if (strcasecmp(buf, "RECIPIENT") == 0)
      return to;
    if (strcasecmp(buf, "SENDER") == 0)
      return !to;
    if (strcasecmp(buf, "DAILY") == 0)
      return FALSE;
  }

  return result;
}
