
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
#include "ze-spool.h"

#define SWAP_PTR(a,b)         do { void *c = a; a = b; b = c;} while (0)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

bool                spool_file_create(CTXPRIV_T *);
bool                spool_file_write(CTXPRIV_T *, char *, size_t);
bool                spool_file_close(CTXPRIV_T *);
bool                spool_file_forget(CTXPRIV_T *);


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
spool_file_create(priv)
     CTXPRIV_T          *priv;
{
  char               *p;
  char                fname[256];

  int                 fd;

  char               *rc = CRLF;

#if _FFR_CLEAN_MSG_BUF
  rc = "\n";
#endif

  if (priv->fd >= 0)
    return TRUE;

  priv->fp_open = FALSE;

  if (priv->fname != NULL)
    ZE_LogMsgWarning(0, "fd closed, but fname not NULL");
  FREE(priv->fname);

  if ((p = cf_get_str(CF_SPOOLDIR)) == NULL)
    p = ZE_SPOOLDIR;

  if (cf_get_int(CF_CLUSTER) == OPT_YES) {
    char               *server = NULL;

    server = STRNULL(priv->mailserver, "SERVER");

    snprintf(fname, sizeof (fname), "%s/%s.%s.%04X", p, server,
             CONNID_STR(priv->id), priv->nb_msgs);

  } else {
    snprintf(fname, sizeof (fname), "%s/%s.%04X", p, CONNID_STR(priv->id),
             priv->nb_msgs);
  }

  fd = open(fname, O_WRONLY | O_APPEND | O_CREAT, 0640);

  if (fd < 0) {
    ZE_LogSysError("can't create spool file (%s)", fname);
    fd = -1;
    return FALSE;
  }

  if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
    ZE_LogSysError("can't change spool file rights (%s)", fname);

  priv->fd = fd;
  priv->fp_open = TRUE;

  if ((priv->fname = strdup(fname)) == NULL)
    ZE_LogSysError("strdup(%s) error", fname);

  /*
   * Let's add a fake From line 
   */
  if (cf_get_int(CF_QUARANTINE_ADD_FROM_LINE) == OPT_YES) {
    char                s[256];
    char                t[256];
    time_t              tid = CONNID_INT(priv->id);

#if HAVE_CTIME_R
#ifdef _POSIX_PTHREAD_SEMANTICS
    ctime_r(&tid, t);
#else
    ctime_r(&tid, t, sizeof (t));
#endif
#else
#if HAVE_CTIME
    strlcpy(t, ctime(&tid, sizeof (t));
#else
    strlcpy(t, "-", sizeof (t));
#endif
#endif

    if ((p = strchr(t, '\r')) != NULL)
      *p = '\0';
    if ((p = strchr(t, '\n')) != NULL)
      *p = '\0';

    if (priv->env_from != NULL)
      snprintf(s, sizeof (s) - 3, "From %s %s", priv->env_from, t);
    else
      snprintf(s, sizeof (s) - 3, "From %s %s", "<unknown>", t);
    strcat(s, rc);

    if (write(priv->fd, s, strlen(s)) != strlen(s))
      ZE_LogSysError("can't write fake From: line to %s", priv->fname);
  }

  if (1) {
    char                rbuf[512];
    char               *mac_srv_name = NULL;
    char               *mac_cl_addr = NULL;
    char               *mac_cl_ptr = NULL;
    char               *mac_cl_name = NULL;
    char               *mac_qid = NULL;

    mac_srv_name = sm_macro_get_str(priv->sm, "j");
    mac_srv_name = STREMPTY(mac_srv_name, my_hostname);
    mac_cl_addr = sm_macro_get_str(priv->sm, "{client_addr}");
    mac_cl_addr = STREMPTY(mac_cl_addr, "null");
    mac_cl_ptr = sm_macro_get_str(priv->sm, "{client_ptr}");
    mac_cl_ptr = STREMPTY(mac_cl_ptr, "null");
    mac_cl_name = sm_macro_get_str(priv->sm, "{client_name}");
    mac_cl_name = STREMPTY(mac_cl_name, "null");
    mac_qid = sm_macro_get_str(priv->sm, "i");
    mac_qid = STREMPTY(mac_qid, "null");

/*
Received: from qxxge.proxad.net (sge78-3-82-247-96-164.fbx.proxad.net [82.247.96.164])
	by paris.ensmp.fr (8.14.4/8.14.4/JMMC-31/May/2010) with SMTP id o73Ed8Cs001045
	for <dns@ensmp.fr>; Tue, 3 Aug 2010 16:39:09 +0200 (MEST)
*/
    snprintf(rbuf, sizeof (rbuf),
             "Received: from %s (%s [%s])%s\tby %s with SMTP id %s%s",
             mac_cl_ptr,
             mac_cl_name, mac_cl_addr, rc, mac_srv_name, mac_qid, rc);

    if (write(priv->fd, rbuf, strlen(rbuf)) != strlen(rbuf))
      ZE_LogSysError("can't write fake Received: line to %s", priv->fname);

    snprintf(rbuf, sizeof (rbuf),
             "X-ze-filter-Envelope: %s from %s/%s/%s/%s/%s%s",
             CONNID_STR(priv->id),
             mac_cl_name, mac_cl_ptr, mac_cl_addr,
             priv->helohost, priv->env_from, rc);
    /*
     * snprintf(rbuf, sizeof(rbuf), 
     * "X-Received: from %s (%s [%s])%s",
     * mac_cl_ptr, 
     * mac_cl_name,
     * mac_cl_addr,
     * rc);
     */
    if (write(priv->fd, rbuf, strlen(rbuf)) != strlen(rbuf))
      ZE_LogSysError("can't write fake X-ze-filter-Envelope: line to %s",
                     priv->fname);
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
spool_file_write(priv, buf, size)
     CTXPRIV_T          *priv;
     char               *buf;
     size_t              size;
{
  if (priv->fd < 0)
    return TRUE;

  if (write(priv->fd, buf, size) != size) {
    ZE_LogSysError("Error writing on spool file %s", priv->fname);
    return FALSE;
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
spool_file_close(priv)
     CTXPRIV_T          *priv;
{
  if (priv->fd < 0)
    return TRUE;

  priv->fp_open = FALSE;
  if (close(priv->fd) < 0) {
    ZE_LogSysError("error closing spool file");
    priv->fd = -1;
    return FALSE;
  }
  priv->fd = -1;

  return TRUE;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
spool_file_forget(priv)
     CTXPRIV_T          *priv;
{
  char               *filename = NULL;
  char               *suffix = NULL;
  size_t              sz = 0;

  (void) spool_file_close(priv);

  if (priv->fname == NULL)
    goto fin;

  if (!priv->save_msg) {
    unlink(priv->fname);
    goto fin;
  }

  suffix = STREMPTY(priv->fsuffix, SUFFIX_UNKNOWN);
  sz = strlen(priv->fname) + strlen(suffix) + 4;

  filename = (char *) malloc(sz);
  if (filename != NULL) {
    snprintf(filename, sz, "%s%s", priv->fname, suffix);
    if (rename(priv->fname, filename) == 0)
      SWAP_PTR(filename, priv->fname);
    else
      ZE_LogSysError("Error renaming quarantine file : %s", priv->fname);
  } else
    ZE_LogSysError("Quarantine file name malloc error : %s", priv->fname);
  ZE_MessageInfo(11, "%-12s : quarantine file %s", CONNID_STR(priv->id),
                 priv->fname);

fin:
  priv->fsuffix = NULL;
  priv->save_msg = FALSE;
  FREE(priv->fname);
  FREE(filename);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
spool_file_is_open(priv)
     CTXPRIV_T          *priv;
{
  if (priv == NULL)
    return FALSE;

  return priv->fp_open;
}
