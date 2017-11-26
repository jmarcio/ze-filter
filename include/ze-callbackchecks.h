/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Nov 15 23:04:23 CET 2004
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


#ifndef ZE_CALLBACKCHECKS_H

#ifndef MAX_MSG_SCORE
# define MAX_MSG_SCORE 15
#endif


uint32_t     check_ehlo_value(SMFICTX *);

bool         compatible_domains(char *, char *);

sfsistat     check_msg_contents(SMFICTX *);
sfsistat     evaluate_message_score(SMFICTX *ctx, bool *);

bool         check_spamtrap_rcpt(char *, char *, char *, char *, int);

bool         check_recipient_quarantine(rcpt_addr_T *, int);

bool         shall_check_content(SMFICTX *);
bool         shall_check_xfiles(SMFICTX *);
bool         shall_check_virus(SMFICTX *);

bool         shall_designated_quarantine(SMFICTX *, char *);

bool         check_intranet_user(char *, char *, char *);

bool         shall_notify_user(char *, bool);


# define ZE_CALLBACKCHECKS_H    1
#endif /* J_CALLBACKCHECKS_H */

