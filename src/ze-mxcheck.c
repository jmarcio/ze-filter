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
  *  Creation     : Thu Oct 27 13:42:06 CEST 2005
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
#include <j-filter.h>
#include <j-mxcheck.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define               SZ_NAME  64

typedef struct MX_T
{
  char                domain[SZ_NAME];
  char                mx[SZ_NAME];
  time_t              last_update;
  time_t              last_query;
} MX__T;

static struct
{
  bool                ok;
  time_t              last;
  int                 nb;

  pthread_mutex_t     mutex;

  JBT_T               db_open;
}
hdata =
{
FALSE, (time_t) 0, 0, PTHREAD_MUTEX_INITIALIZER, JBT_INITIALIZER};

#define MX_OK(x)     (strcasecmp(x, "OK") == 0)


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define MSG_BAD_MX(a,b) 						\
	do {								\
	      MESSAGE_INFO(9, "%s BAD MXs for domain %s : %s (%s)", 	\
                   CONNID_STR(priv->id), a, b, priv->peer_addr); 	\
	} while (0)

int                 mx_check_level = 1;

int
check_sender_mx(ctx, mail_host)
     SMFICTX            *ctx;
     char               *mail_host;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 ip_class = (priv != NULL ? priv->netclass.class : 0);
  int                 result = SMFIS_UNDEF;

  timems_T            ti, tf;

  ti = tf = 0;

  if (mail_host == NULL || strlen(mail_host) == 0)
    return SMFIS_CONTINUE;
  MESSAGE_INFO(11, "%s Checking MXs for domain %s", CONNID_STR(priv->id),
               mail_host);

  if (IS_KNOWN(ip_class))
    goto fin;

  if (mx_check_level < 0)
  {
    char               *env;

    if ((env = getenv("MXCHECKLEVEL")) != NULL)
    {
      int                 i;

      i = str2long(env, NULL, 1);
      if (errno == 0 && i < 100)
        mx_check_level = i;
    }

    if (mx_check_level < 0)
      mx_check_level = 1;
  }

  if (!((mx_check_level > 1) || IS_UNKNOWN(ip_class)))
    goto fin;

#if _FFR_USE_MX_CACHE
  /* Let's check if this guy is already in cache... */
  {
    char                buf[256];

    if (res_cache_check("mx", mail_host, buf, sizeof (buf)))
    {

    }
  }
