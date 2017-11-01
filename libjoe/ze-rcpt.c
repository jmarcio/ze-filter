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

#include <ze-sys.h>

#include "ze-filter.h"

#define DBG_LEVEL     12

#define CHK_DOMAIN_OK         0 /* Accept all rcpts for this domain */
#define CHK_DOMAIN_YES        1 /* Check only full addresses and domain part */
#define CHK_DOMAIN_LOCAL      2 /* Check full addresses, domain and rcpt part */
#define CHK_DOMAIN_REJECT     3 /* Rejects all rcpts for this domain */
#define CHK_DOMAIN_TEMPFAIL   4 /* Rejects all rcpts for this domain */
#define CHK_DOMAIN_SPAMTRAP   5 /* This domain is a spam trap */


#define CHK_RCPT_OK           0 /* Recipient OK */
#define CHK_RCPT_REJECT       1 /* Access denied */
#define CHK_RCPT_SPAMTRAP     2 /* spam Trap */
#define CHK_RCPT_IGNORE       3
#define CHK_RCPT_TEMPFAIL     4 /* Access denied */
#define CHK_RCPT_NET_LOCAL    5 /* Accept only if coming from the local network */
#define CHK_RCPT_NET_DOMAIN   6 /* Accept only if coming from the domain network */
#define CHK_RCPT_NET_FRIEND   7 /* Accept only if coming from some friend network */
#define CHK_RCPT_NET_KNOWN    8
#define CHK_RCPT_USER_UNKNOWN 9

#define CHK_RCPT_UNDEF       -1


static int          decode_domain_check(char *);
static int          chk_rcpt_decode(char *);

static int          check_rcpt_net_access(int, char *, char *, int);


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
/*
** Domain
** - No record 
** - YES             - Check IT
** - NO              - Don't check IT
** - Local           - Also check user part
** - REJECT          - Reject all messages for this domain
**
** Rcpt
** - OK
** - REJECT
** - KNOWN
*/


/*
** Checks :
**
**
**
*/
#define RCPT_PREFIX        "RcptAccess"
#define DOMAIN_PREFIX      "CheckRcptDomain"

