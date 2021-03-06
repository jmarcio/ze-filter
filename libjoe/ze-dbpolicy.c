
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

#include "ze-filter.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define               DREF     32


static ZEDB_T       hdb = ZEDB_INITIALIZER;
static bool         rdonly = TRUE;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_policy_open(rd)
     bool                rd;
{
  bool                res = TRUE;

  char               *dbname;
  char                dbpath[1024];
  char               *cfdir = NULL;

  memset(dbpath, 0, sizeof (dbpath));

  cfdir = cf_get_str(CF_CDBDIR);
  if (cfdir == NULL || strlen(cfdir) == 0)
    cfdir = ZE_CDBDIR;

  dbname = cf_get_str(CF_DB_POLICY);
  ADJUST_FILENAME(dbpath, dbname, cfdir, "ze-policy.db");

  ZE_MessageInfo(15, "Opening Policy Database : %s", dbpath);

  if (zeDb_OK(&hdb))
    return TRUE;

  zeDb_Lock(&hdb);
  rdonly = rd;
  if (!zeDb_OK(&hdb))
    res =
      zeDb_Open(&hdb, NULL, dbpath, (rdonly ? 0444 : 0644), rdonly, TRUE, 0);
  zeDb_Unlock(&hdb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_policy_reopen()
{
  bool                res = TRUE;
  char                path[1024];

  zeDb_Lock(&hdb);

  if (zeDb_OK(&hdb))
    res = zeDb_Close(&hdb);

  snprintf(path, sizeof (path), "%s/%s", cf_get_str(CF_CDBDIR), "ze-policy.db");

  res = zeDb_Open(&hdb, NULL, path, (rdonly ? 0444 : 0644), rdonly, TRUE, 0);
  zeDb_Unlock(&hdb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_policy_close()
{
  bool                res = TRUE;

  if (!zeDb_OK(&hdb))
    return res;

  zeDb_Lock(&hdb);
  if (zeDb_OK(&hdb))
    res = zeDb_Close(&hdb);
  zeDb_Unlock(&hdb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define MAX_ERR 8

/*
** Checks order
**
** 1. Full check of key
** 2. If key is an IP address,
**    Check IP and IP nets
** 3. If key is a domain name
**    Check domain and sous domains
** 4. If key is an email address
**    check user part
**
*/
#define DBG_LEVEL    15

bool
db_policy_check(prefix, key, bufout, size)
     char               *prefix;
     char               *key;
     char               *bufout;
     size_t              size;
{
  char                k[256];
  char                v[1024];
  bool                found = FALSE;
  char               *p = NULL;
  char               *domain = NULL;
  static int          nerr = 0;
  bool                is_email = FALSE;
  char               *email = NULL;

  if (!zeDb_OK(&hdb) && !db_policy_open(TRUE)) {
    if (nerr++ < MAX_ERR)
      ZE_LogMsgError(0, "Can't open policy database");
    return FALSE;
  }

  if (key == NULL)
    return FALSE;

  if (strlen(key) == 0)
    key = "default";

  nerr = 0;
  zeDb_Lock(&hdb);

  /*
   * let's get the domain part and check if this is an email
   * ** address
   */
  if ((domain = strchr(key, '@')) != NULL) {
    is_email = TRUE;

    if ((email = strdup(key)) != NULL)
      (void) extract_email_address(email, key, strlen(key) + 1);
    else
      ZE_LogSysError("strdup(%s) error", key);

    domain++;
  } else
    domain = key;

  /*
   * if this is an email address, let's first check
   * ** the entire key
   */
  if (is_email) {
    /*
     * First of all, let's check the entire key 
     */
    snprintf(k, sizeof (k), "%s:%s", prefix, email);
    (void) zeStr2Lower(k);
    ZE_MessageInfo(DBG_LEVEL, "KEY FULL : Looking for %s ...", k);
    if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
      found = TRUE;
      goto fin;
    }

    if (!found) {

    }
  }

  if (found)
    goto fin;

  ZE_MessageInfo(DBG_LEVEL, "db_policy : domain = %s", domain);

  /*
   * Entire key doesn't match - lets check domain part 
   */
  if (zeStrRegex(domain, IPV4_ADDR_REGEX, NULL, NULL, TRUE)) {
    /*
     * This is a numeric IP address 
     */

    snprintf(k, sizeof (k), "%s:%s", prefix, domain);
    (void) zeStr2Lower(k);
    p = k;
    /*
     * Try each part beginning from the most complete one 
     */
    while (strlen(k) > 0) {
      ZE_MessageInfo(DBG_LEVEL, "IP   : Looking for %s ...", k);
      if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
        if ((bufout != NULL) && (size > 0))
          strlcpy(bufout, v, size);
        ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
        found = TRUE;
        break;
      }
      if ((p = strrchr(k, '.')) != NULL)
        *p = '\0';
      else
        break;
    }
    goto host_check_ok;
  }

  if (zeStrRegex(domain, IPV6_ADDR_REGEX, NULL, NULL, TRUE)) {
    ipv6_T              ipv6;
    char                buf[256];

    snprintf(k, sizeof (k), "%s:%s", prefix, domain);
    ZE_MessageInfo(DBG_LEVEL, "IP   : Looking for %s ...", k);
    if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
      found = TRUE;
      goto host_check_ok;
    }

    if (ipv6_str2rec(&ipv6, domain)) {
      int                 hi, lo, step, i;

      hi = IPV6_PREFIX_MAX;
      lo = IPV6_PREFIX_MIN;
      step = IPV6_PREFIX_STEP;

      ipv6_rec2str(buf, &ipv6, sizeof (buf));
      snprintf(k, sizeof (k), "%s:%s", prefix, buf);
      ZE_MessageInfo(DBG_LEVEL, "IP   : Looking for %s ...", k);
      if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
        if ((bufout != NULL) && (size > 0))
          strlcpy(bufout, v, size);
        ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
        found = TRUE;
        goto host_check_ok;
      }

      for (i = IPV6_PREFIX_MAX; i >= IPV6_PREFIX_MIN; i -= IPV6_PREFIX_STEP) {
#if 1
        ipv6_prefix_str(&ipv6, buf, sizeof (buf), i);
#else
        ipv6_T              ip, net;

        ip = ipv6;
        ipv6_set_prefix(&ip, i);
        ipv6_subnet(&net, &ip);
        ipv6_rec2str(buf, &net, sizeof (buf));
#endif
        snprintf(k, sizeof (k), "%s:%s", prefix, buf);
        ZE_MessageInfo(DBG_LEVEL, "IP   : Looking for %s ...", k);
        if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
          if ((bufout != NULL) && (size > 0))
            strlcpy(bufout, v, size);
          ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
          found = TRUE;
          goto host_check_ok;
        }
      }
    }

    goto host_check_ok;
  }

  {
    /*
     * This is a domain name 
     */
    /*
     * Try each part beginning from the most complete one 
     */
    p = domain;
    while (p != NULL && strlen(p) > 0) {
      snprintf(k, sizeof (k), "%s:%s", prefix, p);
      (void) zeStr2Lower(k);
      ZE_MessageInfo(DBG_LEVEL, "NAME : Looking for %s", k);
      if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
        if ((bufout != NULL) && (size > 0))
          strlcpy(bufout, v, size);
        ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
        found = TRUE;
        break;
      }
      p = strchr(p, '.');
      if (p != NULL)
        p++;
    }
  }

host_check_ok:
  if (found)
    goto fin;

  /*
   * Entire key doesn't match - lets check user part 
   */
  if (is_email) {
    char               *p;

    snprintf(k, sizeof (k), "%s:%s", prefix, email);
    (void) zeStr2Lower(k);
    p = strchr(k, '@');
    if (p != NULL)
      *(++p) = '\0';

    ZE_MessageInfo(DBG_LEVEL, "k = %s", k);

    if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);

      found = TRUE;
      goto fin;
    }
  }

