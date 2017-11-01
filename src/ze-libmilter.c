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
 *  Creation     : Wed Nov 17 22:29:20 CET 2004
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
#include <ze-filter-data.h>
#include <ze-libmilter.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
jsmfi_setreply_from_access(ctx, msg)
     SMFICTX            *ctx;
     char               *msg;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 res;
  char               *sout, buf[1024];
  char               *url = cf_get_str(CF_POLICY_URL);

  char               *rcode, *xcode, *rmsg, *tag;
  char               *s, *p = NULL;

  tag = rcode = xcode = rmsg = NULL;
  {
    if ((s = strdup(msg)) != NULL)
    {
      p = tag = s;

      p = strchr(p, ':');
      if (p != NULL)
      {
        *p++ = '\0';
        rcode = p;

        p = strchr(p, ':');
        if (p != NULL)
        {
          *p++ = '\0';
          xcode = p;

          p = strchr(p, ':');
          if (p != NULL)
          {
            *p++ = '\0';
            rmsg = p;
          }
        }
      }
    }
  }

  rmsg = STRNULL(rmsg, "Access denied");
  rcode = STRNULL(rcode, "421");
  xcode = STRNULL(xcode, "4.5.0");

  sout = rmsg;
  if ((url != NULL) && (strlen(url) > 0))
  {
    snprintf(buf, sizeof (buf), "%s - See %s", rmsg, url);
    sout = buf;
  }

  if ((res = smfi_setreply(ctx, rcode, xcode, sout)) != MI_SUCCESS)
    MESSAGE_WARNING(9, "%s smfi_setreply returned MI_FAILURE",
                    CONNID_STR(priv->id));

  FREE(s);

  return res;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
jsmfi_setreply(ctx, ca, cb, msg)
     SMFICTX            *ctx;
     char               *ca;
     char               *cb;
     char               *msg;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 res;
  char               *sout, buf[1024];
  char               *url = cf_get_str(CF_POLICY_URL);

  sout = msg;

  if ((url != NULL) && (strlen(url) > 0))
  {
    snprintf(buf, sizeof (buf), "%s - See %s", msg, url);
    sout = buf;
  }

  if ((res = smfi_setreply(ctx, ca, cb, sout)) != MI_SUCCESS)
    MESSAGE_WARNING(9, "%s smfi_setreply returned MI_FAILURE",
                    CONNID_STR(priv->id));

  {
    size_t              sz;

    FREE(priv->reply_code);
#if 1
    sz = strlen(ca) + strlen(cb) + strlen(sout) + 4;
    ;
    if ((priv->reply_code = malloc(sz)) != NULL)
      snprintf(priv->reply_code, sz, "%s %s %s", ca, cb, sout);
#else
    sz = strlen(ca) + strlen(cb) + 2;

    if ((priv->reply_code = malloc(sz)) != NULL)
      snprintf(priv->reply_code, sz, "%s %s", ca, cb);
#endif
  }

  if (priv->reply_code == NULL)
    LOG_SYS_ERROR("strdup error");

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
jsmfi_vsetreply(SMFICTX * ctx, char *ca, char *cb, char *format, ...)
#if 0
     SMFICTX            *ctx;
     char               *ca;
     char               *cb;
     char               *format;
#endif
{
  va_list             arg;
  char                s[1024];

  va_start(arg, format);
  vsnprintf(s, sizeof (s), format, arg);
  va_end(arg);

  return jsmfi_setreply(ctx, ca, cb, s);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
Smfi_ChgFrom(ctx, mail, args)
     SMFICTX            *ctx;
     char               *mail;
     char               *args;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

#if HAVE_SMFI_CHGFROM
  if ((priv->mta_caps.f0 & SMFIF_CHGFROM) != 0)
    return smfi_chgfrom(ctx, mail, args);
#endif

  return SMFIS_CONTINUE;
}
