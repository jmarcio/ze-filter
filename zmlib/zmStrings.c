
/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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


#include <zm-sys.h>
#include <zmLibs.h>
#include <zmStrings.h>

#include <j-chkmail.h>

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/



/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
char               *
zmStrJoin(sep, argc, argv)
     char               *sep;
     int                 argc;
     char              **argv;
{
  size_t              nlen = 0;
  int                 i;
  char               *s = NULL;

  for (i = 0, nlen = 0; i < argc && argv[i] != NULL; i++)
    nlen += strlen(argv[i]);
  nlen += (argc - 1) * strlen(sep);

  s = (char *) malloc(nlen + 1);
  strlcpy(s, argv[0], nlen + 1);
  for (i = 1; i < argc; i++) {
    strlcat(s, sep, nlen + 1);
    strlcat(s, argv[i], nlen + 1);
  }
  return s;
}

#if 0

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
count_uint32bits(val)
     uint32_t            val;
{
  int                 r = 0;
  int                 i;

  for (i = 0; i < 8 * sizeof (val); i++)
    if (GET_BIT(val, i))
      r++;
  return r;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#if 0
time_t
timestr2secs(s)
     char               *s;
{
  int                 l;
  time_t              n;
  char                strn[16];

  if ((s == NULL) || (strlen(s) == 0))
    return 3600;

  if ((l = strspn(s, "0123456789")) == 0)
    return 3600;

  if (l >= (sizeof (strn)))
    return 3600;

  memset(strn, 0, sizeof (strn));
  strncpy(strn, s, l);

#if HAVE_STRTOL
  errno = 0;
  n = strtol(strn, (char **) NULL, 10);
  if (errno == ERANGE || errno == EINVAL || n <= 0)
    n = 3600;
#else
  n = atoi(strn);
  if (n <= 0 | n > 32)
    n = 3600;
#endif
  s += l;

  if (strlen(s) == 0)
    return n;

  switch (*s) {
    case 's':
    case 'S':
      return n;
      break;
    case 'm':
    case 'M':
      return 60 * n;
      break;
    case 'h':
    case 'H':
      return 3600 * n;
      break;
    case 'd':
    case 'D':
      return 86400 * n;
      break;
  }
  return 3600;
}
#endif

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
jstrdup(s)
     char               *s;
{
  char               *p;
  size_t              sz = 0;

  if (s == NULL)
    return NULL;

  sz = strlen(s) + 1;
  p = malloc(sz);
  if (p != NULL)
    strlcpy(p, s, sz);
  else
    LOG_SYS_ERROR("malloc(s)");

  return p;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void               *
jmalloc(sz)
     size_t              sz;
{
  void               *p;
  size_t              xtra = (8 - sz % 8) % 8;

  p = malloc(sz + xtra);
  if (p == NULL) {
    LOG_SYS_ERROR("malloc(%lu)", (unsigned long) (sz + xtra));
  }

  return p;
}
#endif

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zmStr2Lower(char *s)
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = tolower(*p);
  return s;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zmStr2upper(char *s)
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = toupper(*p);
  return s;
}


/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zmStrSet(dst, c, len)
     char               *dst;
     int                 c;
     int                 len;
{
  if (dst != NULL) {
    memset(dst, (int) c, len);
    dst[len] = '\0';
  }
  return dst;
}

#if 0

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void
strchknull(s, len)
     char               *s;
     int                 len;
{
  char               *p = s;

  if (s == NULL)
    return;

  while (len-- > 0) {
    if (*p == '\0')
      *p = ' ';
    p++;
  }
}
#endif

/******************************************************************************
 *                                                                            * 
 * replace strclean                                                           *
 ******************************************************************************/
size_t
zmStrRmNulls(s, sz)
     char               *s;
     size_t              sz;
{
  size_t              nsz = 0;
  char               *p, *q;
  size_t              i;

  if (s == NULL)
    return 0;

  p = q = s;
  for (i = 0; i < sz; i++, p++) {
    switch (*p) {
      case '\0':
        break;
      default:
        *q++ = *p;
        nsz++;
        break;
    }
  }
  *q = 0;

  return nsz;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zmStrRmBlanks(s, size)
     char               *s;
     size_t              size;
{
  char               *p, *q;

  if (s == NULL)
    return NULL;

  p = q = s;
  while ((*p != '\0') && (size-- > 0)) {
    /*
     * ' ' or '\t' 
     */
    if (!isblank(*p))
      *q++ = *p;
    p++;
  }
  *q = 0;

  return s;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zmStrClearTrailingBlanks(s)
     char               *s;
{
  char               *p, *last;
  size_t              n;

  if (s == NULL || strlen(s) == 0)
    return s;
  for (p = last = s; *p != '\0'; p++)
    if (isblank(*p) == 0)
      last = p;
  if (isblank(*last) == 0)
    last++;
  *last = '\0';

  return s;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
#if defined(REGCOMP_FLAGS)
#undef REGCOMP_FLAGS
#endif

#define REGCOMP_FLAGS         (REG_ICASE | REG_EXTENDED)

bool
zmStrRegex(s, expr, pi, pf, icase)
     char               *s;
     char               *expr;
     long               *pi;
     long               *pf;
     bool                icase;
{
  regex_t             re;
  bool                found = FALSE;
  int                 rerror;
  int                 flags;

  if ((s == NULL) || (expr == NULL))
    return FALSE;

  flags = REG_EXTENDED | (icase ? REG_ICASE : 0);
  if ((rerror = regcomp(&re, expr, flags)) == 0) {
    regmatch_t          pm;

    if (regexec(&re, s, 1, &pm, 0) == 0) {
      if (pi != NULL)
        *pi = pm.rm_so;
      if (pf != NULL)
        *pf = pm.rm_eo;
      found = TRUE;
    }
    regfree(&re);
  } else {
    char                s[256];

    if (regerror(rerror, &re, s, sizeof (s)) > 0)
      LOG_MSG_ERROR("regcomp(%s) error : %s", expr, s);
  }

  return found;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
zmStrCenter(dst, org, ldst)
     char               *dst;
     char               *org;
     int                 ldst;
{
  char               *p;
  size_t              lorg;

  memset(dst, 0, ldst);
  lorg = strlen(org);
  if (ldst <= lorg + 1) {
    strlcpy(dst, org, ldst);
    return;
  }
  if (ldst > lorg + 1)
    memset(dst, ' ', (ldst - lorg) / 2);
  strlcat(dst, org, ldst);
}

#if 0

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
int
nb_valid_pointer(a, b, c)
     char               *a;
     char               *b;
     char               *c;
{
  int                 r = 0;

  if (a)
    r++;
  if (b)
    r++;
  if (c)
    r++;
  return r;
}

#endif

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zmStr2Tokens(s, sz, argv, sep)
     char               *s;
     int                 sz;
     char              **argv;
     char               *sep;
{
  int                 i;
  char               *p, *ptr;

  if (s == NULL || argv == NULL || sz == 0)
    return 0;

  sep = STRNULL(sep, ":,");

  for (i = 0; i < sz; i++)
    argv[i] = NULL;
  for (p = strtok_r(s, sep, &ptr), i = 0;
       p != NULL && i < sz - 1; p = strtok_r(NULL, sep, &ptr), i++) {
    argv[i] = p;
  }
  argv[i] = NULL;

  return i;
}
