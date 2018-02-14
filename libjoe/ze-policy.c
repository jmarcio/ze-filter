
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

#include <ze-sys.h>
#include <libze.h>
#include "ze-filter.h"

#define JDEBUG 0

#define ACCESS_UNDEF       0
#define ACCESS_NO          1
#define ACCESS_QUICK_NO    2
#define ACCESS_YES         3
#define ACCESS_QUICK_YES   4

static int          access_decode(char *);

int                 policy_log_level = 9;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** policy_check vs policy_lookup
*/

bool
lookup_policy(prefix, key, buf, szbuf, chkdefault)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              szbuf;
     bool                chkdefault;
{
  char                bufout[256];
  bool                ok = FALSE;

  if (key == NULL) {
    if (chkdefault)
      key = "default";
    else
      return FALSE;
  }

  ZE_MessageInfo(15, "Checking %s:%s", prefix, key);

  memset(bufout, 0, sizeof (bufout));
  if (db_policy_check(prefix, key, bufout, sizeof (bufout))) {
    if (strlen(bufout) > 0)
      ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, key, bufout);
    strlcpy(buf, bufout, szbuf);

    ok = TRUE;

    goto fin;
  }

  if (chkdefault) {
    if (db_policy_check(prefix, "default", bufout, sizeof (bufout))) {
      if (strlen(buf) > 0)
        ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, "DEFAULT", buf);
      strlcpy(buf, bufout, szbuf);

      ok = TRUE;

      goto fin;
    }
    if (db_policy_check(prefix, "*", bufout, sizeof (bufout))) {
      if (strlen(buf) > 0)
        ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, "*", buf);
      strlcpy(buf, bufout, szbuf);

      ok = TRUE;

      goto fin;
    }
  }

fin:
  return ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** Checks :
**
**
**
*/

bool
check_policy(prefix, key, buf, szbuf, chkdefault)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              szbuf;
     bool                chkdefault;
{
  char                bufout[256];
  bool                ok = FALSE;

  if (key == NULL) {
    if (chkdefault)
      key = "";
    else
      return FALSE;
  }

  ZE_MessageInfo(15, "Checking %s:%s", prefix, key);

  memset(bufout, 0, sizeof (bufout));
  if (db_policy_check(prefix, key, bufout, sizeof (bufout))) {
    if (strlen(bufout) > 0)
      ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, key, bufout);
    strlcpy(buf, bufout, szbuf);

    ok = TRUE;

    goto fin;
  }

  if (zeStrRegex(key, IPV4_ADDR_REGEX, NULL, NULL, TRUE) ||
      zeStrRegex(key, IPV6_ADDR_REGEX, NULL, NULL, TRUE) ||
      zeStrRegex(key, DOMAINNAME_REGEX, NULL, NULL, TRUE)) {
    char                tb[256];

    if (ze_logLevel > 10) {
      if (zeStrRegex(key, IPV4_ADDR_REGEX, NULL, NULL, TRUE))
        ZE_MessageInfo(15, " -> IP     : %s", key);
      if (zeStrRegex(key, IPV6_ADDR_REGEX, NULL, NULL, TRUE))
        ZE_MessageInfo(15, " -> IP     : %s", key);
      if (zeStrRegex(key, DOMAINNAME_REGEX, NULL, NULL, TRUE))
        ZE_MessageInfo(15, " -> DOMAIN : %s", key);
    }

    if (db_policy_check("NetClass", key, tb, sizeof (tb))) {
      if (strlen(tb) > 0)
        ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", "NetClass", key, tb);
      if (db_policy_check(prefix, tb, bufout, sizeof (bufout))) {
        if (strlen(bufout) > 0)
          ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, tb, bufout);
        strlcpy(buf, bufout, szbuf);
        ok = TRUE;

        goto fin;
      }
    }
  }

  if (chkdefault) {
    if (db_policy_check(prefix, "default", bufout, sizeof (bufout))) {
      if (strlen(bufout) > 0)
        ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, "DEFAULT", buf);
      strlcpy(buf, bufout, szbuf);
      ok = TRUE;

      goto fin;
    }
    if (db_policy_check(prefix, "*", bufout, sizeof (bufout))) {
      if (strlen(bufout) > 0)
        ZE_MessageInfo(15, " -> Got : %s:%-15s %s\n", prefix, "*", buf);
      strlcpy(buf, bufout, szbuf);
      ok = TRUE;

      goto fin;
    }
  }

