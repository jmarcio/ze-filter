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
 *  Creation     : Tue Jan 24 21:29:12 CET 2006
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
#include <ze-filter.h>
#include <ze-dbrcpt.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define               DREF     32


static JDB_T   hdb = JDB_INITIALIZER;
static bool    rdonly = TRUE;

#define DBG_LEVEL    12

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_rcpt_open(rd)
     bool           rd;
{
  bool           res = TRUE;

  char          *dbname;
  char           dbpath[1024];
  char          *cfdir = NULL;

  memset(dbpath, 0, sizeof (dbpath));

  cfdir = cf_get_str(CF_CDBDIR);
  if (cfdir == NULL || strlen(cfdir) == 0)
    cfdir = ZE_CDBDIR;

  dbname = cf_get_str(CF_DB_RCPT);
  ADJUST_FILENAME(dbpath, dbname, cfdir, "ze-rcpt.db");

  MESSAGE_INFO(15, "Opening Rcpt Database : %s", dbpath);
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
db_rcpt_reopen()
{
  bool           res = TRUE;
  char           path[1024];

  jdb_lock(&hdb);
  if (jdb_ok(&hdb))
    res = jdb_close(&hdb);
  snprintf(path, sizeof (path), "%s/%s", cf_get_str(CF_CDBDIR), "ze-rcpt.db");
  res = jdb_open(&hdb, NULL, path, (rdonly ? 0444 : 0644), rdonly, TRUE, 0);
  jdb_unlock(&hdb);
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_rcpt_close()
{
  bool           res = TRUE;

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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
/*
** Checks order
**
** 1. Full check of key
** 2. If key is an IP address,
**    Check IP and IP nets
** 3. If key is a domain name
**    Check domain and sous domains
** 4. If key is an email address
**    check rcpt part
**
*/

bool
db_rcpt_check_email(prefix, key, bufout, size)
     char          *prefix;
     char          *key;
     char          *bufout;
     size_t         size;
{
  char           k[256];
  char           v[1024];
  bool           found = FALSE;
  char          *p = NULL;
  char          *domain = NULL;
  static int     nerr = 0;
  bool           is_email = FALSE;
  char          *email = NULL;

  if (!jdb_ok(&hdb) && !db_rcpt_open(TRUE))
  {
    if (nerr++ < MAX_ERR)
      LOG_MSG_ERROR("Can't open rcpt database");
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
  if (is_email && email != NULL && strlen(email) > 0)
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
  }

  /* if this is only the domain part */
  if (!is_email && strlen(domain) > 0)
  {
    /* First of all, let's check the entire key */
    snprintf(k, sizeof (k), "%s:%s", prefix, domain);
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
  }

#if 0
  /* 
   ** Entire key doesn't match - lets check domain part
   **
   **  Do I really need this ?????
   ** Now done at ze-rcpt.c
   */
  if (FALSE && !found)
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
#if 1
      /* don't check upper levels */
      break;
#endif
    }

    /* Is this really needed ??? */
    if (found)
    {

    }

    if (found)
      goto fin;
  }

#if 0
  /* Entire key doesn't match - lets check rcpt part */
  if (!found && is_email && email != NULL)
  {
    snprintf(k, sizeof (k), "%s:%s", prefix, email);
    (void) strtolower(k);
    domain = strchr(k, '@');
    if (domain != NULL)
      *(++domain) = '\0';
    MESSAGE_INFO(DBG_LEVEL, "k = %s", k);
    if (jdb_get_rec(&hdb, k, v, sizeof (v)))
    {
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      found = TRUE;
      goto fin;
    }
  }
#endif

#endif

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
db_rcpt_check_domain(prefix, key, bufout, size, flags)
     char          *prefix;
     char          *key;
     char          *bufout;
     size_t         size;
     uint32_t       flags;
{
  char           k[256];
  char           v[1024];
  bool           found = FALSE;
  char          *p = NULL;
  char          *domain = NULL;
  static int     nerr = 0;
  int            level = 0;

  if (!jdb_ok(&hdb) && !db_rcpt_open(TRUE))
  {
    if (nerr++ < MAX_ERR)
      LOG_MSG_ERROR("Can't open rcpt database");
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
    domain++;
  else
    domain = key;
  if (domain == NULL || strlen(domain) == 0)
    domain = "default";
  p = domain;
  level = 0;
  if (p != NULL && strlen(p) > 0)
  {
    snprintf(k, sizeof (k), "%s:%s", prefix, p);
    (void) strtolower(k);
    MESSAGE_INFO(DBG_LEVEL, "NAME : Looking for %s", k);
    if (jdb_get_rec(&hdb, k, v, sizeof (v)))
    {
      MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      found = TRUE;
    }
  }

  if (!found)
  {
    p = domain;
    level = 0;
    while (p != NULL && strlen(p) > 0)
    {
      snprintf(k, sizeof (k), "%s:*.%s", prefix, p);
      (void) strtolower(k);
      MESSAGE_INFO(DBG_LEVEL, "NAME : Looking for %s", k);
      if (jdb_get_rec(&hdb, k, v, sizeof (v)))
      {
	MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
	if ((bufout != NULL) && (size > 0))
	  strlcpy(bufout, v, size);
	found = TRUE;
	break;
      }
      p = strchr(p, '.');
      if (p != NULL)
	p++;
    }
  }

  /* not found - let's check if a default value is defined */
  if (!found)
  {
    snprintf(k, sizeof (k), "%s:%s", prefix, "default");
    (void) strtolower(k);
    MESSAGE_INFO(DBG_LEVEL, "NAME : Looking for %s", k);
    if (jdb_get_rec(&hdb, k, v, sizeof (v)))
    {
      MESSAGE_INFO(DBG_LEVEL, "         : Found %s %s...", k, v);
      if ((bufout != NULL) && (size > 0))
        strlcpy(bufout, v, size);
      found = TRUE;
    }
  }

fin:
  jdb_unlock(&hdb);
  return found;
}
