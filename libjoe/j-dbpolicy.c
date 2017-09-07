/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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
 * web site : http://foss.jose-marcio.org
 */

#include <j-sys.h>

#include "j-chkmail.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define               DREF     32


static JDB_T        hdb = JDB_INITIALIZER;
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
    cfdir = J_CDBDIR;

  dbname = cf_get_str(CF_DB_POLICY);
  ADJUST_FILENAME(dbpath, dbname, cfdir, "j-policy.db");

  MESSAGE_INFO(15, "Opening Policy Database : %s", dbpath);

  if (jdb_ok(&hdb))
    return TRUE;

  jdb_lock(&hdb);
  rdonly = rd;
  if (!jdb_ok(&hdb))
    res = jdb_open(&hdb, NULL, dbpath, (rdonly ? 0444 : 0644), rdonly, TRUE, 0);
  jdb_unlock(&hdb);

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

  jdb_lock(&hdb);

  if (jdb_ok(&hdb))
    res = jdb_close(&hdb);

  snprintf(path, sizeof (path), "%s/%s", cf_get_str(CF_CDBDIR), "j-policy.db");

  res = jdb_open(&hdb, NULL, path, (rdonly ? 0444 : 0644), rdonly, TRUE, 0);
  jdb_unlock(&hdb);

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

  if (!jdb_ok(&hdb))
    return res;

  jdb_lock(&hdb);
  if (jdb_ok(&hdb))
    res = jdb_close(&hdb);
  jdb_unlock(&hdb);

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

  if (!jdb_ok(&hdb) && !db_policy_open(TRUE))
  {
    if (nerr++ < MAX_ERR)
      LOG_MSG_ERROR("Can't open policy database");
    return FALSE;
  }

  if (key == NULL)
    return FALSE;

  if (strlen(key) == 0)
    key = "default";

  nerr = 0;
  jdb_lock(&hdb);

  /* let's get the domain part and check if this is an email
   ** address
   */
  if ((domain = strchr(key, '@')) != NULL)
  {
    is_email = TRUE;

    if ((email = strdup(key)) != NULL)
      (void) extract_email_address(email, key, strlen(key) + 1);
    else
      LOG_SYS_ERROR("strdup(%s) error", key);

    domain++;
  } else
    domain = key;

  /* if this is an email address, let's first check
   ** the entire key
   */
  if (is_email)
  {
    /* First of all, let's check the entire key */
    snprintf(k, sizeof (k), "%s:%s", prefix, email);
    (void) strtolower(k);
    MESSAGE_INFO(DBG_LEVEL, "KEY FULL : Looking for %s ...", k);
    if (jdb_get_rec(&hdb, k, v, sizeof (v)))
    {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
      found = TRUE;
      goto fin;
    }

    if (!found)
    {

    }
  }

  if (found)
    goto fin;

  MESSAGE_INFO(DBG_LEVEL, "db_policy : domain = %s", domain);

  /* Entire key doesn't match - lets check domain part */
  if (strexpr(domain, IPV4_ADDR_REGEX, NULL, NULL, TRUE))
  {
    /* This is a numeric IP address */

    snprintf(k, sizeof (k), "%s:%s", prefix, domain);
    (void) strtolower(k);
    p = k;
    /* Try each part beginning from the most complete one */
    while (strlen(k) > 0)
    {
      MESSAGE_INFO(DBG_LEVEL, "IP   : Looking for %s ...", k);
      if (jdb_get_rec(&hdb, k, v, sizeof (v)))
      {
        if ((bufout != NULL) && (size > 0))
          strlcpy(bufout, v, size);
        MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
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

  if (strexpr(domain, IPV6_ADDR_REGEX, NULL, NULL, TRUE))
  {
    ipv6_T              ipv6;
    char                buf[256];

    snprintf(k, sizeof (k), "%s:%s", prefix, domain);
    MESSAGE_INFO(DBG_LEVEL, "IP   : Looking for %s ...", k);
    if (jdb_get_rec(&hdb, k, v, sizeof (v)))
    {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
      found = TRUE;
      goto host_check_ok;
    }

    if (ipv6_str2rec(&ipv6, domain))
    {
      int                 hi, lo, step, i;

      hi = IPV6_PREFIX_MAX;
      lo = IPV6_PREFIX_MIN;
      step = IPV6_PREFIX_STEP;

      ipv6_rec2str(buf, &ipv6, sizeof (buf));
      snprintf(k, sizeof (k), "%s:%s", prefix, buf);
      MESSAGE_INFO(DBG_LEVEL, "IP   : Looking for %s ...", k);
      if (jdb_get_rec(&hdb, k, v, sizeof (v)))
      {
        if ((bufout != NULL) && (size > 0))
          strlcpy(bufout, v, size);
        MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
        found = TRUE;
        goto host_check_ok;
      }

      for (i = IPV6_PREFIX_MAX; i >= IPV6_PREFIX_MIN; i -= IPV6_PREFIX_STEP)
      {
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
        MESSAGE_INFO(DBG_LEVEL, "IP   : Looking for %s ...", k);
        if (jdb_get_rec(&hdb, k, v, sizeof (v)))
        {
          if ((bufout != NULL) && (size > 0))
            strlcpy(bufout, v, size);
          MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
          found = TRUE;
          goto host_check_ok;
        }
      }
    }

    goto host_check_ok;
  }

  {
    /* This is a domain name */
    /* Try each part beginning from the most complete one */
    p = domain;
    while (p != NULL && strlen(p) > 0)
    {
      snprintf(k, sizeof (k), "%s:%s", prefix, p);
      (void) strtolower(k);
      MESSAGE_INFO(DBG_LEVEL, "NAME : Looking for %s", k);
      if (jdb_get_rec(&hdb, k, v, sizeof (v)))
      {
        if ((bufout != NULL) && (size > 0))
          strlcpy(bufout, v, size);
        MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
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

  /* Entire key doesn't match - lets check user part */
  if (is_email)
  {
    char               *p;

    snprintf(k, sizeof (k), "%s:%s", prefix, email);
    (void) strtolower(k);
    p = strchr(k, '@');
    if (p != NULL)
      *(++p) = '\0';

    MESSAGE_INFO(DBG_LEVEL, "k = %s", k);

    if (jdb_get_rec(&hdb, k, v, sizeof (v)))
    {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);

      found = TRUE;
      goto fin;
    }
  }

fin:
  FREE(email);
  jdb_unlock(&hdb);

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

  if (!jdb_ok(&hdb) && !db_policy_open(TRUE))
  {
    if (nerr++ < MAX_ERR)
      LOG_MSG_ERROR("Can't open policy database");
    return FALSE;
  }

  if (key == NULL)
    return FALSE;

  if (key == NULL || strlen(key) == 0)
    key = "default";

  
  jdb_lock(&hdb);
  nerr = 0;
  /* First of all, let's check the entire key */
  snprintf(k, sizeof (k), "%s:%s", prefix, key);
  (void) strtolower(k);
  MESSAGE_INFO(DBG_LEVEL, "KEY FULL : Looking for %s ...", k);
  if (jdb_get_rec(&hdb, k, v, sizeof (v)))
  {
    if ((bufout != NULL) && (size > 0))
      strlcpy(bufout, v, size);
    MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
    found = TRUE;
    goto fin;
  }

fin:
  jdb_unlock(&hdb);

  if (getenv("SHOWLOOKUP") != NULL)
    MESSAGE_INFO(0, "%-40s %s", k, found ? bufout : ""); 

  return found;
}

