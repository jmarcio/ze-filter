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
 *  Creation     : Fri Oct  8 15:44:40 CEST 2004
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


#ifndef J_DNS_PARSE_H


#define MAX_HOST    128

typedef struct DNS_HOST_T
{
  int                 pref;
  char               *ip;
  char               *name;
  char               *txt;
} DNS_HOST_T;

typedef struct DNS_HOSTARR_T
{
  char               *domain;
  DNS_HOST_T          host[MAX_HOST];
  int                 count;
} DNS_HOSTARR_T;

void                dns_free_hostarr(DNS_HOSTARR_T *mx);

int                 dns_get_a(char *domain, DNS_HOSTARR_T *a);

int                 dns_get_aaaa(char *domain, DNS_HOSTARR_T *a);

int                 dns_get_mx(char *domain, DNS_HOSTARR_T *mx);

void                print_dns_reply(DNS_REPLY_T * r, int level);

# define J_DNS_PARSE_H    1
#endif             /* J_DNS_PARSE_H */
