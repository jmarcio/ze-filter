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

#ifndef __JFILTER_H__

#include "libmilter/mfapi.h"

#ifndef  SM_LM_VRS_MAJOR
# define SM_LM_VRS_MAJOR(v)      (((v) & 0x7f000000) >> 24)
#endif
#ifndef  SM_LM_VRS_MINOR
# define SM_LM_VRS_MINOR(v)      (((v) & 0x007fff00) >> 8)
#endif
#ifndef  SM_LM_VRS_PLVL
# define SM_LM_VRS_PLVL(v)       ((v) & 0x0000007f)
#endif

#define SMFI_VERSION_MAJOR       SM_LM_VRS_MAJOR(SMFI_VERSION)
#define SMFI_VERSION_MINOR       SM_LM_VRS_MINOR(SMFI_VERSION)
#define SMFI_VERSION_PLVL        SM_LM_VRS_PLVL(SMFI_VERSION)

#include "ze-smmacros.h"
#include "ze-reply.h"

#if 0
#ifndef LIBMILTER_VERSION_MAJOR
# define LIBMILTER_VERSION_MAJOR        8
# define LIBMILTER_VERSION_MINOR        12
# define LIBMILTER_VERSION_PATCH        10
#endif
#endif

#include "ze-history.h"

typedef struct
{
  char               *virus;
} MSG_T;

typedef struct
{
  unsigned long       f0;
  unsigned long       f1;
  unsigned long       f2;
  unsigned long       f3;
} MTA_caps_T;

typedef struct
{
  sfsistat            result;
  int                 callback;
  char               *reply;
  char               *why;
} dresult_T;

typedef struct
{
  time_t              conn_id;  /* connection ID - obsolete */
  CONNID_T            id;       /* connection ID */
#if 1
  char               *mailserver; /* server name (Macro $j) */
#endif
  int                 callback_id;

  int                 nb_unknown_cmd;

  char               *reply_code;

  dresult_T           delayed_result;

  /*
   ** Connection data 
   */
#if HAVE_HRTIME_T
  hrtime_t            t_open;   /* connection open date */
  hrtime_t            t_close;  /* connection close date */
  hrtime_t            t_callback; /* time spent by the callback */
  hrtime_t            t_xmsg;    /* time spent processing message body */
#else
  time_t              t_open;
  time_t              t_close;
  time_t              t_callback;
  time_t              t_xmsg;
#endif

  MTA_caps_T          mta_caps;

  /*
   ** Client and server state
   */
  int                 serv_rate;  /* server connection rate */
  int                 conn_rate;  /* client connection rate */
  int                 nb_open;  /* # of open connections */
  int                 msg_rate; /* client message rate */

  iprbwl_T            rbwl;
  netclass_T          netclass;

  int                 resolve_res;

  uint32_t            ehlo_flags; /* result of helo parameter check */

  bool                reject_connect;

  /*
   ** Spool file 
   */
  char               *fname;    /* spool file name */
  char               *fsuffix;  /* quarantined file name suffix */
  int                 fd;
  bool                fp_open;  /* True if spool open */
  bool                save_msg; /* shall message be quarantined */
  int                 save_why; /* quarantine reason */

#if 0
  MSG_SPOOL_T         spoolfile;
#endif
  sm_mac_T           *sm;

  /*
   ** Input data
   */
  char               *daemon;
  sa_family_t         addr_family;
  char               *peer_addr;
  char               *peer_name;
  char               *ident;

  char               *helohost;

  /* message flags */
  msg_flags_T         flags;

  /*
   ** Message
   */
  MSG_T               msg;
  char               *sm_msgid;

  char               *env_from;
  char               *env_to;
  rcpt_addr_T        *env_rcpt;
  int                 env_nb_rcpt;

  bool                pass_ok;  /* access granted */

  char               *hdr_mailer;
  char               *hdr_from;
  char               *hdr_to;
  char               *hdr_subject;
  int                 hdr_content_encoding;

  header_T           *headers;

  /* Anti-spam */
  msg_scores_T        rawScores;
  msg_scores_T        netScores;
  msg_scores_T        dspScores;

  spamchk_T           spamchk;

  char               *score_str;
  char               *status_str;

  /* bayes filter result */
  int                 nb_bspam;
  int                 nb_bham;

  /* Contents */
  char                body_chunk[SZ_CHUNK];
  int                 body_res_scan;
  int                 body_scan_state;
  int                 body_nb;

  content_field_T     tcontent;
  content_field_T    *lcontent;

  bool                msg_short;
  unsigned long       msg_size;
  unsigned long       nb_bytes;

  int                 nb_from;
  int                 nb_rcpt;  /* nb of recipients */

  int                 nb_files; /* nb of attached files */
  int                 nb_xfiles;  /* nb of X-Files */
  int                 nb_virus; /* nb of virus */
  int                 nb_policy;  /* nb of policy violations */
  int                 nb_msgs;  /* nb of messagess */
  int                 nb_abort; /* nb of aborted messages */

  int                 nb_spams; /* nb spams */

  /* RCPT database check results */
  int                 dbrcpt_reject;
  int                 dbrcpt_access;
  int                 dbrcpt_bad_network;
  int                 dbrcpt_conn_unknown;
  int                 dbrcpt_conn_spamtrap;
  int                 dbrcpt_msg_unknown;
  int                 dbrcpt_msg_spamtrap;

  int                 nb_mbadrcpt;  /* nb of bad recipients - message */
  int                 nb_cbadrcpt;  /* nb of bad recipients - connection */

  /*
   ** Results
   */
  uint32_t            flag_rej_connection;
  uint32_t            flag_rej_contents;

  int                 rej_rcpt;
  int                 rej_rcpt_rate;
  int                 rej_regex;
  int                 rej_luser;
  int                 rej_badmx;
  int                 rej_spamtrap;

  bool                rej_resolve;
  bool                rej_conn_rate;
  bool                rej_open;
  bool                rej_empty;
  bool                rej_badrcpt;
  bool                rej_msgs;
  bool                rej_msg_rate;

  int                 rej_greyreply;
  int                 rej_greyrcpt;
  int                 rej_greymsgs;

  int                 nb_oracle;

  int                 result;
} CTXPRIV_T;

