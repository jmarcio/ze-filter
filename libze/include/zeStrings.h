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
 *  Creation     : Sun Jun 15 21:10:02 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#ifndef ZMSTRINGS_H

char               *zeStrJoin(char *, int, char **);
bool                zeStrRegex(char *, char *, long *, long *, bool);

#if 0
zeStrRev(s)
zeStrDupRev(s)
zeStrlEqual(sa, sb)
zeSafeStrnCat(out, sz, in, n)
zeSafeStrnCpy(out, sz, in, n)
zeStrnDup(sin, n)
zm_malloc(size)
zeStrCatDup(s1, s2)
zeStrCountChar(s, c)
zm_snprintf(char *s, size_t maxlen, const char *format, ...)

zeStrJoin(sep, argc, argv)
zeStrDup(s)
zeMalloc(sz)
zeStr2Lower(s)
zeStr2Upper(s)
zeStrSet(dst, c, len)
zeStrChkNull(s, len)
zeStrRmNulls(s, sz)
zeStrChomp(s)
zeStrRmBlanks(s, size)
zeStrClearTrailingBlanks(s)
zeStrRegex(s, expr, pi, pf, icase)
zeStrCenter(dst, org, ldst)
zeStr2Tokens(s, sz, argv, sep)
#endif

#if 0
#define STRCASEEQUAL(a,b)                                               \
  ((a) != NULL && (b) != NULL ? strcasecmp((a),(b)) == 0 : ((a) == (b)))

#define STRNCASEEQUAL(a,b,n)                                            \
  ((a) != NULL && (b) != NULL ? strcasecmp((a),(b),(n)) == 0 : ((a) == (b)))

#define STREQUAL(a,b)                                                   \
  ((a) != NULL && (b) != NULL ? strcmp((a),(b)) == 0 : ((a) == (b)))

#endif
#define STRNULL(x,r)             ((x) != NULL ? (x) : (r))
#define STREMPTY(x,r)            ((x) != NULL && strlen(x) > 0 ? (x) : (r))
#define STRBOOL(x,t,f)           ((x) ? t : f)


# define ZMSTRINGS_H    1
#endif             /* ZMSTRINGS_H */
