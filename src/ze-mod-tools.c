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
 *  Creation     : Sun Jun 10 15:23:49 CEST 2007
 *
 * This program is free software, but with restricted license :
 *
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#include <j-sys.h>
#include <ze-filter.h>
#include "j-filter.h"
#include <j-spool.h>
#include <j-mod-tools.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
ctx2mod_args(mod, ctx)
     mod_ctx_T          *mod;
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  ASSERT(priv != NULL);
  memset(mod, 0, sizeof (*mod));
  mod->callback = priv->callback_id;
  mod->id = CONNID_STR(priv->id);
  mod->claddr = priv->peer_addr;
  mod->clname = priv->peer_name;
  mod->helo = priv->helohost;
  mod->from = priv->env_from;
  mod->rcpt = priv->env_to;
  mod->sfile = priv->fname;
  mod->xfiles = FALSE;          /* XXX */
  mod->raw_scores = &priv->rawScores; /* XXX */
  mod->net_scores = &priv->netScores; /* XXX */
  mod->result = 0;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
mod2ctx_result(r)
     int                 r;
{
  switch (r)
  {
    case MODR_REJECT:
      return SMFIS_REJECT;
      break;
    case MODR_TEMPFAIL:
      return SMFIS_TEMPFAIL;
      break;
  }
  return SMFIS_CONTINUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
do_module_callback(ctx, step, result)
     SMFICTX            *ctx;
     int                 step;
     int                *result;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  mod_ctx_T           mctx;
  bool                stop = FALSE;

  *result = SMFIS_CONTINUE;
  memset(&mctx, 0, sizeof (mctx));
  ctx2mod_args(&mctx, ctx);

  module_call(priv->callback_id, step, &mctx);

  *result = mod2ctx_result(mctx.result);

  if (MOD_QUARANTINE(mctx.flags))
    DO_QUARANTINE_MESSAGE(priv, WHY_QUARANTINE, SUFFIX_QUARANTINE);

  MESSAGE_INFO(12, "RESULT : %08x %08x", mctx.result, mctx.flags);

  if (*result != SMFIS_CONTINUE)
  {
    char                reason[128];

    /* set reply */
    if (strlen(mctx.code) > 0 && strlen(mctx.xcode) > 0 && strlen(mctx.reply) > 0)
    {
      (void) jsmfi_setreply(ctx, mctx.code, mctx.xcode, mctx.reply);
    }

    /* log context */
    snprintf(reason, sizeof (reason), "Message rejected by module %s",
             mctx.modname);
    log_msg_context(ctx, reason);
  }

  stop = MOD_STOP_CHECKS(mctx.flags);

  return stop;
}