fin:
  return ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
check_host_policy(prefix, addr, name, class, buf, size, cdef)
     char               *prefix;
     char               *addr;
     char               *name;
     char               *class;
     char               *buf;
     size_t              size;
     bool                cdef;
{
  bool                ok = FALSE;
  char                lclass[128];

  memset(buf, 0, size);
  if (addr != NULL)
    ok = lookup_policy(prefix, addr, buf, size, FALSE);
  ZE_MessageInfo(11, "Prefix : %s; Addr  : %s; Buf : %s",
                 prefix, STRNULL(addr, "???"), buf);
  if (ok)
    goto fin;

  if (name != NULL)
    ok = lookup_policy(prefix, name, buf, size, FALSE);
  ZE_MessageInfo(11, "Prefix : %s; Name  : %s; Buf : %s",
                 prefix, STRNULL(name, "???"), buf);
  if (ok)
    goto fin;

  memset(lclass, 0, sizeof (lclass));
  if (class == NULL || strlen(class) == 0) {
    if (addr != NULL && strlen(addr) > 0)
      ok = lookup_policy("NetClass", addr, lclass, sizeof (lclass), FALSE);
    if (!ok && name != NULL && strlen(name) > 0)
      ok = lookup_policy("NetClass", name, lclass, sizeof (lclass), FALSE);
  } else
    strlcpy(lclass, class, sizeof (lclass));

  if (strlen(lclass) == 0)
    goto fin;

  ok = lookup_policy(prefix, lclass, buf, size, cdef);
  ZE_MessageInfo(11, "Prefix : %s; Class : %s; Buf : %s",
                 prefix, STRNULL(class, "???"), buf);

fin:
  return ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define MAX_TOKENS      256

#define DECODE_ACCESS(value, result)	     \
  do {					     \
    switch (access_decode(value)) {	     \
      case ACCESS_YES:			     \
        result = TRUE;			     \
        break;				     \
      case ACCESS_NO:			     \
        result = FALSE;			     \
        break;				     \
      case ACCESS_QUICK_YES:                 \
        return TRUE;                         \
        break;                               \
      case ACCESS_QUICK_NO:                  \
        return FALSE;                        \
        break;				     \
    }                                        \
} while (0)

bool
check_policy_tuple(prefix, ip, name, netclass, from, to, result)
     char               *prefix;
     char               *ip;
     char               *name;
     char               *netclass;
     char               *from;
     char               *to;
     bool                result;
{
  char                tag[256], value[256], key[256];
  char                send[256], rcpt[256];

  prefix = STRNULL(prefix, "");
  ip = STRNULL(ip, "");
  name = STRNULL(name, "");
  netclass = STRNULL(netclass, "UNKNOWN");
  from = STRNULL(from, "nullsender");
  to = STRNULL(to, "");

  memset(send, 0, sizeof (send));
  extract_email_address(send, from, sizeof (send));
  memset(rcpt, 0, sizeof (rcpt));
  extract_email_address(rcpt, to, sizeof (rcpt));

  from = send;
  to = rcpt;

  memset(tag, 0, sizeof (tag));
  memset(key, 0, sizeof (key));
  memset(value, 0, sizeof (value));

  ZE_MessageInfo(15, "TUPLE %s %s %s %s", ip, name, from, to);

  /*
   ** Checking CONNECT
   */
  snprintf(tag, sizeof (tag), "%sConnect", prefix);
  if (check_host_policy(tag, ip, name, netclass, value, sizeof (value), TRUE)) {
    switch (access_decode(value)) {
      case ACCESS_YES:
        result = TRUE;
        break;
      case ACCESS_NO:
        result = FALSE;
        break;
      case ACCESS_QUICK_YES:
        return TRUE;
        break;
      case ACCESS_QUICK_NO:
        return FALSE;
        break;
    }
  }

  if (from == NULL)
    goto fin;

  /*
   ** Checking FROM
   */
  {
    char               *token = NULL;

    token = cf_get_str(CF_FROM_PASS_TOKEN);
    if (token != NULL && strlen(token) > 0) {
      char               *argv[MAX_TOKENS];
      int                 argc;

      int                 i;
      char               *p;

      ZE_MessageInfo(11, " Checking FROM_TOKENS %s", token);

      snprintf(tag, sizeof (tag), "%sFrom", prefix);
      argc = zeStr2Tokens(token, MAX_TOKENS, argv, " ");
      for (i = 0; i < argc; i++) {
        ZE_MessageInfo(11, " Checking %s:%s", tag, argv[i]);
        if ((p = strstr(from, argv[i])) != NULL) {
          if (check_policy(tag, p, value, sizeof (value), FALSE)) {
            switch (access_decode(value)) {
              case ACCESS_YES:
                result = TRUE;
                break;
              case ACCESS_NO:
                result = FALSE;
                break;
              case ACCESS_QUICK_YES:
                return TRUE;
                break;
              case ACCESS_QUICK_NO:
                return FALSE;
                break;
            }
          }
        }
      }
    }
  }

  snprintf(tag, sizeof (tag), "%sFrom", prefix);
  if (check_policy(tag, from, value, sizeof (value), TRUE)) {
    switch (access_decode(value)) {
      case ACCESS_YES:
        result = TRUE;
        break;
      case ACCESS_NO:
        result = FALSE;
        break;
      case ACCESS_QUICK_YES:
        return TRUE;
        break;
      case ACCESS_QUICK_NO:
        return FALSE;
        break;
    }
  }

  if (to == NULL)
    goto fin;

  /*
   ** Checking TO
   */
  {
    char               *token = NULL;

    token = cf_get_str(CF_TO_PASS_TOKEN);
    if (token != NULL && strlen(token) > 0) {
      char               *argv[MAX_TOKENS];
      int                 argc;

      int                 i;
      char               *p;

      ZE_MessageInfo(11, " Checking TO_TOKENS %s", token);

      snprintf(tag, sizeof (tag), "%sTo", prefix);
      argc = zeStr2Tokens(token, MAX_TOKENS, argv, " ,");
      for (i = 0; i < argc; i++) {
        ZE_MessageInfo(11, " Checking %s:%s", tag, argv[i]);
        if ((p = strstr(from, argv[i])) != NULL) {
          if (check_policy(tag, p, value, sizeof (value), FALSE)) {
            switch (access_decode(value)) {
              case ACCESS_YES:
                result = TRUE;
                break;
              case ACCESS_NO:
                result = FALSE;
                break;
              case ACCESS_QUICK_YES:
                return TRUE;
                break;
              case ACCESS_QUICK_NO:
                return FALSE;
                break;
            }
          }
        }
      }
    }
  }

  snprintf(tag, sizeof (tag), "%sTo", prefix);
  ZE_MessageInfo(20, "TAG : %s %s", tag, to);
  if (check_policy(tag, to, value, sizeof (value), TRUE)) {
    switch (access_decode(value)) {
      case ACCESS_YES:
        result = TRUE;
        break;
      case ACCESS_NO:
        result = FALSE;
        break;
      case ACCESS_QUICK_YES:
        return TRUE;
        break;
      case ACCESS_QUICK_NO:
        return FALSE;
        break;
    }
  }

fin:
  ZE_MessageInfo(15, "TUPLE %s %s %s %s : %s", ip, name, send, rcpt,
                 STRBOOL(result, "YES", "NO"));
  return result;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
check_policy_all_rcpts(prefix, ip, name, netclass, from, rcpt, result, conflict)
     char               *prefix;
     char               *ip;
     char               *name;
     char               *netclass;
     char               *from;
     rcpt_addr_T        *rcpt;
     bool                result;
     int                 conflict;
{
  bool                doit = result;

  rcpt_addr_T        *p = NULL;
  int                 nok = 0, nko = 0;

  for (p = rcpt; p != NULL; p = p->next) {
    bool                ok;

    if (p->access != RCPT_OK)
      continue;

    ok = check_policy_tuple(prefix, ip, name, netclass, from, p->rcpt, result);
    if (ok == result)
      nok++;
    else
      nko++;
  }
  doit = (nok > 0) ? result : !result;
  if (nko > 0 && nok > 0) {
    switch (conflict) {
      case OPT_ONE_WIN:
        doit = (nko > 0) ? !result : result;
        break;
      case OPT_MAJORITY_WIN:
        doit = (nok > nko) ? result : !result;
        break;
      case OPT_DEFAULT:
      default:
        doit = result;
        break;
    }
  }
  return doit;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define       VAL2LIMIT(r,buffer)		\
  do {						\
    long v = 0;					\
						\
    v = zeStr2long(buffer, NULL, r);		\
    if (r <= 0)					\
      r = MAX(v,0);				\
    else					\
      r = (v > 0) ? MIN(v,r) : r;		\
  } while (0)

long
check_limit_tuple(prefix, ip, name, netclass, from, to, result)
     char               *prefix;
     char               *ip;
     char               *name;
     char               *netclass;
     char               *from;
     char               *to;
     long                result;
{
  char                tag[256], buffer[256], key[256];
  char                send[256], rcpt[256];

  prefix = STRNULL(prefix, "");
  ip = STRNULL(ip, "");
  name = STRNULL(name, "");
  netclass = STRNULL(netclass, "UNKNOWN");
  from = STRNULL(from, "nullsender");
  to = STRNULL(to, "");

  memset(send, 0, sizeof (send));
  extract_email_address(send, from, sizeof (send));
  memset(rcpt, 0, sizeof (rcpt));
  extract_email_address(rcpt, to, sizeof (rcpt));

  from = send;
  to = rcpt;

  memset(tag, 0, sizeof (tag));
  memset(key, 0, sizeof (key));
  memset(buffer, 0, sizeof (buffer));

  ZE_MessageInfo(15, "TUPLE %s %s %s %s", ip, name, from, to);

  result = MAX(result, 0);
  /*
   ** Checking CONNECT
   */

  snprintf(tag, sizeof (tag), "%sConnect", prefix);
  if (check_host_policy(tag, ip, name, netclass, buffer, sizeof (buffer), TRUE)) {
    VAL2LIMIT(result, buffer);
  }

  if (from == NULL)
    goto fin;

  /*
   ** Checking FROM
   */
  {
    char               *token = NULL;

    token = cf_get_str(CF_FROM_PASS_TOKEN);
    if (token != NULL && strlen(token) > 0) {
      char               *argv[MAX_TOKENS];
      int                 argc;

      int                 i;
      char               *p;

      ZE_MessageInfo(11, " Checking FROM_TOKENS %s", token);

      snprintf(tag, sizeof (tag), "%sFrom", prefix);
      argc = zeStr2Tokens(token, MAX_TOKENS, argv, " ");
      for (i = 0; i < argc; i++) {
        ZE_MessageInfo(11, " Checking %s:%s", tag, argv[i]);
        if ((p = strstr(from, argv[i])) != NULL) {
          if (check_policy(tag, p, buffer, sizeof (buffer), FALSE)) {
            VAL2LIMIT(result, buffer);
          }
        }
      }
    }
  }

  snprintf(tag, sizeof (tag), "%sFrom", prefix);
  if (check_policy(tag, from, buffer, sizeof (buffer), TRUE)) {
    VAL2LIMIT(result, buffer);
  }

  if (to == NULL)
    goto fin;

  /*
   ** Checking TO
   */
  {
    char               *token = NULL;

    token = cf_get_str(CF_TO_PASS_TOKEN);
    if (token != NULL && strlen(token) > 0) {
      char               *argv[MAX_TOKENS];
      int                 argc;

      int                 i;
      char               *p;

      ZE_MessageInfo(11, " Checking TO_TOKENS %s", token);

      snprintf(tag, sizeof (tag), "%sTo", prefix);
      argc = zeStr2Tokens(token, MAX_TOKENS, argv, " ,");
      for (i = 0; i < argc; i++) {
        ZE_MessageInfo(11, " Checking %s:%s", tag, argv[i]);
        if ((p = strstr(from, argv[i])) != NULL) {
          if (check_policy(tag, p, buffer, sizeof (buffer), FALSE)) {
            VAL2LIMIT(result, buffer);
          }
        }
      }
    }
  }

  snprintf(tag, sizeof (tag), "%sTo", prefix);
  ZE_MessageInfo(20, "TAG : %s %s", tag, to);
  if (check_policy(tag, to, buffer, sizeof (buffer), TRUE)) {
    VAL2LIMIT(result, buffer);
  }

fin:
  ZE_MessageInfo(20, "TUPLE %s %s %s %s : %ld", ip, name, send, rcpt, result);
  return result;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
long
check_limit_all_rcpts(prefix, ip, name, netclass, from, rcpt, defval)
     char               *prefix;
     char               *ip;
     char               *name;
     char               *netclass;
     char               *from;
     rcpt_addr_T        *rcpt;
     long                defval;
{
  long                result = defval;

  rcpt_addr_T        *p = NULL;

  for (p = rcpt; p != NULL; p = p->next) {
    long                v;

    v = check_limit_tuple(prefix, ip, name, netclass, from, p->rcpt, result);
    v = MAX(0, v);
    if (result <= 0) {
      result = v;
    } else {
      result = MIN(v, result);
    }
  }
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
policy_init()
{
  return db_policy_open(TRUE);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
policy_close()
{
  return db_policy_close();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
policy_reopen()
{
  return db_policy_reopen();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
policy_decode(code)
     char               *code;
{
  if (code == NULL || strlen(code) == 0)
    return JC_DEFAULT;

  if (strcasecmp(code, "DEFAULT") == 0)
    return JC_DEFAULT;

  if (strcasecmp(code, "REJECT") == 0)
    return JC_REJECT;

  if (strcasecmp(code, "OK") == 0)
    return JC_OK;

  return JC_DEFAULT;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
access_decode(s)
     char               *s;
{
  if (s == NULL)
    return ACCESS_UNDEF;

  if (strcasecmp(s, "NO") == 0)
    return ACCESS_NO;

  if (strcasecmp(s, "QUICKNO") == 0)
    return ACCESS_QUICK_NO;

  if (strcasecmp(s, "QUICK-NO") == 0)
    return ACCESS_QUICK_NO;

  if (strcasecmp(s, "NOQUICK") == 0)
    return ACCESS_QUICK_NO;

  if (strcasecmp(s, "NO-QUICK") == 0)
    return ACCESS_QUICK_NO;

  if (strcasecmp(s, "YES") == 0)
    return ACCESS_YES;

  if (strcasecmp(s, "QUICKYES") == 0)
    return ACCESS_QUICK_YES;

  if (strcasecmp(s, "QUICK-YES") == 0)
    return ACCESS_QUICK_YES;

  if (strcasecmp(s, "YESQUICK") == 0)
    return ACCESS_QUICK_YES;

  if (strcasecmp(s, "YES-QUICK") == 0)
    return ACCESS_QUICK_YES;

  return ACCESS_UNDEF;
}
