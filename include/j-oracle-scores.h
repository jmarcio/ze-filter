/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Apr 24 22:00:19 CEST 2006
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


#ifndef J_ORACLE_SCORES_H


#define SPAM_CONN_RESOLVE_FAIL          1
#define SPAM_CONN_RESOLVE_FORGED        2
#define SPAM_CONN_RESOLVE_TEMPFAIL      3
#define SPAM_CONN_FALSE_LOCALHOST       4
#define SPAM_CONN_BL_SPAMTRAP           5
#define SPAM_CONN_BAD_EHLO              6
#define SPAM_CONN_FORGED_EHLO           7
#define SPAM_CONN_RBL                   8
#define SPAM_CONN_NB                    9

#define SPAM_HTML_CLEAN_TOO_SHORT       3
#define SPAM_HTML_BASE64                4
#define SPAM_HTML_UNWANTED_TAGS         5
#define SPAM_HTML_TAGS_RATIO            6
#define SPAM_HTML_INVALID_TAGS         10
#define SPAM_HTML_NB                   11

#define SPAM_PLAIN_EMPTY                1
#define SPAM_PLAIN_BASE64               2
#define SPAM_PLAIN_NO_CHARSET           3
#define SPAM_PLAIN_TOO_SHORT            4
#define SPAM_PLAIN_NB                   5

#define SPAM_MSG_NO_TEXT_PART           1
#define SPAM_MSG_TOO_MUCH_HTML          2
#define SPAM_MSG_BAD_DATE               3
#define SPAM_MSG_FUTURE_DATE            4
#define SPAM_MSG_TOO_OLD_DATE           5
#define SPAM_MSG_UNWANTED_CHARSET       6
#define SPAM_MSG_BAD_EXPRESSIONS        8
#define SPAM_MSG_FORGED_POSTMASTER      9
#define SPAM_MSG_BAD_SENDER_ADDRESS    10
#define SPAM_MSG_BAD_DOMAIN_ADDRESS    11
#define SPAM_MSG_NO_SUBJECT            12
#define SPAM_MSG_RFC2822_HEADERS       13
#define SPAM_MSG_HEADERS_SYNTAX        14
#define SPAM_MSG_BASE64                15
#define SPAM_MSG_BASE64_SUBJECT        16
#define SPAM_MSG_UNWANTED_BOUNDARY     17
#define SPAM_MSG_HAS_BADRCPT           18
#define SPAM_MSG_MIME_ERRORS           19
#define SPAM_MSG_UNWANTED_MAILER       21
#define SPAM_MSG_MATCH_MIME_PARTS      22
#define SPAM_MSG_HAS_SPAMTRAP          23
#define SPAM_MSG_TOO_SHORT             24
#define SPAM_MSG_BAD_NULL_SENDER       25
#define SPAM_MSG_SUBJECT_HI_CAPS       26
#define SPAM_MSG_CONTENT_ID            27
#define SPAM_MSG_EMPTY_ATTACHMENT      28
#define SPAM_MSG_SUBJECT_NO_ALPHA      29
#define SPAM_MSG_SIZE_000_010          30
#define SPAM_MSG_SIZE_010_020          31
#define SPAM_MSG_SIZE_020_040          32
#define SPAM_MSG_SIZE_040_080          33
#define SPAM_MSG_SIZE_080_160          34
#define SPAM_MSG_SIZE_160_999          35
#define SPAM_MSG_NB                    36

#define SPAM_MIME_CONTENT_ID            1

#define SPAM_GLOB_TAGGED                1

#define ORACLE_TYPE_CONN                1
#define ORACLE_TYPE_MSG                 2
#define ORACLE_TYPE_HTML                3
#define ORACLE_TYPE_PLAIN               4
#define ORACLE_TYPE_NB                  5

#define ORACLE_TYPE_GLOB              255


char               *oracle_get_label(int, int);
double              oracle_get_score(int, int);
bool                oracle_set_score(int, int, double);

long                oracle_get_count(int, int);
long                oracle_set_count(int, int, long);
long                oracle_inc_count(int, int);

void                oracle_stats_update(int );
void                oracle_stats_get(double *, double *, long *,
                                     double *, double *,long *);

bool                oracle_dump_counters(int , bool);
bool                oracle_read_counters();
bool                oracle_save_counters();

bool                oracle_read_scores();

bool                load_oracle_defs(char *cfdir, char *fname);
void                dump_oracle_defs();

bool                oracle_check_enabled(int, int);

int                 oracle_compute_score(char *, char *, spamchk_T *);



# define J_ORACLE_SCORES_H    1
#endif /* J_ORACLE_SCORES_H */

