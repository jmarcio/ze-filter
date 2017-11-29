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


char               *zeStrRev(char *);
char               *zeStrDupRev(char *);
int                 zeStrlEqual(char *, char*);
int                 zeSafeStrnCat(char *, size_t, char *, size_t );
int                 zeSafeStrnCpy(char *, size_t, char *, size_t );
char               *zeStrnDup(const char *, size_t);
char               *zm_malloc(size_t);
char               *zeStrCatDup(char *, char *);
int                 zeStrCountChar(char *, int);
char               *zeStrJoin(char *, int, char **);
char               *zeStrDup(char *);
void               *zeMalloc(size_t);
char               *zeStr2Lower(char *);
char               *zeStr2Upper(char *);
char               *zeStrSet(char *, int, int);
void                zeStrChkNull(char *, int);
size_t              zeStrRmNulls(char *, size_t);
char               *zeStrRmBlanks(char *, size_t);
char               *zeStrRmHeadBlanks(char *, size_t);
char               *zeStrRmTailBlanks(char *, size_t);
char               *zeStrClearTrailingBlanks(char *);
char               *zeStrChomp(char *);
bool                zeStrRegex(char *, char *, long *, long *, bool);
void                zeStrCenter(char *, char *, int);
int                 zeStr2Tokens(char *, int, char **, char *);

char               *zm_malloc(size_t);
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
