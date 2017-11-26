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
 *  Creation     : Tue Feb 21 22:09:21 CET 2006
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


#ifndef ZE_RCPT_LIST_H

typedef struct rcpt_addr_T rcpt_addr_T;

struct rcpt_addr_T
{
  char          *rcpt;
  char          *email;
  char          *host;
  char          *user;

  bool           quarantine;
  bool           deleted;
  int            access;
  rcpt_addr_T   *next;
};

#define        rcpt_count(r)    count_rcpt(r)

rcpt_addr_T   *rcpt_list_free(rcpt_addr_T *);

rcpt_addr_T   *rcpt_list_add(rcpt_addr_T **, char *, int);

int            rcpt_list_del(rcpt_addr_T **, char *rcpt);

int            count_rcpt(rcpt_addr_T *);


# define ZE_RCPT_LIST_H    1
#endif             /* J_RCPT_LIST_H */
