/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Tue Nov 28 21:56:02 CET 2006
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


#ifndef ZE_MSG_SCORE_H


#define   BSCORE_LO     0.25
#define   BSCORE_MI     0.50
#define   BSCORE_HI     0.65



#define   MSG_ACTION_UNDEF            -1
#define   MSG_ACTION_OK                0

#define   MSG_ACTION_REJECT            1
#define   MSG_ACTION_DISCARD           2

#define   MSG_ACTION_QUARANTINE        3

#define   MSG_ACTION_HEADER_HAM        4
#define   MSG_ACTION_HEADER_SPAM_LO    5
#define   MSG_ACTION_HEADER_SPAM_HI    6
#define   MSG_ACTION_HEADER_NEUTRAL    7

#define   MSG_EVAL_FUNCTION           11

typedef struct scores_scale_T
{
  double              kf1;
  double              kf0;
  double              bayes;
  double              oracle;
  double              urlbl;
  double              regex;
} scores_scale_T;

typedef struct test_score_T
{
  bool                actif;
  double              value;
  double              odds;
} test_score_T;

typedef struct msg_scores_T
{
  double              combined;
  bool                spam;

  bool                do_regex;
  int                 body;
  int                 headers;

  bool                do_urlbl;
  int                 urlbl;

  bool                do_oracle;
  int                 oracle;
  double              noracle;

  bool                do_bayes;
  double              bayes;
#if 0
  bool                do_logreg;
  double              logreg;
#endif
  scores_scale_T      scale;

  /* NEW */
  test_score_T         Bayes;
  test_score_T         LogReg;
  test_score_T         Urlbl;
  test_score_T         Oracle;
  test_score_T         Headers;
  test_score_T         Body;

  test_score_T         Global;
} msg_scores_T;

/*
 *
 */
bool                configure_msg_eval_function(char *val);
bool                configure_msg_score_scales(char *val);

bool                display_msg_eval();
double              compute_msg_score(msg_scores_T * scores);

bool                create_msg_score_header(char *buf,
                                            size_t size,
                                            char *id,
                                            char *hostname,
                                            msg_scores_T * scores);

#if 1
#define   DEFAULT_MSG_EVAL  "VECTOR; KBAYES=1.0; KURLBL=0.040; KREGEX=0.020; KORACLE=0.083"
#else
#define   DEFAULT_MSG_EVAL  "LOGIT; KBAYES=1.0; KURLBL=0.040; KREGEX=0.020; KORACLE=0.083"
#endif
#define   DEFAULT_MSG_SCALE "SSCORE1=7.; SSCORE0=0.; SBAYES=1.0; SURLBL=0.2; SREGEX=0.2; SORACLE=1."

bool                fill_msg_scale(scores_scale_T * scale);

/*
 *
 */
bool                register_msg_action(int which, char *val);
bool                evaluate_msg_action(int action,
                                        msg_scores_T * scp,
                                        double score, char *str);

# define ZE_MSG_SCORE_H    1
#endif             /* J_MSG_SCORE_H */