#define MLFIPRIV(ctx)						\
  ((ctx) != NULL ? (CTXPRIV_T *) smfi_getpriv(ctx) : NULL)

#define CTX_NETCLASS_LABEL(priv)					\
  ((priv) != NULL ?							\
   STREMPTY(priv->netclass.label, NET_CLASS_LABEL(priv->netclass.class)) : \
   "UNKNOWN")

#include <ze-check-connection.h>

#include <ze-callback.h>
#include <ze-callbackchecks.h>
#include <ze-callbacklogs.h>

#include <ze-log-files.h>
#include <ze-log-virus.h>
#include <ze-log-quarantine.h>
#include <ze-log-grey.h>
#include <ze-log-regex.h>

#include <ze-libmilter.h>
#include <ze-mod-tools.h>

extern char         my_hostname[];

extern int          mx_check_level;

#define LOG_CONNECTION_HEADER(s,msg,n,ip,class)         \
  do                                                    \
  {                                                     \
    snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]", \
	     msg, ip, n, class, NET_CLASS(class));      \
  }                                                     \
  while (0)

#define   DO_QUARANTINE_MESSAGE(priv,why,suffix)	\
  do {							\
    ASSERT(priv != NULL);				\
    priv->save_msg = TRUE;				\
    priv->save_why = why;				\
    if (suffix == NULL) {				\
      switch (why) {					\
        case WHY_XFILE:					\
	  priv->fsuffix = SUFFIX_XFILE;			\
	  break;					\
        case WHY_VIRUS:					\
	  priv->fsuffix = SUFFIX_VIRUS;			\
	  break;					\
        case WHY_POLICY:				\
	  priv->fsuffix = SUFFIX_POLICY;		\
	  break;			      		\
        case WHY_SPAM:					\
	  priv->fsuffix = SUFFIX_SPAM;			\
	  break;					\
        case WHY_QUARANTINE:				\
	  priv->fsuffix = SUFFIX_QUARANTINE;	       	\
	  break;					\
        case WHY_ARCHIVE:				\
	  priv->fsuffix = SUFFIX_ARCHIVE;	       	\
	  break;					\
        default:			       		\
	  priv->fsuffix = NULL;		       		\
	  break;			       		\
      }							\
    }							\
    else						\
      priv->fsuffix = suffix;				\
  } while (0)

#define __JFILTER_H__
#endif
