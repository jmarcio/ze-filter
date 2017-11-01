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

#include <ze-sys.h>

#include <ze-filter.h>

#include <ze-avclient.h>

/*
**
**
*/

void
debug_reply_msg(s)
     char               *s;
{
  smtp_reply_T        r;

  if (s == NULL)
    return;

  printf("REPLY : %s\n", s);
  if (jc_string2reply(&r, s));
  {
    printf("  RCODE : %s\n", r.rcode);
    printf("  XCODE : %s\n", r.xcode);
    printf("  MSG   : %s\n", r.msg);
    printf("  RES   : %d\n", r.result);
  }
  printf("\n");
}


/*
**
**
*/
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

  configure("av-test", conf_file, FALSE);

  memset(answer, 0, sizeof (answer));

  set_log_output(TRUE, TRUE);
  log_level = 10;

  printf("Let's begin with serious things...\n");
  if (0)
  {
    LOG_T               log = LOG_INITIALIZER;
    bool                res;
    char               *spec = NULL;

    /* file */
    spec = "/tmp/singlepath.txt";
    log_debug(&log, TRUE);
    res = log_open(&log, spec);
    printf("log_open : %d\n", res);

    res = log_write(&log, "Test log_write before\n");

    res = log_reopen(&log);
    printf("log_reopen : %d\n", res);

    res = log_write(&log, "Test log_write after\n");
    res = log_printf(&log, "Test log_printf %s\n", "Coucou");

    res = log_close(&log);
    printf("log_close : %d\n", res);

    /* file */
    spec = "file:/tmp/logtest.txt";
    log_debug(&log, TRUE);
    res = log_open(&log, spec);
    printf("log_open : %d\n", res);

    res = log_write(&log, "Test log_write before\n");

    res = log_reopen(&log);
    printf("log_reopen : %d\n", res);

    res = log_write(&log, "Test log_write after\n");
    res = log_printf(&log, "Test log_printf %s\n", "Coucou");

    res = log_close(&log);
    printf("log_close : %d\n", res);

    /* udp */
    spec = "udp:10001@127.0.0.1";
    log_debug(&log, TRUE);
    res = log_open(&log, spec);
    printf("log_open : %d\n", res);

    res = log_write(&log, "Test log_write before\n");
    res = log_printf(&log, "Test log_printf %s\n", spec);

    res = log_reopen(&log);
    printf("log_reopen : %d\n", res);

    res = log_write(&log, "Test log_write after\n");
    res = log_printf(&log, "Test log_printf %s\n", spec);

    res = log_close(&log);
    printf("log_close : %d\n", res);

    /* syslog */
    spec = "syslog:av-test:warning";
    log_debug(&log, TRUE);
    res = log_open(&log, spec);
    printf("log_open : %d\n", res);

    res = log_write(&log, "Test log_write before\n");
    res = log_printf(&log, "Test log_printf %s\n", spec);

    res = log_reopen(&log);
    printf("log_reopen : %d\n", res);

    res = log_write(&log, "Test log_write after\n");
    res = log_printf(&log, "Test log_printf %s\n", spec);

    res = log_close(&log);
    printf("log_close : %d\n", res);

    /* syslog */
    spec = "syslog";
    log_debug(&log, TRUE);
    res = log_open(&log, spec);
    printf("log_open : %d\n", res);

    res = log_write(&log, "Test log_write before\n");
    res = log_printf(&log, "Test log_printf %s\n", spec);

    res = log_reopen(&log);
    printf("log_reopen : %d\n", res);

    res = log_write(&log, "Test log_write after\n");
    res = log_printf(&log, "Test log_printf %s\n", spec);

    res = log_close(&log);
    printf("log_close : %d\n", res);
  }

  if (0)
  {
    debug_reply_msg("error:421:4.4.0:Message with error");
    debug_reply_msg("error:550:5.7.0:Message with error");
    debug_reply_msg("ok");
    debug_reply_msg("tempfail");
    debug_reply_msg("reject");

    exit(0);
  }

  if (argc > 1)
    strlcpy(question, argv[1], sizeof (question));
  else
    strlcpy(question, "allnaturalpills.info", sizeof (question));

  {
    db_map_T            bl;

    memset(&bl, 0, sizeof (bl));

    if (db_blackliste_check("URLBL", question, &bl))
    {
      printf("Found       %s\n", question);
      printf(" WEIGHT     %d\n", bl.weight);
      printf(" DATE       %d\n", bl.date);
      printf(" IP         %s\n", bl.ipres);
      printf(" MSG        %s\n", bl.msg);
    }
  }

  exit(0);

  if (argc > 1)
    strlcpy(question, argv[1], sizeof (question));
  else
    strlcpy(question, in, sizeof (question));

  memset(out, 0, sizeof (out));

  res = av_client(out, sizeof (out), msg, sizeof (msg), question);
  printf("OUT : RES=(%d)\n", res);
  printf("OUT : ANSWER=(%s)\n", out);
  printf("OUT : MSG=(%s)\n", msg);
}
