
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Nov 15 23:13:14 CET 2004
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
#include <ze-callbacklogs.h>
#include <ze-spool.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define ADDROKTOANSWER(x)  ((strlen(STRNULL(x)) != 0) && (strcmp(NULLSENDER,x) != 0))

bool
check_address_ok2warn(p)
     char               *p;
{
  if ((p == NULL) || (strlen(p) == 0))
    return FALSE;

  if (strcmp(NULLSENDER, p) == 0)
    return FALSE;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
do_notify(ctx, ahead, answer, why, tag)
     SMFICTX            *ctx;
     attachment_T       *ahead;
     char               *answer;
     char               *why;
     char               *tag;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  char               *msg;
  bool                warn_send = FALSE;
  bool                warn_rcpt = FALSE;

  warn_send = (cf_get_int(CF_NOTIFY_SENDER) == OPT_YES);
  warn_rcpt = (cf_get_int(CF_NOTIFY_RCPT) == OPT_YES);

  if (!warn_send && !warn_rcpt)
    return SMFIS_DISCARD;

  if (!warn_rcpt) {
    rcpt_addr_T        *p = priv->env_rcpt;

    while (p != NULL) {
      if (p->rcpt != NULL)
        smfi_delrcpt(ctx, p->rcpt);
      p = p->next;
    }
  }

  if (warn_send) {
    char               *p = NULL;

    if (check_address_ok2warn(priv->env_from)) {
      p = priv->env_from;
    } else {
      if (check_address_ok2warn(priv->hdr_from))
        p = priv->hdr_from;
    }

    if (p != NULL)
      smfi_addrcpt(ctx, p);
    else
      warn_send = FALSE;
  }

  if (!warn_send && !warn_rcpt)
    return SMFIS_DISCARD;

  if ((msg = (char *) malloc(WARN_MSG_SIZE)) != NULL) {
    char                buf[256];

    /*
     * Content-Type: mutipart/report; report-type=delivery-status 
     */
    smfi_chgheader(ctx, "Content-type", 1, NULL);
    smfi_chgheader(ctx, "Content-disposition", 1, NULL);
    smfi_chgheader(ctx, "Content-transfer-encoding", 1, NULL);

#if 1
    snprintf(buf, sizeof (buf), "<%s.%04X.ze-filter@%s>", CONNID_STR(priv->id),
             priv->nb_msgs, my_hostname);
    smfi_chgheader(ctx, "Message-ID", 1, buf);
    ZE_MessageInfo(10, "%s changing Message-ID header to %s",
                   CONNID_STR(priv->id), buf);
#endif

    if (cf_get_int(CF_ZE_SENDER) != OPT_SENDER) {
      char               *sender = cf_get_str(CF_ZE_SENDER);

      if ((sender != NULL) && (strlen(sender) > 0))
        smfi_chgheader(ctx, "From", 1, sender);
    }

    Smfi_ChgFrom(ctx, "<>", NULL);

    if (cf_get_int(CF_ZE_SUBJECT) != OPT_SUBJECT) {
      char               *subject = cf_get_str(CF_ZE_SUBJECT);

      if ((subject != NULL) && (strlen(subject) > 0))
        smfi_chgheader(ctx, "Subject", 1, subject);
    }
    memset(msg, 0, WARN_MSG_SIZE);
    read_error_msg(msg, WARN_MSG_SIZE, ahead, answer, priv->env_from, why, tag,
                   priv);
    if (smfi_replacebody(ctx, (unsigned char *) msg, strlen(msg)) != 0)
      ZE_LogMsgError(0, "Erreur replacing Body Message");
  } else
    ZE_LogSysError("Error malloc ing msg buf");

  FREE(msg);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

void
read_error_msg(buf, sz, ahead, answer, from, why, tag, priv)
     char               *buf;
     int                 sz;
     attachment_T       *ahead;
     char               *answer;
     char               *from;
     char               *why;
     char               *tag;
     CTXPRIV_T          *priv;
{
  FILE               *fin;
  char                line[JCOMBUFSZ];
  char                s1[JCOMBUFSZ];

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  char               *fname;
  char                path[1024];
  char               *cfdir = NULL;

  cfdir = cf_get_str(CF_CONFDIR);
  if (cfdir == NULL || strlen(cfdir) == 0)
    cfdir = ZE_CONFDIR;

  fname = cf_get_str(CF_ERROR_MSG_FILE);
  ADJUST_FILENAME(path, fname, cfdir, ZE_ERROR_MSG_FILE);

  MUTEX_LOCK(&mutex);

  *buf = '\0';
  if ((fin = fopen(path, "r")) != NULL) {
    regex_t             re;
    regmatch_t          pm;
    bool                compile_ok;
    int                 state = 0;

    char                tag_begin[256];
    char                tag_end[256];

    snprintf(tag_begin, sizeof (tag_begin), "<%s>", tag);
    snprintf(tag_end, sizeof (tag_end), "</%s>", tag);
    compile_ok = !regcomp(&re, "_+[A-Z]+_+", REG_ICASE | REG_EXTENDED);

    while (((strlen(buf) + sizeof (line)) < sz) &&
           (fgets(line, sizeof (line), fin) != NULL)) {

      char               *p = line + strlen(line) - 1;

      if (*p == '\n')
        *p = '\0';
      strcat(line, CRLF);

      if (state == 0) {
        p = line + strspn(line, " \t");
        if (*p == '#')
          continue;

        if (strstr(line, tag_begin) == NULL)
          continue;
        state = 1;
        continue;
      } else {
        if (strstr(line, tag_end) != NULL)
          break;
      }

      if (compile_ok && !regexec(&re, line, 1, &pm, 0)) {
        char               *p = s1;

        /*
         * int  len = pm.rm_eo - pm.rm_so; 
         */
        int                 j;

        for (j = pm.rm_so; j < pm.rm_eo; j++) {
          if (line[j] != '_')
            *p++ = line[j];
        }
        *p = '\0';
        if (strcasecmp(s1, "MSGID") == 0) {
          char                filename[256];
          char               *suffix = STRNULL(priv->fsuffix, SUFFIX_UNKNOWN);

          if (priv->fname == NULL)
            continue;

          /*
           * delete directory part from filename 
           */
          if (zeMyBasename(filename, priv->fname, sizeof (filename)) ==
              filename)
            snprintf(line, sizeof (line), "  **** MSG ID : %s%s", filename,
                     suffix);
          else
            snprintf(line, sizeof (line), "  **** MSG ID : %s%s", priv->fname,
                     suffix);

          strcat(buf, line);
          strcat(buf, CRLF);
          continue;
        }
        if (strcasecmp(s1, "FROM") == 0) {
#if 1
          from = STRNULL(priv->hdr_from, priv->env_from);
          from = STRNULL(from, "Unknown sender");
#else
          from = priv->env_from ? priv->env_from : "???";
#endif
          snprintf(line, sizeof (line), "  **** From : %s", from);
          strcat(buf, line);
          strcat(buf, CRLF);
          continue;
        }
        if (strcasecmp(s1, "TO") == 0) {
          char               *to = NULL;

          if (priv != NULL)
            to = priv->hdr_to;
          to = STRNULL(to, "???");
          snprintf(line, sizeof (line), "  **** To   : %s", to);
          strcat(buf, line);
          strcat(buf, CRLF);
          continue;
        }
        if (strcasecmp(s1, "SUBJECT") == 0) {
          char               *p;

          p = priv->hdr_subject != NULL ? priv->hdr_subject : "-- NULL --";
          snprintf(line, sizeof (line), "  **** Subject : %s", p);
          strlcat(buf, line, sz);
          strlcat(buf, CRLF, sz);
          continue;
        }
        if (strcasecmp(s1, "SMTP-PEER") == 0) {
          char               *p;

          p = priv->peer_name != NULL ? priv->peer_name : priv->peer_addr;
          snprintf(line, sizeof (line), "  **** Peer : %s", p);
          strlcat(buf, line, sz);
          strlcat(buf, CRLF, sz);
          continue;
        }
        if (strcasecmp(s1, "WHY") == 0) {
          strlcpy(line, why, sizeof (line));
          strlcat(buf, line, sz);
          strlcat(buf, CRLF, sz);
          continue;
        }
        if (strcasecmp(s1, "VIRUS") == 0) {
          snprintf(line, sizeof (line), "  VIRUS FOUND : %s",
                   answer ? answer : "NONE");
          strlcat(buf, line, sz);
          strlcat(buf, CRLF, sz);
          continue;
        }
        if (strcasecmp(s1, "TO") == 0)
          continue;
        if (strcasecmp(s1, "ATTACHMENT") == 0 && ahead != NULL) {
          attachment_T       *p = ahead;
          int                 nb = 0;

          if (p == NULL) {
            snprintf(line, sizeof (line),
                     "      There isn't attached files !!!");
            strlcat(buf, line, sz);
            strlcat(buf, CRLF, sz);
            continue;
          }
          while (p != NULL) {
            char               *serror;

            serror = STRBOOL(p->xfile, "SUSPECT", "CLEAN");
            snprintf(line, sizeof (line), "  **** (%-11s) : %s", serror,
                     p->name);
            strlcat(buf, line, sz);
            strlcat(buf, CRLF, sz);
            if (p->mimetype != NULL) {
              sprintf(line, "        TYPE         : %s", p->mimetype);
              strlcat(buf, line, sz);
              strlcat(buf, CRLF, sz);
            }
            p = p->next;
          }
          if (nb > 0) {
            strlcat(buf, CRLF, sz);
            snprintf(line, sizeof (line), "  **** SUSPECT FILES : %d", nb);
            strlcat(buf, line, sz);
            strlcat(buf, CRLF, sz);
          }
          continue;
        }
      } else
        strlcat(buf, line, sz);
    }
    fclose(fin);
    regfree(&re);
    if (state == 0)
      ZE_LogMsgError(0, "read_error_msg : msg for tag %s not found",
                     STRNULL(tag, ""));
  } else {
    char               *s = "";

    ZE_LogSysError("Error opening %s file", STRNULL(path, ""));

    s = "\n"
      " This message was verified by the filtering system of our mail server.\n"
      "\n"
      "Original message was replaced by this one by the following reason :\n"
      "\n"
      "*** A suspicious file (executable code) was found in the message !\n"
      "\n";

    strlcpy(buf, s, sz);
  }

  /*
   * Let's add a copyright footer 
   */
  if (cf_get_int(CF_FOOTER) == OPT_SHOW) {
    char                s[128];
    char               *msg =
      "ze-filter - (c) Ecole des Mines de Paris 2002, ...";

    zeStrSet(s, '-', 64);
    strlcat(buf, s, sz);
    strlcat(buf, CRLF, sz);
    zeStrCenter(s, msg, 64);
    strlcat(buf, s, sz);
    strlcat(buf, CRLF CRLF, sz);
  }
  MUTEX_UNLOCK(&mutex);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void
strlcpy_replace(out, in, sz, except, replace)
     char               *out;
     char               *in;
     size_t              sz;
     char               *except;
     char                replace;
{
  char               *p;

  if (in != out)
    strlcpy(out, in, sz);
  if (strchr(except, replace) != NULL)
    replace = ' ';
  while ((p = strpbrk(out, except)) != NULL)
    *p = replace;
}

void
log_msg_context(ctx, why)
     SMFICTX            *ctx;
     char               *why;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  char               *smid = NULL;
  char               *id = NULL;

  char                buf[1024];
  char                st[1024];
  char                stc[1024];
  char               *pc = NULL;

  if (priv == NULL)
    return;

  id = CONNID_STR(priv->id);
  if (id == NULL || strlen(id) == 0)
    id = "NOID";

  smid = priv->sm_msgid;
  smid = STRNULL(smid, "NOID");
  why = STRNULL(why, "UNKNOWN");

  memset(buf, 0, sizeof (buf));
  memset(st, 0, sizeof (st));

  strlcpy(buf, st, sizeof (buf));

  snprintf(buf, sizeof (buf), "%12s : SMQID=(%s)", id, smid);

  if (1) {
    snprintf(st, sizeof (st), ", Txn#=(%d)", priv->nb_from);
    strlcat(buf, st, sizeof (buf));
  }

  pc = callback_name(priv->callback_id);
  if (pc != NULL && strlen(pc) > 0) {
    snprintf(st, sizeof (st), ", Callback=(%s)", pc);
    strlcat(buf, st, sizeof (buf));
  }

  {
    char               *name, *port, *addr;

    name = sm_macro_get_str(priv->sm, "{daemon_name}");
    name = STRNULL(name, "*");
    addr = sm_macro_get_str(priv->sm, "{if_addr}");
    if (addr == NULL)
      addr = sm_macro_get_str(priv->sm, "{daemon_addr}");
    addr = STRNULL(addr, "*");
    port = sm_macro_get_str(priv->sm, "{daemon_port}");
    port = STRNULL(port, "*");

    snprintf(st, sizeof (st), ", Daemon=(%s;%s;%s)", name, addr, port);
    strlcat(buf, st, sizeof (buf));
  }

  if (why != NULL) {
    snprintf(st, sizeof (st), ", Why=(%s)", why);
    strlcat(buf, st, sizeof (buf));
  }

  if (priv->peer_addr != NULL && strlen(priv->peer_addr) > 0) {
    snprintf(st, sizeof (st), ", PeerAddr=(%s)", priv->peer_addr);
    strlcat(buf, st, sizeof (buf));
  }

  if (priv->peer_name != NULL && strlen(priv->peer_name) > 0) {
    snprintf(st, sizeof (st), ", PeerName=(%s)", priv->peer_name);
    strlcat(buf, st, sizeof (buf));
  }

  pc = CTX_NETCLASS_LABEL(priv);
  if (pc != NULL && strlen(pc) > 0) {
    snprintf(st, sizeof (st), ", NetClass=(%s)", pc);
    strlcat(buf, st, sizeof (buf));
  }
#if 0
  if (priv->callback_id == CALLBACK_EHLO || priv->callback_id == CALLBACK_MAIL)
#endif
  {
    if (priv->helohost != NULL && strlen(priv->helohost) > 0) {
      snprintf(st, sizeof (st), ", Ehlo=(%s)", priv->helohost);
      strlcat(buf, st, sizeof (buf));
    }
  }

  if (priv->env_from != NULL && strlen(priv->env_from) > 0) {
    snprintf(st, sizeof (st), ", MAIL=(%s)", priv->env_from);
    strlcat(buf, st, sizeof (buf));
  }

  if (priv->callback_id == CALLBACK_RCPT) {
    snprintf(st, sizeof (st), ", NbRCPT=(%d)", priv->env_nb_rcpt);
    strlcat(buf, st, sizeof (buf));

    snprintf(st, sizeof (st), ", RCPT=(%s)", STRNULL(priv->env_to, "NULL"));
    strlcat(buf, st, sizeof (buf));
  }

  if (priv->callback_id <= CALLBACK_RCPT) {
#if 0
    pc = NULL;
    switch (reply) {
      case SMFIS_CONTINUE:
        pc = "continue";
        break;
      case SMFIS_TEMPFAIL:
        pc = "tempfail";
        break;
      case SMFIS_REJECT:
        pc = "reject";
        break;
      case SMFIS_ACCEPT:
        pc = "accept";
        break;
    }
#endif
    if (priv->reply_code != NULL) {
      snprintf(st, sizeof (st), ", Reply=(%s)", priv->reply_code);
      strlcat(buf, st, sizeof (buf));
    }
    ZE_MessageNotice(8, "%s", buf);

    goto fin;
  }

  if (priv->callback_id > CALLBACK_RCPT) {
    rcpt_addr_T        *p;
    char                sbuf[1024];
    int                 nr = 0;

    for (p = priv->env_rcpt; p != NULL; p = p->next) {
      nr++;

      if (p->access != RCPT_OK)
        continue;

      strlcpy(sbuf, buf, sizeof (sbuf));

      snprintf(st, sizeof (st), ", NbRCPT=(%d/%d)", nr, priv->env_nb_rcpt);
      strlcat(sbuf, st, sizeof (sbuf));

      if (p->rcpt != NULL && strlen(p->rcpt) > 0) {
        snprintf(st, sizeof (st), ", RCPT=(%s)", p->rcpt);
        strlcat(sbuf, st, sizeof (sbuf));
      }

      if (priv->hdr_from != NULL && strlen(priv->hdr_from) > 0) {
        unsigned char      *p = (unsigned char *) priv->hdr_from;
        char               *q;

        while (*p != '\0') {
          if (*p > 0x7F || *p < 0x20)
            *p = '.';
          p++;
        }

        while ((q = strchr(priv->hdr_from, '[')) != NULL)
          *q = '\'';
        while ((q = strchr(priv->hdr_from, ']')) != NULL)
          *q = '\'';

        snprintf(st, sizeof (st), ", HeaderFrom=(%s)", priv->hdr_from);
        strlcat(sbuf, st, sizeof (sbuf));
      }

      if (priv->hdr_mailer != NULL && strlen(priv->hdr_mailer) > 0) {
        unsigned char      *p = (unsigned char *) priv->hdr_mailer;
        char               *q;

        while (*p != '\0') {
          if (*p > 0x7F || *p < 0x20)
            *p = '.';
          p++;
        }

#if 0
        while ((q = strchr(priv->hdr_mailer, '[')) != NULL)
          *q = '\'';
        while ((q = strchr(priv->hdr_mailer, ']')) != NULL)
          *q = '\'';

        snprintf(st, sizeof (st), ", Mailer=(%s)", priv->hdr_mailer);
#else
        strlcpy(stc, priv->hdr_mailer, sizeof (stc));
        strlcpy_replace(stc, stc, sizeof (stc), "[]", '\'');
        strlcpy_replace(stc, stc, sizeof (stc), "()", '.');
        snprintf(st, sizeof (st), ", Mailer=(%s)", stc);
#endif

        strlcat(sbuf, st, sizeof (sbuf));
      }

      if (priv->callback_id == CALLBACK_EOM) {
#if 0
        int                 gScore = 0;
        int                 iscore = 0;

        extern int          combine_scores(msg_scores_T *);

        if (priv->rawScores.do_bayes)
          gScore = combine_scores(&priv->netScores);
        else
          gScore = priv->netScores.body + priv->netScores.headers +
            priv->netScores.oracle + priv->netScores.urlbl;

        iscore =
          priv->netScores.body + priv->netScores.headers +
          priv->netScores.oracle + priv->netScores.urlbl;
#endif
        if (priv->rawScores.spam) {
          snprintf(st, sizeof (st),
                   ", Scores=(R=%d U=%d O=%d B=%5.3f -> %6.3f)",
                   priv->rawScores.body + priv->rawScores.headers,
                   priv->rawScores.urlbl, priv->rawScores.oracle,
                   priv->rawScores.bayes, priv->rawScores.combined);
          strlcat(sbuf, st, sizeof (sbuf));
        }
      }

      if (priv->msg_size >= 0) {
        snprintf(st, sizeof (st), ", Size=(%ld)", priv->msg_size);
        strlcat(sbuf, st, sizeof (sbuf));
      }

      if (priv->reply_code != NULL) {
        snprintf(st, sizeof (st), ", Reply=(%s)", priv->reply_code);
        strlcat(sbuf, st, sizeof (sbuf));
      }

      if (priv->save_msg) {
        snprintf(st, sizeof (st), ", QuarantineFile=(%s)",
                 STRNULL(priv->fname, "NOFILE"));
        strlcat(sbuf, st, sizeof (sbuf));
      }

      ZE_MessageNotice(8, "%s", sbuf);
    }
  }

fin:
  return;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define   UNDEF_TAG    "[SUSPECT]"

bool
add_tag2subject(ctx, tag)
     SMFICTX            *ctx;
     char               *tag;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  char               *osubject = NULL;
  bool                result = TRUE;
  char               *ntag = UNDEF_TAG;

  if (tag != NULL && strlen(tag) > 0)
    ntag = tag;

  osubject = STRNULL(priv->hdr_subject, "");
#if 0
  if (!zeStrRegex(osubject, ntag, NULL, NULL, TRUE))
#else
  if (strstr(osubject, ntag) == NULL)
#endif
  {
    char               *nsubject = NULL;
    size_t              sz;

    sz = strlen(osubject) + strlen(ntag) + 8;
    if ((nsubject = (char *) malloc(sz)) != NULL) {
      snprintf(nsubject, sz, "%s %s", ntag, osubject);
      result = (smfi_chgheader(ctx, "Subject", 1, nsubject) == MI_SUCCESS);
      FREE(nsubject);
    } else {
      ZE_LogSysError("malloc error");
      result = FALSE;
    }
  }

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static pthread_mutex_t scf_mutex = PTHREAD_MUTEX_INITIALIZER;
static int          scf_fd = -1;
static int          scf_nerr = 0;

#define  ZE_SERIES_FNAME    ZE_WORKDIR "/ze-series.txt"

bool
open_scores4stats_file()
{
  bool                ret = TRUE;
  char               *fname = ZE_SERIES_FNAME;

  if (scf_fd < 0) {
    scf_fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (scf_fd < 0) {
      ZE_LogSysError("Error opening %s", fname);
      scf_nerr++;
      ret = FALSE;
    } else
      scf_nerr = 0;
  }
  return ret;
}

bool
reopen_scores4stats_file()
{
  bool                ret = TRUE;
  char               *fname = ZE_SERIES_FNAME;

  MUTEX_LOCK(&scf_mutex);
  if (scf_fd >= 0) {
    close(scf_fd);
    scf_fd = -1;
  }

  scf_fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (scf_fd < 0) {
    ZE_LogSysError("Error opening %s", fname);
    scf_nerr++;
    ret = FALSE;
  } else
    scf_nerr = 0;

  MUTEX_UNLOCK(&scf_mutex);

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define GLOBAL_CLASS(s)   ((s) >= 4 ? "FORT": ((s) > 1 ? "FAIBLE" : "HAM"))

#define BAYES_CLASS(s)    ((s) >= BSCORE_HI ? "SPAM": ((s) > BSCORE_LO ? "UNSURE" : "HAM"))

#define SPAM_CLASS(s)    ((s) >= BSCORE_HI ? "SPAM" : (((s) >= BSCORE_MI ? "LO" : ((s) > 0.25 ? "UNSURE" : "HAM"))))

bool
dump_msg_scores4stats(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  char               *fname = ZE_SERIES_FNAME;
  int                 gScore;
  char                buf[1024];
  char                from[256];

#if 0
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  static int          scf_fd = -1;
  static int          scf_nerr = 0;
#endif

  ASSERT(priv != NULL);

  MUTEX_LOCK(&scf_mutex);
  if (scf_fd < 0) {
    scf_fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (scf_fd < 0) {
      ZE_LogSysError("Error opening %s", fname);
      scf_nerr++;
    }
  }

  if (scf_fd >= 0) {
    msg_scores_T       *scp;

    scp = &priv->rawScores;

#if 0
    gScore = combine_scores(&priv->netScores);
#endif
    memset(from, 0, sizeof (from));
    extract_email_address(from, priv->env_from, sizeof (from));

#if 1
    gScore = scp->scale.kf1 * (scp->combined - scp->scale.kf0);

#if 1
    snprintf(buf, sizeof (buf),
             "%10ld SCORE=%.3f GSCORE=%d CLASS=%s B=%.3f R=%d O=%d U=%d IP=%s FROM=%s\n",
             time(NULL),
             scp->combined,
             gScore, SPAM_CLASS(scp->combined),
             scp->bayes,
             scp->body + scp->headers,
             scp->oracle, scp->urlbl, priv->peer_addr, priv->env_from);
#else
    snprintf(buf, sizeof (buf),
             "%10ld SCORE=%5d %-6s %6.3f %-6s %5d %5d %5d %-16s %s\n",
             time(NULL),
             gScore, GLOBAL_CLASS(gScore), scp->bayes,
             BAYES_CLASS(scp->bayes),
             scp->body + scp->headers,
             scp->oracle, scp->urlbl, priv->peer_addr, priv->env_from);
#endif
#else
    snprintf(buf, sizeof (buf),
             "%10ld %5d %-6s %6.3f %-6s %5d %5d %5d %-16s %s\n", time(NULL),
             gScore, GLOBAL_CLASS(gScore), priv->rawScores.bayes,
             BAYES_CLASS(priv->rawScores.bayes),
             priv->rawScores.body + priv->rawScores.headers,
             priv->rawScores.oracle, priv->rawScores.urlbl, priv->peer_addr,
             priv->env_from);
#endif

    if (write(scf_fd, buf, strlen(buf)) < strlen(buf)) {
      ZE_LogSysError("Error writing into file %s", fname);
      close(scf_fd);
      scf_fd = -1;
      scf_nerr++;
    } else
      scf_nerr = 0;
  }
  MUTEX_UNLOCK(&scf_mutex);

  return TRUE;
}
