/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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

#ifndef __ZE_STATS_H

#define           STAT_RESTART            0
#define           STAT_BYTES              1
#define           STAT_MSGS               2
#define           STAT_CONNECT            3
#define           STAT_ABORT              4
#define           STAT_CLOSE              5
#define           STAT_ENVTO              6
#define           STAT_FILES              7
#define           STAT_XFILES             8
#define           STAT_VIRUS              9
#define           STAT_LUSERS            10

#define           STAT_NO_TO_HEADERS     12
#define           STAT_NO_FROM_HEADERS   13

#define           STAT_RESOLVE_FAIL      14
#define           STAT_RESOLVE_FORGED    15
#define           STAT_MAX_RCPT          16
#define           STAT_CONN_RATE         17
#define           STAT_RCPT_RATE         18
#define           STAT_MAX_MSGS          19

#define           STAT_NO_HEADERS        20

#define           STAT_PATTERN_MATCHING  21
#define           STAT_SUBJECT_CONTENTS  22
#define           STAT_HEADERS_CONTENTS  23
#define           STAT_HELO_CONTENTS     24
#define           STAT_FROM_CONTENTS     25
#define           STAT_ORACLE            26
#define           STAT_URLBL             27

#define           STAT_POLICY            30

#define           STAT_BINARY            31
#define           STAT_BASE64            32
#define           STAT_QUOTED_PRINTABLE  33

#define           STAT_BAYES_SPAM        36
#define           STAT_BAYES_HAM         37
#define           STAT_BAYES_DUB         38

#define           STAT_SINGLE_MESSAGE    40
#define           STAT_MSG_RATE          41

#define           STAT_OPEN_CONN         42
#define           STAT_EMPTY_CONN        43

#define           STAT_BAD_RCPT          44
#if 0
#define           STAT_SPAMTRAP          45
#endif
#define           STAT_BADMX             46
#define           STAT_GREY_RCPT         47
#define           STAT_GREY_MSGS         48

#define           STAT_RCPT_TEMPFAIL     50
#define           STAT_RCPT_REJECT       51
#define           STAT_RCPT_ACCESS       52
#define           STAT_RCPT_BAD_NETWORK  53
#define           STAT_RCPT_UNKNOWN      54
#define           STAT_RCPT_SPAMTRAP     55

#define           STAT_XX               100

#define           DIM_STATS             128



typedef struct p_stats_T
{
  pid_t               pid;
  time_t              start;
#if 0
  int64_t             value[DIM_STATS];
#else
  long                value[DIM_STATS];
#endif
} p_stats_T;

typedef struct j_stats_T
{
  long                signature;
  long                vers;
  char                version[128];
  time_t              last_save;
  p_stats_T           glob;
  p_stats_T           proc;
} j_stats_T;


void                log_counters(int, bool);

void                stats_inc(int, long);

void                stats_reset();

void                init_proc_state();

void                save_state();

void                read_state();

void                reset_state();

int                 print_state();

char               *stats_title(int);


void                print_p_stats_all(int, p_stats_T *, char *, p_stats_T *, char *,
                                      int, bool);
void                print_p_stats(int, p_stats_T *, char *, int, int);
int                 dump_state(int, int, int, int, int);

void                print_filter_stats_summary(void);

void                msg_score_stats_update(msg_scores_T *scores);
void                msg_score_stats_print(int, int);

#define __ZE_STATS_H
#endif
