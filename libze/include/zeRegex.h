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

/** @addtogroup APIs
*
* @{
*/


struct zeRegex_T
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

typedef struct zeRegex_T zeRegex_T;


bool                zeRegexComp(zeRegex_T *, char *, int);

bool                zeRegexExec(zeRegex_T *, char *, long *, long *, int);

void                zeRegexFree(zeRegex_T *);

int                 zeRegexCount(char *, char *);

bool                zeRegexLookup(char *, char *, long *, long *);

char               *zeRegexError(zeRegex_T *);



/** @} */

#define __ZE_REGEX_H
#endif