#endif             /* _FFR_USE_MX_CACHE */

  ti = time_ms();

  {
    DNS_HOSTARR_T       mx;
    int                 i;
    int                 res = 0;
    int                 nchk = 0;
    char                buf[256];
    smtp_reply_T        r;

    if (check_policy("BadMX", mail_host, buf, sizeof (buf), FALSE))
    {
      if (!MX_OK(buf))
      {
        MSG_BAD_MX(mail_host, mail_host);

        jc_string2reply(&r, buf);
        (void) jsmfi_setreply(ctx, r.rcode, r.xcode, r.msg);
        result = r.result;
      } else
        result = SMFIS_CONTINUE;
    }

    if (result != SMFIS_UNDEF)
      goto fin;

    memset(&mx, 0, sizeof (mx));
    res = dns_get_mx(mail_host, &mx);

    /* some MXs found : let's check them */
    if (res > 0)
    {
      for (i = 0; i < mx.count; i++)
      {
        nchk++;

        MESSAGE_INFO(11, "%s -> MX %3d %-16s %s\n",
                     CONNID_STR(priv->id),
                     mx.host[i].pref,
                     STRNULL(mx.host[i].ip, ""), STRNULL(mx.host[i].name, ""));

        memset(buf, 0, sizeof (buf));
        if (check_policy("BadMX", mx.host[i].ip, buf, sizeof (buf), FALSE))
        {
          if (!MX_OK(buf))
          {
            MSG_BAD_MX(mail_host, mx.host[i].ip);

            /* Found at database - let's decode */
            jc_string2reply(&r, buf);
            (void) jsmfi_setreply(ctx, r.rcode, r.xcode, r.msg);
            result = r.result;
          } else
            result = SMFIS_CONTINUE;
          break;
        }
        if (check_policy("BadMX", mx.host[i].name, buf, sizeof (buf), FALSE))
        {
          if (!MX_OK(buf))
          {
            MSG_BAD_MX(mail_host, mx.host[i].name);

            /* Found at database - let's decode */
            jc_string2reply(&r, buf);
            (void) jsmfi_setreply(ctx, r.rcode, r.xcode, r.msg);
            result = r.result;
          } else
            result = SMFIS_CONTINUE;
          break;
        }
      }
    }
    dns_free_hostarr(&mx);

    if (result != SMFIS_UNDEF)
      goto fin;

    /* No MX found - let's look for IP address associated to the domain part */
    if (nchk == 0)
    {
      memset(&mx, 0, sizeof (mx));
      res = dns_get_a(mail_host, &mx);
      if (res > 0)
      {
        for (i = 0; i < mx.count; i++)
        {
          nchk++;

          MESSAGE_INFO(11, "%s -> A  %3d %-16s %s\n",
                       CONNID_STR(priv->id),
                       mx.host[i].pref,
                       STRNULL(mx.host[i].ip, ""), STRNULL(mx.host[i].name,
                                                           ""));

          memset(buf, 0, sizeof (buf));
          if (check_policy("BadMX", mx.host[i].ip, buf, sizeof (buf), FALSE))
          {
            if (!MX_OK(buf))
            {
              MSG_BAD_MX(mail_host, mx.host[i].ip);

              /* Found at database - let's decode */
              jc_string2reply(&r, buf);
              (void) jsmfi_setreply(ctx, r.rcode, r.xcode, r.msg);
              result = r.result;
            } else
              result = SMFIS_CONTINUE;
            break;
          }
        }
      }
      dns_free_hostarr(&mx);

      if (result != SMFIS_UNDEF)
        goto fin;

      /* NO IP associated with this domain name ??? */
      if (nchk == 0)
      {
        char               *reply = NULL;

        MESSAGE_INFO(10, "%s Domain %s doesn't resolve", CONNID_STR(priv->id),
                     mail_host);

        reply = cf_get_str(CF_DEFAULT_BAD_MX_REPLY);
        if (reply == NULL || strlen(reply) == 0)
          reply = "ERROR:421:4.5.1:DNS problems... Try later !";

        strlcpy(buf, reply, sizeof (buf));

        if (jc_string2reply(&r, buf) != SMFIS_CONTINUE)
        {
          (void) jsmfi_setreply(ctx, r.rcode, r.xcode, r.msg);
          result = r.result;
        }
      }

      if (res < 0)
      {
      }
    }
  }

fin:
  if (result == SMFIS_UNDEF)
    result = SMFIS_CONTINUE;

  tf = time_ms();
  if (ti != 0 && tf > ti)
  {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static kstats_T     st = KSTATS_INITIALIZER;
    static int          ns = 0;

    double              dt;

    MUTEX_LOCK(&mutex);

    dt = (double) (tf - ti);

    kstats_update(&st, dt);

    if ((++ns % 1000) == 0)
    {
      MESSAGE_INFO(10,
                   "MX CHECK delay : nb=%d min=%5.3f mean=%5.3f max=%5.3f stddev=%5.3f ",
                   ns, kmin(&st), kmean(&st), kmax(&st), kstddev(&st));
      kstats_reset(&st);
      ns = 0;
    }

    /* add to resolve cache map */
    if (dt >= 1000 || result != SMFIS_CONTINUE)
    {
#if _FFR_USE_MX_CACHE
      /* don't know what exactly add to the map... */
      res_cache_add("mx", mail_host, "xxx");
#endif             /* _FFR_USE_MX_CACHE */
    }

    MUTEX_UNLOCK(&mutex);
  }

  if (result != SMFIS_CONTINUE)
  {
    char                logbuf[256];

    priv->rej_badmx++;
    stats_inc(STAT_BADMX, 1);
    snprintf(logbuf, sizeof (logbuf), "%s %s", "BAD MX -> ", mail_host);

    log_msg_context(ctx, logbuf);
  }

  return result;
}
