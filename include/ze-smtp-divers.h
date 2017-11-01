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


#ifndef __JSMTP_DIVERS_H__

time_t              header_date2secs(char *date);

char               *extract_email_address(char *, char *, size_t);
char               *extract_host_from_email_address(char *, char *, size_t);

typedef struct smtp_reply_T
{
  uint32_t            signature;
  char                rcode[6];
  char                xcode[6];
  char                msg[256];
  int                 result;
} smtp_reply_T;

int                 jc_string2reply(smtp_reply_T *, char *);
bool                jc_fill_reply(smtp_reply_T *, char *, char *, char *, int);
void                jc_reply_free(smtp_reply_T *);

#define __JSMTP_DIVERS_H__
#endif
