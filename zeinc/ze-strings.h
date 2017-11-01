/*
 *
 * ze-filter - Mail Server Filter for sendmail
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
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#ifndef __JSTRINGS_H__

#define              STRLPCY_IS_SNPRINTF    1
#undef               STRLCPY_IS_SNPRINTF

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

long                jstrtol(char *);
char               *strrev(char *);
char               *strduprev(char *);
char               *strchomp(char *);
int                 strlequal(char *, char *);
int                 safe_strncpy(char *, size_t, char *, size_t);
int                 safe_strncat(char *, size_t, char *, size_t);
#if !HAVE_STRNDUP
char               *strndup(const char *sin, size_t n);
#endif  /* HAVE_STRNDUP */
char               *strcatdup(char *s1, char *s2);

int                 strcountchar(char *s, int c);

/*
** My versions of string functions
*/
char               *jm_malloc(size_t size);

long                jm_strtol(char *);

char               *jm_strrev(char *);

char               *jm_strchomp(char *);
int                 jm_strlequal(char *, char *);

int                 jm_safe_strncpy(char *, size_t, char *, size_t);
int                 jm_safe_strncat(char *, size_t, char *, size_t);

size_t              jm_strlcpy(char *, const char *, size_t);
size_t              jm_strlcat(char *, const char *, size_t);

char               *jm_strdup(char *);
char               *jm_strndup(char *sin, size_t n);
char               *jm_strcatdup(char *s1, char *s2);
char               *jm_strrevdup(char *);

char               *jm_strtolower(char *);
char               *jm_strtoupper(char *);

void                jm_center_string(char *, char *, int);

char               *jm_strset(char *, int, int);

void                jm_strchknull(char *, int);

size_t              jm_strclean(char *, size_t);

char               *jm_strnoblanks(char *, size_t);

char               *jm_str_clear_trailing_blanks(char *);

bool                jm_strexpr(char *, char *, long *, long *, bool);

int                 jm_str2tokens(char *, int, char **, char *);



#ifndef HAVE_STRLCPY
size_t              strlcpy(char *, const char *, size_t);
#endif             /* HAVE_STRLCPY */


#define __JSTRINGS_H__
#endif
