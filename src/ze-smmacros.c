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

#include "ze-filter.h"
#include "ze-filter-data.h"

#undef  DEBUG
#define DEBUG 0


/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
#if 0
typedef struct sm_mac_T sm_mac_T;

sm_mac_T           *sm_macro_new();
void                sm_macro_free(sm_mac_T *);

void                sm_macro_update(SMFICTX *, sm_mac_T *);
char               *sm_macro_get_str(sm_mac_T *, char *);
int                 sm_macro_get_int(sm_mac_T *, char *);
#endif

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
struct sm_mac_T
{
  char               *name;
  char               *value;
  char               *label;
};

static sm_mac_T     smmac[] = {
  {"_", NULL, "The validated sender address"},
  {"c", NULL, "The hop count (number of Received: lines"},
  {"g", NULL, "The sender address relative to the recipient"},
  {"i", NULL, "Queue ID"},
  {"j", NULL, "The official domain name for this site"},
  {"v", NULL, "The version number of the sendmail binary"},

  {"{auth_type}", NULL,
   "The mechanism used for SMTP auth (only set if successful"},
  {"{auth_authen}", NULL, ""},
  {"{auth_ssf}", NULL, ""},
  {"{auth_author}", NULL, ""},

  {"{daemon_addr}", NULL, ""},
  {"{daemon_family}", NULL, ""},
  {"{daemon_name}", NULL, ""},
  {"{daemon_port}", NULL, ""},

  {"{cert_issuer}", NULL, "Distinguished Name of the Certification Authority"},
  {"{cert_subject}", NULL, "Distinguished Name of the certificate"},
  {"{cipher_bits}", NULL, "Effective key length of the symmetric key"},
  {"{cipher}", NULL, "Cypher used for the connection"},
  {"{tls_version}", NULL, ""},
  {"{verify}", NULL, ""},

  {"{client_addr}", NULL, "IP address of SMTP client"},
  {"{client_name}", NULL, "Verified hostname of the client"},
  {"{client_ptr}", NULL, "The result of PTR lookup for the client IP address"},
  {"{client_resolve}", NULL, "Result of the resolve call for client_name"},

#if 1
  {"{if_addr}", NULL, ""},
  {"{if_name}", NULL, ""},
#endif

  {"{mail_addr}", NULL, ""},
  {"{mail_host}", NULL, ""},

  {"{rcpt_addr}", NULL, ""},
  {"{rcpt_host}", NULL, ""},
  {"{rcpt_mailer}", NULL, ""},

  {"{nrcpts}", NULL, ""},

  {"{msg_size}", NULL, ""},

  {NULL, NULL, NULL}
};

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
sm_mac_T           *
sm_macro_new()
{
  sm_mac_T           *p;

  ZE_MessageInfo(19, "sizeof smmac : %d", sizeof smmac);

  if ((p = malloc(sizeof smmac)) != NULL)
  {
    memcpy(p, smmac, sizeof (smmac));
  } else;

  return p;
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
void
sm_macro_free(sm)
     sm_mac_T           *sm;
{
  sm_mac_T           *p = sm;

  for (p = sm; p != NULL && p->name != NULL; p++)
    FREE(p->value);

  FREE(sm);
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
bool                log_sm_macros = FALSE;

void
sm_macro_update(ctx, sm)
     SMFICTX            *ctx;
     sm_mac_T           *sm;
{
  sm_mac_T           *p = sm;
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  char               *callback = NULL;

  callback = callback_name(priv->callback_id);
  callback = STRNULL(callback, "CALLBACK");

  for (p = sm; p != NULL && p->name != NULL; p++)
  {
    char               *s;

    if (strlen(p->name) == 0)
      continue;

#if 1
    FREE(p->value);
#endif
    if ((s = smfi_getsymval(ctx, p->name)) != NULL)
    {
      FREE(p->value);
      if (s != NULL)
      {
        p->value = strdup(s);
        if (p->value == NULL)
          ZE_LogSysError("strdup(%s)", s);
      }
    }

    if (log_sm_macros && p->value != NULL)
    {
      ZE_MessageInfo(9, "%s : %-9s : SM Macro %-15s %s", CONNID_STR(priv->id),
                   callback, p->name, STRNULL(p->value, "(null)"));
    }
  }
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
char               *
sm_macro_get_str(sm, name)
     sm_mac_T           *sm;
     char               *name;
{
  sm_mac_T           *p = sm;

  for (p = sm; p != NULL && p->name != NULL; p++)
  {
    if (strcasecmp(name, p->name) == 0)
      return p->value;
  }
  return NULL;
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
int
sm_macro_get_int(sm, name)
     sm_mac_T           *sm;
     char               *name;
{
  sm_mac_T           *p = sm;

  for (p = sm; p != NULL && p->name != NULL; p++)
  {
    if (strcasecmp(name, p->name) == 0)
    {
      if (p->value != NULL)
        return atoi(p->value);
    }
  }
  return 0;
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
void
sm_macro_log_all(id, sm)
     char               *id;
     sm_mac_T           *sm;
{
  sm_mac_T           *p = sm;

  id = STRNULL(id, "NOID");

  for (p = sm; p != NULL && p->name != NULL; p++)
  {
    if (strlen(p->name) > 0)
      ZE_MessageInfo(9, "%s : %-15s - %s", id, p->name,
                   STRNULL(p->value, "(null)"));
  }
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/


static name2id_T    callbacks[] = {
  {"connect", CALLBACK_CONNECT},
  {"ehlo", CALLBACK_EHLO},
  {"mail", CALLBACK_MAIL},
  {"rcpt", CALLBACK_RCPT},
  {"data", CALLBACK_DATA},
  {"header", CALLBACK_HEADER},
  {"eoh", CALLBACK_EOH},
  {"body", CALLBACK_BODY},
  {"eom", CALLBACK_EOM},
  {"abort", CALLBACK_ABORT},
  {"close", CALLBACK_CLOSE},
  {NULL, -1}
};


char               *
callback_name(id)
     int                 id;
{
  if (id < 0)
    return NULL;

  return get_name_by_id(callbacks, id);
}