int
check_rcpt(email, ip, name, netclass)
     char               *email;
     char               *ip;
     char               *name;
     int                 netclass;
{
  char                buf[256];
  bool                found = FALSE;
  int                 result = RCPT_OK;
  char               *kbuf = NULL;
  char               *kemail = NULL, *kbemail = NULL;
  size_t              size = 0;
  char               *rcpt, *domain;
  char               *p;

  int                 access = CHK_DOMAIN_OK;

  ASSERT(email != NULL && strlen(email) > 0);

  MESSAGE_INFO(DBG_LEVEL, "Checking %s:%s", RCPT_PREFIX, email);

  /* Create a copy with rcpt/domain parts */
  if ((kemail = strdup(email)) == NULL)
  {
    LOG_SYS_ERROR("strdup(%s) error :", email);
    goto fin;
  }

  /* remove detail part of address */
  {
    char               *p, *q;
    bool                ok = TRUE;

    for (p = email, q = kemail; *p != '\0'; p++)
    {
      if (ok && *p == '+')
      {
        ok = FALSE;
        continue;
      }
      if (!ok && *p == '@')
        ok = TRUE;
      if (!ok)
        continue;
      *q++ = *p;
    }
    *q = '\0';
  }

  MESSAGE_INFO(DBG_LEVEL, "Checking %s:%s", RCPT_PREFIX, kemail);

  /* Create a copy with rcpt/domain parts */
  if ((kbemail = strdup(kemail)) == NULL)
  {
    LOG_SYS_ERROR("strdup(%s) error :", kemail);
    goto fin;
  }

  rcpt = domain = NULL;
  p = strchr(kbemail, '@');
  if (p != NULL)
  {
    *p++ = '\0';
    domain = p;
    rcpt = kbemail;
  } else
  {
    rcpt = NULL;
    domain = kbemail;
  }

  MESSAGE_INFO(DBG_LEVEL, "KEY = %s, RCPT = %s, DOMAIN = %s",
               kemail, STRNULL(rcpt, "(null)"), STRNULL(domain, "(null)"));

  ASSERT(domain != NULL);

  /* 
   ** Shall check recipient access for this domain ? 
   ** May return
   ** - Not found       - ACCESS OK 
   ** - NO              - ACCESS OK
   ** - YES             - Check it (full address only)
   ** - LOCAL           - Check it (full address and user part only)
   ** - REJECT          - ACCESS DENIED
   */
  found = FALSE;
  memset(buf, 0, sizeof (buf));
  if (db_rcpt_check_domain(DOMAIN_PREFIX, domain, buf, sizeof (buf), 0))
  {
    MESSAGE_INFO(DBG_LEVEL, " -> Got : %s:%-15s %s\n", "CheckDomainRcpt",
                 domain, buf);

    found = TRUE;
    /* what more ??? */
  }
#if 0
  if (!found)
  {
    /* XXX */
    if (db_rcpt_check_domain(DOMAIN_PREFIX, "default", buf, sizeof (buf), 0))
    {
      MESSAGE_INFO(DBG_LEVEL, " -> Got : %s:%-15s %s\n", "CheckDomainRcpt",
                   "default", "buf");

      found = TRUE;
      /* what more ??? */
    }
  }
#endif
  if (found)
  {
    /* decode the value found for this domain */
    access = decode_domain_check(buf);
    switch (access)
    {
      case CHK_DOMAIN_OK:
        result = RCPT_OK;
        goto fin;
        break;
      case CHK_DOMAIN_REJECT:
        result = RCPT_ACCESS_DENIED;
        goto fin;
        break;
      case CHK_DOMAIN_TEMPFAIL:
        result = RCPT_TEMPFAIL;
        goto fin;
        break;
      case CHK_DOMAIN_YES:
        break;
      case CHK_DOMAIN_LOCAL:
        break;
      case CHK_DOMAIN_SPAMTRAP:
        result = RCPT_SPAMTRAP;
        goto fin;
        break;
      default:
        goto fin;
        break;
    }
  } else
    goto fin;

  MESSAGE_INFO(DBG_LEVEL, "CheckRcptDomain access : %d", access);

  /*
   ** Now let's check full recipient address
   **
   */

  found = false;
  memset(buf, 0, sizeof (buf));
  /* first of all, let's check full address */
  if (db_rcpt_check_email(RCPT_PREFIX, kemail, buf, sizeof (buf)))
  {
    if (strlen(buf) > 0)
      MESSAGE_INFO(DBG_LEVEL, " -> Got : %s:%-15s %s\n", RCPT_PREFIX, kemail,
                   buf);

    found = TRUE;

    goto docheck;
  }

  /* a key build buffer */
  size = strlen(kemail) + 16;
  if ((kbuf = malloc(size)) == NULL)
  {
    LOG_SYS_ERROR("malloc(%d) error :", size);
    goto fin;
  }

  /* 
   ** Let's check rcpt part
   **
   ** This shall be done only for local domains (class W)
   */

  if (access == CHK_DOMAIN_LOCAL)
  {
    MESSAGE_INFO(DBG_LEVEL, "Checking local part : %s",
                 STRNULL(rcpt, "(null)"));
    /* let's check only rcpt part */
    if (rcpt != NULL && strlen(rcpt) > 0)
    {
      snprintf(kbuf, size, "%s@", rcpt);
      if (db_rcpt_check_email(RCPT_PREFIX, kbuf, buf, sizeof (buf)))
      {
        MESSAGE_INFO(DBG_LEVEL, " -> Got : %s:%-15s %s\n", RCPT_PREFIX, kbuf,
                     buf);
        if (strlen(buf) > 0)
        {
          found = TRUE;
          goto docheck;
        }
      }

      /* remove detail part, if there is one */
      if ((p = strchr(rcpt, '+')) != NULL)
      {
        *p = '\0';
        snprintf(kbuf, size, "%s@", rcpt);
        if (db_rcpt_check_email(RCPT_PREFIX, kbuf, buf, sizeof (buf)))
        {
          MESSAGE_INFO(DBG_LEVEL, " -> Got : %s:%-15s %s\n", RCPT_PREFIX, kbuf,
                       buf);
          if (strlen(buf) > 0)
          {
            found = TRUE;
            goto docheck;
          }
        }
      }
    }
  }

  /* let's now check default behaviour for this domain */
  if (domain != NULL)
  {
    p = domain;
    while (p != NULL && strlen(p) > 0)
    {
      snprintf(kbuf, size, "%s", p);
      if (db_rcpt_check_domain(RCPT_PREFIX, kbuf, buf, sizeof (buf), 0))
      {
        MESSAGE_INFO(DBG_LEVEL, " -> Got : %s:%-15s %s\n", RCPT_PREFIX, kbuf,
                     buf);
        if (strlen(buf) > 0)
        {
          found = TRUE;
          goto docheck;
        }
      }
      if ((p = strchr(p, '.')) != NULL)
        p++;

      /* don't check upper level domains */
#if 1
      break;
#endif
    }
  }

  result = RCPT_USER_UNKNOWN;

  goto docheck;

docheck:
  /*
   ** Restrict recipients :
   ** - All domains
   ** - Some domains
   ** - Local domains
   */

  /*
   ** Check if
   ** - rcpt exists
   ** - rcpt accepts messages or is a service only rcpt
   ** - rcpt mailbox access is restricted to some domains
   */
  /*
   ** #define RCPT_OK            0
   ** #define RCPT_ACCESS_DENIED        1
   ** #define RCPT_ACCESS_LOCAL_ONLY    2
   ** #define RCPT_USER_UNKNOWN         3
   */

  if (!found)
  {
    result = RCPT_USER_UNKNOWN;
    goto fin;
  }

  result = chk_rcpt_decode(buf);
  switch (result)
  {
    case CHK_RCPT_OK:
      result = RCPT_OK;
      break;
    case CHK_RCPT_NET_LOCAL:
    case CHK_RCPT_NET_DOMAIN:
    case CHK_RCPT_NET_FRIEND:
    case CHK_RCPT_NET_KNOWN:
      result = check_rcpt_net_access(result, ip, name, netclass);
      break;
    case CHK_RCPT_REJECT:
      result = RCPT_ACCESS_DENIED;
      break;
    case CHK_RCPT_TEMPFAIL:
      result = RCPT_TEMPFAIL;
      break;
    case CHK_RCPT_SPAMTRAP:
      result = RCPT_SPAMTRAP;
      break;
    case CHK_RCPT_IGNORE:
      result = RCPT_IGNORE;
      break;
    case CHK_RCPT_USER_UNKNOWN:
      result = RCPT_USER_UNKNOWN;
      break;
    default:
      MESSAGE_NOTICE(9, "Error ...");
      result = RCPT_USER_UNKNOWN;
      break;
  }

fin:
  FREE(kemail);
  FREE(kbemail);
  FREE(kbuf);

  return result;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
decode_domain_check(s)
     char               *s;
{
  if (s == NULL)
    return CHK_DOMAIN_OK;

  if (strcasecmp(s, "NO") == 0)
    return CHK_DOMAIN_OK;

  if (strcasecmp(s, "YES") == 0)
    return CHK_DOMAIN_YES;

  if (strcasecmp(s, "REJECT") == 0)
    return CHK_DOMAIN_REJECT;

  if (strcasecmp(s, "TEMPFAIL") == 0)
    return CHK_DOMAIN_TEMPFAIL;

  if (strcasecmp(s, "SPAMTRAP") == 0)
    return CHK_DOMAIN_SPAMTRAP;

  if (strcasecmp(s, "LOCAL") == 0)
    return CHK_DOMAIN_LOCAL;

  return CHK_DOMAIN_OK;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
chk_rcpt_decode(code)
     char               *code;
{
  if (code == NULL || strlen(code) == 0)
    return CHK_RCPT_UNDEF;

  if (strcasecmp(code, "OK") == 0)
    return RCPT_OK;

  if (strcasecmp(code, "REJECT") == 0)
    return CHK_RCPT_REJECT;

  if (strcasecmp(code, "TEMPFAIL") == 0)
    return CHK_RCPT_TEMPFAIL;

  if (strcasecmp(code, "SPAMTRAP") == 0)
    return CHK_RCPT_SPAMTRAP;

  if (strcasecmp(code, "IGNORE") == 0)
    return CHK_RCPT_IGNORE;

  if (strcasecmp(code, "LOCAL-NET") == 0)
    return CHK_RCPT_NET_LOCAL;

  if (strcasecmp(code, "DOMAIN-NET") == 0)
    return CHK_RCPT_NET_DOMAIN;

  if (strcasecmp(code, "FRIEND-NET") == 0)
    return CHK_RCPT_NET_FRIEND;

  if (strcasecmp(code, "KNOWN-NET") == 0)
    return CHK_RCPT_NET_KNOWN;

  if (strcasecmp(code, "USER-UNKNOWN") == 0)
    return CHK_RCPT_USER_UNKNOWN;

  return CHK_RCPT_UNDEF;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define CHECK_NET_CLASS(check, netclass)		\
  if (netclass != NET_UNKNOWN) {			\
    bool classok = FALSE;				\
							\
    classok = IS_LOCAL(netclass) || IS_AUTH(netclass);	\
    if (classok && check == CHK_RCPT_NET_LOCAL)		\
      return RCPT_OK;					\
							\
    classok = classok || IS_DOMAIN(netclass);		\
    if (classok && check == CHK_RCPT_NET_DOMAIN)	\
      return RCPT_OK;					\
							\
    classok = classok || IS_FRIEND(netclass);		\
    if (classok && check == CHK_RCPT_NET_FRIEND)	\
      return RCPT_OK;					\
							\
    classok = classok || IS_KNOWN(netclass);		\
    if (classok && check == CHK_RCPT_NET_KNOWN)		\
      return RCPT_OK;					\
  }

static int
check_rcpt_net_access(check, ip, name, netclass)
     int                 check;
     char               *ip;
     char               *name;
     int                 netclass;
{
  CHECK_NET_CLASS(check, netclass);

  netclass = GetClientNetClass(ip, name, NULL, NULL, 0);

  CHECK_NET_CLASS(check, netclass);

  return RCPT_BAD_NETWORK;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

char               *
rcpt_code_string(code)
     int                 code;
{
  static name2id_T    names[] = {
    {"OK", RCPT_OK},
    {"REJECT", RCPT_REJECT},
    {"TEMPFAIL", RCPT_TEMPFAIL},
    {"Access Denied", RCPT_ACCESS_DENIED},
    {"Bad Network", RCPT_BAD_NETWORK},
    {"User Unknown", RCPT_USER_UNKNOWN},
    {"SpamTrap", RCPT_SPAMTRAP},
    {NULL, -1}
  };

  return get_name_by_id(names, code);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
rcpt_init()
{
  return db_rcpt_open(TRUE);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
rcpt_close()
{
  return db_rcpt_close();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
rcpt_reopen()
{
  return db_rcpt_reopen();
}

