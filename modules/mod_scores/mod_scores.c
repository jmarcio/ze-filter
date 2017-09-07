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
#include <mod_scores.h>
#include <modmac.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static mod_info_T   info = {
  "mod_scores",
  "Jose-Marcio Martins da Cruz",
  "0.1.0 Alpha",
  0,
  0
};

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
  if (version != MOD_VERSION)
    printf("API Version mismatch : %08X %08X\n", version, MOD_VERSION);
  printf("NAME     : %s\n", info.name);
  printf("AUTHOR   : %s\n", info.author);
  printf("VERSION  : %s\n", info.version);
  printf("VERSION  : %08X\n", version);
  printf("ARGS     : %s\n", STRNULL(modsys->args,"null"));

  printf("CALL OFF : %08X\n", modsys->calloffer);
  printf("CALL REQ : %08X\n", modsys->callrequest);

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
     mod_ctx_T         *arg;
{
  char *code = "400";
  char *xcode = "4.5.0";
  char *reply = "Access denied";

  if (arg == NULL)
    goto fin;

  arg->result = MODR_CONTINUE;

  syslog(LOG_INFO, "CALLBACK=%-10s : checking %-10s %-16s %s %s %s\n", 
	 CALLBACK_LABEL(callback),
	 STRNULL(arg->id, "(null)"), STRNULL(arg->claddr, "(null)"), 
	 STRNULL(arg->clname, "(null)"), STRNULL(arg->from, "(null)"), 
	 STRNULL(arg->rcpt, "(null)"));

#if 0
  switch (callback)
  {
    default:
      printf("     Callback  : %-12s %2d\n", CALLBACK_LABEL(callback), step);
      SET_REPLY(arg,code,xcode,reply);

      arg->result = MODR_CONTINUE;
      break;
  }
#endif

fin:
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_service(why)
     int why;
{
  printf("  * Servicing module %s\n", info.name);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
mod_close()
{
  printf("  * Closing module %s\n", info.name);

  return TRUE;
}

