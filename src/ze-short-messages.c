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
 *  Creation     : Sun Sep 21 18:52:30 CEST 2008
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
#include <ze-short-messages.h>

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

#if 0
 HELp
 INFO <liste>
 LISts
 REView <liste>
 WHICH
 SUBscribe <liste> <nom>
 UNSubscribe <liste> <email>
 UNSubscribe * <EMAIL>
 SET <liste>|* NOMAIL
 SET <list>|* DIGEST
 SET <liste|*> DIGESTPLAIN
 SET <liste>|* SUMMARY
 SET <liste>|* NOTICE
 SET <liste>|* MAIL
 SET <liste>|* CONCEAL
 SET <liste>|* NOCONCEAL
 INDex
 GET <liste> <fichier>
 LAST <liste>
 INVITE <liste> <email>
 CONFIRM <clé>

 ADD <liste> jean@serveur nom complet
 DEL <liste> jean@serveur
 STATS <liste>

 REMIND <liste>

 DISTribute <liste> <clé>
 REJect <liste> <clé>
 MODINDEX <liste>

 QUIT
#endif

static char *sympa_cmds[] = {
  "^hel(p)?",
  "^info .+",
  "^lis(ts)?",
  "^rev(iew) .+",
  "^which",
  "^sub(scribe)? .+",
  "^uns(ubscribe)? .+",
  "^set .+",
  "^ind(ex)?",
  "^get .+ .+",
  "^last .+",
  "^invite .+ .+",
  "^confirm .+",
  "^quit",
  "^add .+",
  "^del .+",
  "^stats .+",
  "^remind .+",
  "^dist(ribute) .+",
  "^rej(ect) .+",
  "^modindex .+",
  NULL};

#if 0
  if (priv->body_nb == 0 && IS_UNKNOWN(priv->ip_class))
  {
    char                buf[1024];
    header_T           *h = priv->headers;
    int                 i;
    bool                doit = FALSE;

    if (bodylen > 1023)
      goto ok;

    if (cf_get_int(CF_REJECT_SHORT_BODIES) != OPT_YES)
      goto ok;

    i = 0;
    while (i < bodylen && isspace(bodyp[i]))
      i++;

    memcpy(buf, bodyp + i, bodylen - i);
    buf[bodylen - i] = '\0';

    while ((i = strlen(buf)) > 0)
    {
      if (!isspace(buf[i - 1]))
        break;
      buf[i - 1] = '\0';
    }

    if (strlen(buf) >= cf_get_int(CF_MIN_BODY_LENGTH))
      goto ok;

    /* is this a subscription message ? */
    if (strexpr(buf, "subscribe", NULL, NULL, TRUE))
      goto ok;

    while ((h = get_msgheader_next(h, "Subject")) != NULL)
    {
      if (h->value != NULL && strexpr(h->value, "subscribe", NULL, NULL, TRUE))
        goto ok;
    }

    /* autres ??? */
#if 0
    if (0)
    {
      char *p, *q;
      int   i;

      while (p != NULL && *p != '\0') {
	if ((p = strpbrk(p, " \t\n")) != NULL) {
	  i++;
	  p++;
	  continue;
	}
      }
      for (p = buf, i = 0; 
    }
#endif

    ZE_MessageInfo(10, "%s : This is a short message...",
		 CONNID_STR(priv->id), strlen(buf));

    priv->msg_short = TRUE;
  }

ok:
#endif
