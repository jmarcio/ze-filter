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

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/


static bool         rule2reply(SMFICTX *, char *, char *, size_t, char *, char *);

typedef struct replymsg_T
{
  char               *code;
  char               *msg;
} replymsg_T;

static replymsg_T   replymsg[] = {
  {"NO_PEER_HOSTNAME", "*** No peer hostname when connecting"},
  {"XFILE", "*** A suspicious file (executable code) was found in the message!"},
  {"NO_FROM_HEADER", "*** There is no From: header field"},
  {"NO_RCPT_HEADER", "*** There is no To, CC or Bcc header field"},
  {"NO_SUBJECT_HEADER", "*** There is no Subject Header"},
  {"NO_HEADERS", "*** There are no headers..."},
  {"INTRANET_USER", "*** Mail to intranet user from outside world..."},
  {"LOCAL_USER", "*** %s can receive mail only from local network"},
  {"BINARY_MESSAGE", "*** Trying a buffer overflow ??? - Let's discard it! "},
  {"VIRUS", "*** A virus was detected in your message"},
  {"POLICY", "*** This message violates our policy"},
  {"TOO_MUCH_RCPT", "*** Too many recipients for this message"},
  {"RESOLVE_FAIL", "*** RESOLVE : mail gateway DNS lookup failed"},
  {"RESOLVE_FORGED", "*** RESOLVE : mail gateway hostname seems to be forged"},
  {"TOO_MUCH_EMPTY_CONN", "*** Too many empty connections"},
  {"TOO_MUCH_BADRCPT", "*** Harvesting ???"},
  {"TOO_MUCH_OPEN_CONN", "*** Too many open connections"},
  {"THROTTLE_EXCEEDED", "*** CONN THROTTLE : too high"},
  {"RCPT_THROTTLE_EXCEEDED", "*** RCPT THROTTLE : too high"},
  {"FROM_CONTENTS", "*** From: contents violates our site policy"},
  {"SUBJECT_CONTENTS", "*** Subject: contents violates our site policy"},
  {"HEADERS_CONTENTS", "*** Header contents violates our site policy"},
  {"BODY_CONTENTS",
   "*** Sorry, this server is configured to refuse that is indistinguishable from spam"},
  {"HELO_CONTENTS", "*** HELO contents : strange!!!"},
  {"TEXT_NOT_ALLOWED", "*** Message contents violate our site policy"},
  {"ENCODED_BODY", "*** We don't accept encoded messages"},
  {"BODY_ENCODED_BINARY", "*** No BINARY messages!"},
  {"BODY_ENCODED_BASE64", "*** No BASE 64 messages!"},
  {"BODY_ENCODED_QP", "*** No QUOTED PRINTABLE messages!"},
  {"BAD_NULL_SENDER", "*** Bad NULL sender"},
  {NULL, NULL}
};

static char        *
reply_code2msg(code)
     char               *code;
{
  replymsg_T         *p = replymsg;

  if (p == NULL)
    return NULL;

  while (p->code != NULL)
  {
    if (strcasecmp(p->code, code) == 0)
      return p->msg;
    p++;
  }

  return NULL;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define      STRING(a)     #a

bool
get_reply_msg(ctx, code, msg, sz, attr, value)
     SMFICTX            *ctx;
     char               *code;
     char               *msg;
     size_t              sz;
     char               *attr;
     char               *value;
{
#if 0
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
#endif
  char                b_in[256];

  char                b_out[1024];
  char               *sin = NULL, *sout = NULL;
  bool                result = FALSE;

  if (db_policy_check("ReplyMsg", code, b_in, sizeof (b_in)))
    sin = b_in;
  else
    sin = reply_code2msg(code);

  if (sin == NULL)
    ZE_LogMsgWarning(0, "Can't get message for code : %s", code);

  if ((sin != NULL) && rule2reply(ctx, b_out, b_in, sizeof (b_out), attr, value))
  {
    safe_strncpy(msg, sz, b_out, strlen(b_out));
    result = TRUE;
  } else
  {
    sout = "*** Unknown error";

    safe_strncpy(msg, sz, sout, strlen(sout));
    result = TRUE;
  }

  return result;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
rule2reply(ctx, out, in, sz, attr, value)
     SMFICTX            *ctx;
     char               *out;
     char               *in;
     size_t              sz;
     char               *attr;
     char               *value;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  char               *p, *q;
  long                pi, pf;

  if (in == NULL || out == NULL)
    return FALSE;

  if (sz-- < 1)
    return FALSE;

  p = in;
  q = out;
  while ((sz > 0) && strexpr(p, "__[A-Z0-9_]+__", &pi, &pf, TRUE))
  {
    char               *tag = NULL;
    int                 n;

    if (pi > 0)
    {
      n = safe_strncat(q, sz, p, pi);
      p += pi;
      q += n;
      sz -= n;

      continue;
    }

    /*
     **  Check for variables...
     */
    if ((attr != NULL) && (strlen(attr) > 0)
        && (strncasecmp(p, attr, strlen(attr)) == 0))
    {
      if (value != NULL)
      {
        n = safe_strncat(q, sz, value, strlen(value));
        q += n;
        sz -= n;
      }
      p += pf;
      continue;
    }

    tag = "__VIRUS__";
    if (strncasecmp(p, tag, strlen(tag)) == 0)
    {

      p += pf;
      continue;
    }

    tag = "__VIRUS__";
    if (strncasecmp(p, tag, strlen(tag)) == 0)
    {

      p += pf;
      continue;
    }

    tag = "__CLNT_ADDR__";
    if (strncasecmp(p, tag, strlen(tag)) == 0)
    {
      char               *addr = NULL;

      addr = (priv != NULL && priv->peer_addr) ? priv->peer_addr : "???";

      n = safe_strncat(q, sz, addr, strlen(addr));

      /* printf(" pi = %ld; pf = %ld; n = %d\n", pi, pf, n); */

      q += n;
      sz -= n;

      p += pf;
      continue;
    }

    tag = "__CLNT_NAME__";
    if (strncasecmp(p, tag, strlen(tag)) == 0)
    {

      p += pf;
      continue;
    }

    tag = "__FROM__";
    if (strncasecmp(p, tag, strlen(tag)) == 0)
    {

      p += pf;
      continue;
    }

    tag = "__TO__";
    if (strncasecmp(p, tag, strlen(tag)) == 0)
    {

      p += pf;
      continue;
    }

    p += pf;
  }

  if (p != NULL)
    (void) safe_strncat(q, sz, p, strlen(p));

  return TRUE;
}
