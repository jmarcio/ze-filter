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
 *  Creation     : Mon Nov 15 23:13:14 CET 2004
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


#ifndef __ZE_CALLBACKLOGS_H

/**
 * @addtogroup Logging
 *
 * @{
 */

#define             WARN_MSG_SIZE      0x4000

sfsistat            do_notify(SMFICTX *, attachment_T *, char *, char *, char *);


void                read_error_msg(char *buf, int, attachment_T *, char *,
                                   char *, char *, char *, CTXPRIV_T *);

void                log_msg_context(SMFICTX *ctx, char *why);


bool                add_tag2subject(SMFICTX *, char*);

#define  LOG_SPAM_CHECK(id, ...)                               \
do {                                                           \
  if (log_level >= 9 && cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)  \
  {                                                            \
    char sxa[256], sxb[256];                                   \
    snprintf(sxa, sizeof(sxa), "%s SPAM CHECK", id);           \
    snprintf(sxb, sizeof(sxb), __VA_ARGS__);                   \
    j_syslog(LOG_NOTICE, "%s - %s", sxa, sxb);	               \
  }                                                            \
} while (0)

#define  LOG_ORACLE(id, ...)                                   \
do {                                                           \
  if (log_level >= 9 && cf_get_int(CF_LOG_LEVEL_ORACLE) >= 1)  \
  {                                                            \
    char sxa[256], sxb[256];                                   \
    snprintf(sxa, sizeof(sxa), "%s ORACLE", id);               \
    snprintf(sxb, sizeof(sxb), __VA_ARGS__);                   \
    j_syslog(LOG_NOTICE, "%s - %s", sxa, sxb);	               \
  }                                                            \
} while (0)

bool                open_scores4stats_file();
bool                reopen_scores4stats_file();
bool                dump_msg_scores4stats(SMFICTX *ctx);

/** @} */

# define __ZE_CALLBACKLOGS_H    1
#endif             /* __ZE_CALLBACKLOGS_H */