fin:
  FREE(email);
  zeDb_Unlock(&hdb);

  return found;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool
db_policy_lookup(prefix, key, bufout, size)
     char               *prefix;
     char               *key;
     char               *bufout;
     size_t              size;
{
  char                k[256];
  char                v[1024];
  bool                found = FALSE;
  static int          nerr = 0;

  if (!zeDb_OK(&hdb) && !db_policy_open(TRUE)) {
    if (nerr++ < MAX_ERR)
      ZE_LogMsgError(0, "Can't open policy database");
    return FALSE;
  }

  if (key == NULL)
    return FALSE;

  if (key == NULL || strlen(key) == 0)
    key = "default";


  zeDb_Lock(&hdb);
  nerr = 0;
  /*
   * First of all, let's check the entire key 
   */
  snprintf(k, sizeof (k), "%s:%s", prefix, key);
  (void) zeStr2Lower(k);
  ZE_MessageInfo(DBG_LEVEL, "KEY FULL : Looking for %s ...", k);
  if (zeDb_GetRec(&hdb, k, v, sizeof (v))) {
    if ((bufout != NULL) && (size > 0))
      strlcpy(bufout, v, size);
    ZE_MessageInfo(DBG_LEVEL, "         : Found %s %s...", k, v);
    found = TRUE;
    goto fin;
  }

fin:
  zeDb_Unlock(&hdb);

  if (getenv("SHOWLOOKUP") != NULL)
    ZE_MessageInfo(0, "%-40s %s", k, found ? bufout : "");

  return found;
}
