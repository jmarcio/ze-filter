/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2007 - Ecole des Mines de Paris
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 Jose-Marcio.Martins@ensmp.fr
 *
 *  Historique   :
 *  Creation     : Sat Jun  2 22:11:39 CEST 2007
 *
 * This program is free software, but with restricted license :
 *
 * - j-chkmail is distributed only to registered users
 * - j-chkmail license is available only non-commercial applications,
 *   this means, you can use j-chkmail if you make no profit with it.
 * - redistribution of j-chkmail in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about j-chkmail license can be found at j-chkmail
 * web site : http://j-chkmail.ensmp.fr
 */

#include <j-sys.h>
#include <j-chkmail.h>
#include <mod_sympa.h>
#include <modmac.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static mod_info_T   info = {
  "mod_sympa",
  "Jose-Marcio Martins da Cruz",
  "0.1.0 Alpha",
  0,
  0
};

#define MAX_DOMAINS     32

static char        *dConf = NULL;
static char        *dArray[MAX_DOMAINS];
static int          nDomains = 0;

static char        *uArray[] = { ".*-owner", "bounce-.*", NULL };

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_init(version, modsys)
     int                 version;
     mod_open_T         *modsys;
{
  /*
   ** Comment out useless callbacks
   ** Rarely a module will be used in all callbacks...
   */
  SET_BIT(info.calloffer, CALLBACK_CONNECT);
#if 1
  SET_BIT(info.calloffer, CALLBACK_EHLO);
  SET_BIT(info.calloffer, CALLBACK_MAIL);
  SET_BIT(info.calloffer, CALLBACK_RCPT);
  SET_BIT(info.calloffer, CALLBACK_DATA);
  SET_BIT(info.calloffer, CALLBACK_HEADER);
  SET_BIT(info.calloffer, CALLBACK_EOH);
  SET_BIT(info.calloffer, CALLBACK_BODY);
  SET_BIT(info.calloffer, CALLBACK_EOM);
  SET_BIT(info.calloffer, CALLBACK_ABORT);
  SET_BIT(info.calloffer, CALLBACK_CLOSE);
#endif

  modsys->calloffer = info.calloffer;
  info.callrequest = modsys->callrequest;

  /* check version */
  if ((version & 0xFFFF00) != (MOD_VERSION & 0xFFFF00))
  {
    syslog(LOG_WARNING, "API Version mismatch : %08X %08X\n", version,
           MOD_VERSION);
    return FALSE;
  }

  syslog(LOG_INFO, "NAME     : %s\n", info.name);
  syslog(LOG_INFO, "AUTHOR   : %s\n", info.author);
  syslog(LOG_INFO, "VERSION  : %s\n", info.version);
  syslog(LOG_INFO, "VERSION  : %08X\n", version);
  syslog(LOG_INFO, "ARGS     : %s\n", STRNULL(modsys->args, "null"));

  syslog(LOG_INFO, "CALL OFF : %08X\n", modsys->calloffer);
  syslog(LOG_INFO, "CALL REQ : %08X\n", modsys->callrequest);

  memset(dArray, 0, sizeof (dArray));
  dConf = strdup(STRNULL(modsys->args, ""));

  if (dConf != NULL)
  {
    int                 i;

    nDomains = str2tokens(dConf, MAX_DOMAINS, dArray, ",");
    for (i = 0; i < nDomains; i++)
      syslog(LOG_INFO, "  * mod_sympa : handling domain %s", dArray[i]);
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_info(infop)
     mod_info_T         *infop;
{
  *infop = info;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_call(callback, step, arg)
     int                callback;
     int                step;
     mod_ctx_T          *arg;
{
  char               *code = "400";
  char               *xcode = "4.5.0";
  char               *reply = "Access denied";

  if (arg == NULL)
    goto fin;

  arg->result = MODR_CONTINUE;

  syslog(LOG_INFO, "CALLBACK=%-10s : checking %-10s %-16s %s %s %s\n",
         CALLBACK_LABEL(callback),
         STRNULL(arg->id, "(null)"), STRNULL(arg->claddr, "(null)"),
         STRNULL(arg->clname, "(null)"), STRNULL(arg->from, "(null)"),
         STRNULL(arg->rcpt, "(null)"));

  if (callback == CALLBACK_RCPT)
  {
    char                rcpt[256];
    char               *user, *domain;

    char              **pu, **pd;

    user = domain = NULL;

    memset(rcpt, 0, sizeof (rcpt));
    extract_email_address(rcpt, arg->rcpt, sizeof (rcpt));

    user = rcpt;
    domain = strchr(rcpt, '@');
    if (domain != NULL)
      *domain++ = '\0';
    domain = STRNULL(domain, "(null)");

#if 0
    syslog(LOG_INFO, "mod_sympa : USER=%s DOMAIN=%s", user, domain);
#endif

    for (pu = uArray; *pu != NULL; pu++)
      if (strexpr(user, *pu, NULL, NULL, TRUE))
        break;

    if (*pu != NULL)
    {
      for (pd = dArray; *pd != NULL; pd++)
        if (strcasecmp(domain, *pd) == 0)
        {
	  syslog(LOG_INFO, "mod_sympa : USER=%s DOMAIN=%s MATCH !", user, domain);
          arg->flags = MODF_STOP_CHECKS;
          break;
        }
    }
  }

fin:
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_service(why)
     int                 why;
{
  syslog(LOG_INFO, "  * Servicing module %s", info.name);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_close()
{
  syslog(LOG_INFO, "* Closing module %s", info.name);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

char               *
extract_email_address(dst, org, sz)
     char               *dst;
     char               *org;
     size_t              sz;
{
  char               *expr = "<.*@.*>";
  long                pi, pf;

  expr = "<.*>";

  if (dst != NULL && sz > 0)
    memset(dst, 0, sz);

  if ((dst == NULL) || (org == NULL) || (sz == 0))
    return dst;

  pi = pf = 0;

  if (strexpr(org, expr, &pi, &pf, TRUE))
  {
    if (pf - pi - 1 <= sz)
      sz = pf - pi - 1;

    strlcpy(dst, org + pi + 1, sz);
  } else
  {
    int                 l = strlen(org);

    memset(dst, 0, sz);
    if (l >= sz)
      l = sz - 1;
    strncpy(dst, org, l);
  }
  strtolower(dst);

  return dst;
}

char               *
strtolower(char *s)
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = tolower(*p);
  return s;
}

bool
strexpr(s, expr, pi, pf, icase)
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
  if ((rerror = regcomp(&re, expr, flags)) == 0)
  {
    regmatch_t          pm;

    if (regexec(&re, s, 1, &pm, 0) == 0)
    {
      if (pi != NULL)
        *pi = pm.rm_so;
      if (pf != NULL)
        *pf = pm.rm_eo;
      found = TRUE;
    }
    regfree(&re);
  } else
  {
    char                s[256];

    if (regerror(rerror, &re, s, sizeof (s)) > 0)
      syslog(LOG_ERR, "regcomp(%s) error : %s", expr, s);
  }

  return found;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
str2tokens(s, sz, argv, sep)
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

  for (p = strtok_r(s, sep, &ptr), i = 0;
       p != NULL && i < sz - 1; p = strtok_r(NULL, sep, &ptr), i++)
  {
    argv[i] = p;
  }
  argv[i] = NULL;

  return i;
}
