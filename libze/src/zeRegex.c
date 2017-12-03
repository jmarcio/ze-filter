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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>
#include <libze.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#if USE_PCRE
#define J_PCRE_FLAGS           (PCRE_CASELESS | PCRE_DOTALL)
#else
#define J_PCRE_FLAGS            0
#endif

#define JREGCOMP_FLAGS         (REG_ICASE | REG_EXTENDED)
#define JREGEXEC_FLAGS          0

#include "libze.h"

#include <ze-regex.h>

#if USE_PCRE
static bool         use_pcre = TRUE;
#else
static bool         use_pcre = FALSE;
#endif             /* USE_PCRE */


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeRegexComp(re, expr, flags)
     zeRegex_T           *re;
     char               *expr;
     int                 flags;
{
  if ((re == NULL) || (expr == NULL))
    return FALSE;

#if USE_PCRE
  use_pcre = TRUE;
#else
  use_pcre = FALSE;
#endif             /* USE_PCRE */

  ZE_MessageInfo(20, "Using PCRE : %s", use_pcre ? "YES" : "NO"); 

  if (re->signature == SIGNATURE)
    zeRegexFree(re);

#if USE_PCRE
  if (use_pcre)
  {

    if (re->signature != SIGNATURE)
    {
      const char         *errptr = NULL;
      int                 erroffset = 0;

      re->pcre_rebase = pcre_compile(expr, J_PCRE_FLAGS, &errptr, &erroffset, NULL);
      if (re->pcre_rebase == NULL)
        ZE_LogMsgError(0, "pcre_compile error : %s", errptr);
      if (re->pcre_rebase != NULL)
      {
        re->pcre_rextra = pcre_study(re->pcre_rebase, 0, &errptr);
        if (re->pcre_rextra == NULL)
          ZE_LogMsgInfo(12, "pcre_study error : %s", errptr);
      }
      re->re_ok = (re->pcre_rebase != NULL);
    }
  } else
#endif
  {
    re->result = regcomp(&re->re, expr, flags);
  }

  if (re->result == 0)
  {
    /*
    ** See this latter if shall save expression
    */
#if 1
    re->expr = strdup(expr);
#endif
    re->signature = SIGNATURE;
  }
  return (re->result == 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define    DIM_VECTOR     (3 * 32)

bool
zeRegexExec(re, s, pi, pf, flags)
     zeRegex_T           *re;
     char               *s;
     long               *pi;
     long               *pf;
     int                 flags;
{
  regmatch_t          pm;

  if ((re == NULL) || (re->signature != SIGNATURE) || (s == NULL))
    return FALSE;

  re->result = -1;
#if USE_PCRE
  if (use_pcre)
  {
    int                 ovector[DIM_VECTOR];
    int                 rc;
    
    rc = pcre_exec(re->pcre_rebase, re->pcre_rextra, s, strlen(s),
                   0, 0, ovector, DIM_VECTOR);
    if (rc >= 0)
    {
      re->result = 0;
      if (pi != NULL)
        *pi = ovector[0];
      if (pf != NULL)
        *pf = ovector[1];
    }
  } else
#endif
  {
    re->result = regexec(&re->re, s, 1, &pm, flags);
    if (re->result == 0)
    {
      if (pi != NULL)
        *pi = pm.rm_so;
      if (pf != NULL)
        *pf = pm.rm_eo;
    }
  }

  return (re->result == 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
zeRegexFree(re)
     zeRegex_T           *re;
{
  if ((re == NULL) || (re->signature != SIGNATURE))
    return;

#if USE_PCRE
  if (use_pcre)
  {
    if (re->pcre_rebase != NULL)
      pcre_free(re->pcre_rebase);
    if (re->pcre_rextra != NULL)
      pcre_free(re->pcre_rextra);
    re->pcre_rebase = NULL;
    re->pcre_rextra = NULL;
  } else
#endif
  {
    regfree(&re->re);
  }

  FREE(re->expr);
  re->signature = 0;
  memset(re, 0, sizeof (zeRegex_T));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeRegexCount(s, expr)
     char               *s;
     char               *expr;
{
  int                 nb = 0;
  long                pf = 0;
  char               *p;
  zeRegex_T            re;

  if ((s == NULL) || (expr == NULL))
    return FALSE;

  memset(&re, 0, sizeof (re));

  if (zeRegexComp(&re, expr, JREGCOMP_FLAGS))
  {
    for (p = s; zeRegexExec(&re, p, NULL, &pf, 0); p += pf)
      nb++;
    zeRegexFree(&re);
  }

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeRegexLookup(s, expr, pi, pf)
     char               *s;
     char               *expr;
     long               *pi;
     long               *pf;
{
  zeRegex_T            re;
  bool                found = FALSE;
  long                xi, xf;

  if ((s == NULL) || (expr == NULL))
    return FALSE;

  xi = xf = 0;
  memset(&re, 0, sizeof (re));

  if (zeRegexComp(&re, expr, JREGCOMP_FLAGS))
  {
    found = zeRegexExec(&re, s, &xi, &xf, JREGEXEC_FLAGS);
    zeRegexFree(&re);
    if (found)
    {
      if (pi != NULL)
        *pi = xi;
      if (pf != NULL)
        *pf = xf;
    }
  }
  return found;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
zeRegexError(re)
     zeRegex_T           *re;
{

  return NULL;
}
