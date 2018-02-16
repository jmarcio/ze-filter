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


#ifndef __ZE_REGEX_H


struct jregex_t
{
  uint32_t            signature;
  char               *expr;
  regex_t             re;

#if USE_PCRE
  pcre               *pcre_rebase;
  pcre_extra         *pcre_rextra;
#endif

  int                 result;
  bool                re_ok;
};

typedef struct jregex_t jregex_t;


bool                jreg_comp(jregex_t *, char *, int);

bool                jreg_exec(jregex_t *, char *, long *, long *, int);

void                jreg_free(jregex_t *);

int                 jreg_count(char *, char *);

bool                jreg_lookup(char *, char *, long *, long *);

char               *jreg_error(jregex_t *);



#define __ZE_REGEX_H
#endif
