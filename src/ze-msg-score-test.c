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

#if 1
#define COEFS "KBAYES=1.0, KURLBL=0.04, KREGEX=0.02, KORACLE=0.08  "
#define SCALE "SSCORE=7."
#endif

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 i;

  int                 res = 0;
  extern int          ze_logLevel;

  bool                eval = FALSE;
  char               *s;
  double              score = 0.;
  char                buf[256];

  msg_scores_T        scores;

  memset(&scores, 0, sizeof (scores));
  scores.bayes = 0.4;
  scores.urlbl = 10;
  scores.body = 0;
  scores.headers = 0;
  scores.oracle = 0;


  ze_logLevel = 10;
  zeLog_SetOutput(FALSE, TRUE);

  ZE_MessageInfo(9, "\n --- ");
  s = "VECTOR; " COEFS;

  eval = register_msg_action(MSG_ACTION_OK, "REGEX:");
  printf("EVAL = %d\n", eval);
  eval = register_msg_action(MSG_ACTION_REJECT, "REGEX:");
  printf("EVAL = %d\n", eval);
  eval = register_msg_action(MSG_ACTION_DISCARD, "REGEX:");
  printf("EVAL = %d\n", eval);
  eval = register_msg_action(MSG_ACTION_QUARANTINE, "REGEX:");
  printf("EVAL = %d\n", eval);
  eval = register_msg_action(MSG_ACTION_HEADER_HAM, "REGEX:");
  printf("EVAL = %d\n", eval);
  eval = register_msg_action(MSG_ACTION_HEADER_SPAM_LO, "REGEX:");
  printf("EVAL = %d\n", eval);
  eval = register_msg_action(MSG_ACTION_HEADER_SPAM_HI, "THRESHOLD:0.650");
  printf("EVAL = %d\n", eval);

  s = "REGEX: ze-filter score : XXX.*U=##";
  eval = register_msg_action(MSG_ACTION_HEADER_NEUTRAL, s);
  printf("EVAL = %d\n", eval);

  eval = evaluate_msg_action(MSG_ACTION_HEADER_SPAM_HI, NULL, 0.7, NULL);
  printf("ACTION = %d\n", eval);
  eval = evaluate_msg_action(MSG_ACTION_HEADER_SPAM_HI, NULL, 0.8, NULL);
  printf("ACTION = %d\n", eval);

  eval = evaluate_msg_action(MSG_ACTION_HEADER_NEUTRAL, NULL, 0.8, NULL);
  printf("ACTION = %d\n", eval);

  s =
    "MSGID : NOID on UNKNOWN : ze-filter score : XX : R=. U=# O=. B=0.400 -> S=0.566";
  eval = evaluate_msg_action(MSG_ACTION_HEADER_NEUTRAL, NULL, 0.8, s);
  printf("ACTION = %d\n", eval);

  s =
    "MSGID : NOID on UNKNOWN : ze-filter score : XXX : R=. U=## O=. B=0.400 -> S=0.566";
  eval = evaluate_msg_action(MSG_ACTION_HEADER_NEUTRAL, NULL, 0.8, s);
  printf("ACTION = %d\n", eval);


  s = NULL;
  eval = configure_msg_eval_function(s);
  printf("* %-60s : %s\n", STRNULL(s, "NULL"), STRBOOL(eval, "OK", "KO"));
  display_msg_eval();

  score = compute_msg_score(&scores);
  ZE_MessageInfo(9, "  SCORE : %7.3f", score);
  create_msg_score_header(buf, sizeof (buf), NULL, NULL, &scores);
  ZE_MessageInfo(9, "  HEADER : %s", buf);

#if 0
  ZE_MessageInfo(9, "\n --- ??? ");
  score = compute_msg_score(regex, oracle, urlbl, -1.);
  ZE_MessageInfo(9, "  SCORE : %7.3f", score);
  create_msg_score_header(buf, sizeof (buf), NULL, NULL, regex, oracle, urlbl, -1);
  ZE_MessageInfo(9, "  HEADER : %s", buf);

  ZE_MessageInfo(9, "\n --- ??? ");
  score = evaluate_msg_score(regex, oracle, urlbl, -1.);
  ZE_MessageInfo(9, "  SCORE : %7.3f", score);
  create_msg_score_header(buf, sizeof (buf), NULL, NULL, regex, oracle, urlbl, -1);
  ZE_MessageInfo(9, "  HEADER : %s", buf);
#endif

#if 0
  ZE_MessageInfo(9, "\n --- ");
  s = "VEC; " COEFS;
  eval = configure_msg_eval_function(s);
  printf("* %-60s : %s\n", STRNULL(s, "NULL"), STRBOOL(eval, "OK", "KO"));
  display_msg_eval();

  score = evaluate_msg_score(regex, oracle, urlbl, bayes);
  ZE_MessageInfo(9, "  SCORE : %7.3f", score);
  create_msg_score_header(buf, sizeof (buf), NULL, NULL,
                          regex, oracle, urlbl, bayes);
  ZE_MessageInfo(9, "  HEADER : %s", buf);
#endif

#if 0
  ZE_MessageInfo(9, "\n --- ");
  s = "VECTOR; " COEFS;
  eval = configure_msg_eval_function(s);
  printf("* %-60s : %s\n", STRNULL(s, "NULL"), STRBOOL(eval, "OK", "KO"));
  display_msg_eval();

  score = evaluate_msg_score(regex, oracle, urlbl, bayes);
  ZE_MessageInfo(9, "  SCORE : %7.3f", score);
  create_msg_score_header(buf, sizeof (buf), NULL, NULL,
                          regex, oracle, urlbl, bayes);
  ZE_MessageInfo(9, "  HEADER : %s", buf);
#endif

  return 0;
}
