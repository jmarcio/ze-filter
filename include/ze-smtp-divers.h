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


#ifndef __ZE_SMTP_DIVERS_H

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

#define __ZE_SMTP_DIVERS_H
#endif
