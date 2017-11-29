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
 *  Creation     : Thu May 28 17:51:54 CEST 2009
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
#include <ze-lr-funcs.h>


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

# define LR_BODY_LENGTH        256
# define LR_RAW_LENGTH         2500

typedef struct
{
  bool                ok;
  JBT_T               bt;
  int                 dummy;
}
DATA_T;

#define DATA_INIT {FALSE, JBT_INITIALIZER}

static bool         lr_task(char *id,
                            char *fname, lr_cargs_T * cargs, lr_margs_T * margs,
                            test_score_T * mscore, int task, bool learn, bool spam);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

typedef struct
{
  union
  {
    uint32_t            utok;
    char                xtok[8];
  } tok;

  int                 nb;
  int                 nbs;
  int                 nbh;
  double              weight;
} lrtok_T;

static int
lrtokcmp(void *a, void *b)
{
  lrtok_T            *ta = (lrtok_T *) a;
  lrtok_T            *tb = (lrtok_T *) b;

  if (ta->tok.utok > tb->tok.utok)
    return 1;
  if (ta->tok.utok < tb->tok.utok)
    return -1;
  return 0;
}

#define LRATE   0.004

static double       lrate = LRATE;

typedef struct
{
  bool                ok;
  pthread_mutex_t     mutex;
  JBT_T               lrbt;

  char               *fname;    /* data file name */

  int                 ns;       /* count of spams */
  int                 nh;       /* count of hams */

  int                 nsu;      /* count of (unbalanced) spams */
  int                 nhu;      /* count of (unbalanced) hams */

  lr_callback_F       learn_callback;

  /* options */
  lr_opts_T           opts;

} LR_T;

#define LR_INITIALIZER							\
  {									\
    FALSE, PTHREAD_MUTEX_INITIALIZER, JBT_INITIALIZER,			\
      NULL,								\
      0, 0, 0, 0,							\
      NULL, LR_OPTS_INITIALIZER}

static LR_T         lr_data = LR_INITIALIZER;


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
lr_data_read(bt, fname)
     JBT_T              *bt;
     char               *fname;
{
  FILE               *fin = NULL;
  bool                inHead = FALSE, inData = FALSE;
  int                 nl = 0;
  char                buf[1024];

  if (fname == NULL)
    return -1;

  fin = fopen(fname, "r");
  if (fin == NULL)
  {
    ZE_LogSysError("fopen %s", fname);
    return -1;
  }
  while (fgets(buf, sizeof (buf), fin) != NULL)
  {
    nl++;
    (void) strchomp(buf);

    if (inHead)
    {
      char               *argv[8];
      int                 argc;

      if (strexpr(buf, "</head>", NULL, NULL, TRUE))
      {
        inHead = FALSE;
        continue;
      }
      argc = str2tokens(buf, 8, argv, "=: ");
      if (argc >= 2)
      {
        if (STRCASEEQUAL(argv[0], "count"))
        {
          continue;
        }
        if (STRCASEEQUAL(argv[0], "spams"))
        {
          lr_data.ns = atoi(argv[1]);
          continue;
        }
        if (STRCASEEQUAL(argv[0], "hams"))
        {
          lr_data.nh = atoi(argv[1]);
          continue;
        }
        if (STRCASEEQUAL(argv[0], "usebody"))
        {
          lr_data.opts.useBody = TRUE;
          if (STRCASEEQUAL(argv[1], "NO"))
            lr_data.opts.useBody = FALSE;
          if (STRCASEEQUAL(argv[1], "FALSE"))
            lr_data.opts.useBody = FALSE;
          continue;
        }
        if (STRCASEEQUAL(argv[0], "useheaders"))
        {
          lr_data.opts.useHeaders = TRUE;
          if (STRCASEEQUAL(argv[1], "NO"))
            lr_data.opts.useHeaders = FALSE;
          if (STRCASEEQUAL(argv[1], "FALSE"))
            lr_data.opts.useHeaders = FALSE;
          continue;
        }
        if (STRCASEEQUAL(argv[0], "bodylength"))
        {
          size_t              len;

          len = str2size(argv[0], NULL, LR_BODY_LENGTH);
          if (len > 0)
            lr_data.opts.bodyLength = len;
          continue;
        }
      }
    }

    if (inData)
    {
      lrtok_T             tok;

      if (strexpr(buf, "</data>", NULL, NULL, TRUE))
      {
        inData = FALSE;
        continue;
      }

      memset(&tok, 0, sizeof (tok));
      if (sscanf(buf, "%x %lg %d %d %d", &tok.tok.utok, &tok.weight,
                 &tok.nb, &tok.nbs, &tok.nbh) == 5)
      {
        if (!jbt_add(bt, &tok))
        {

        }
        continue;
      }
      ZE_MessageNotice(10, "# error %d : %s", nl, buf);
      continue;
    }

    if (strexpr(buf, "<head>", NULL, NULL, TRUE))
    {
      inHead = TRUE;
      continue;
    }
    if (strexpr(buf, "<data>", NULL, NULL, TRUE))
    {
      inData = TRUE;
      continue;
    }

  }
  fclose(fin);

  return nl;
}

bool
lr_data_open(fname)
     char               *fname;
{
  bool                result = FALSE;

  if (lr_data.ok)
    return TRUE;

  MUTEX_LOCK(&lr_data.mutex);
  if (!lr_data.ok)
  {
    char               *env = NULL;

    (void) jbt_init(&lr_data.lrbt, sizeof (lrtok_T), lrtokcmp);

    /* JOE XXX hmm ... and if fname is NULL ??? */
    if ((lr_data.fname = strdup(fname)) == NULL)
    {
      goto fin;
    }

    if (lr_data_read(&lr_data.lrbt, fname) < 0)
#if 0
      goto fin;
#else
      ;
#endif

    lr_data.ok = TRUE;

    lr_data.opts.tok_type = 0;
    lr_data.opts.tok_len = 4;

    lr_data.opts.lrate = LRATE;
    lr_data.learn_callback = NULL;

    if ((env = getenv("LR_LRATE")) != NULL)
    {
      double              rate;

      rate = str2double(env, NULL, lrate);
      if (rate > 0.)
      {
        lrate = rate;
        lr_data.opts.lrate = rate;
      }
    }

    if ((env = getenv("LR_USE_RAW_MSG")) != NULL)
    {
      if (strexpr(env, "yes|true|oui", NULL, NULL, TRUE))
        lr_data.opts.useRawMsg = TRUE;
    }

    if ((env = getenv("LR_RAW_LENGTH")) != NULL)
    {
      size_t              len;

      len = str2size(env, NULL, LR_RAW_LENGTH);
      if (len > 0)
        lr_data.opts.rawLength = len;
    }

    if ((env = getenv("LR_BODY_LENGTH")) != NULL)
    {
      size_t              len;

      len = str2size(env, NULL, LR_BODY_LENGTH);
      if (len > 0)
        lr_data.opts.bodyLength = len;
    }

    if ((env = getenv("LR_USE_BODY")) != NULL)
    {
      lr_data.opts.useBody = TRUE;
      if (STRCASEEQUAL(env, "NO"))
        lr_data.opts.useBody = FALSE;
      if (STRCASEEQUAL(env, "FALSE"))
        lr_data.opts.useBody = FALSE;
    }

    if ((env = getenv("LR_USE_HEADERS")) != NULL)
    {
      lr_data.opts.useHeaders = TRUE;
      if (STRCASEEQUAL(env, "NO"))
        lr_data.opts.useHeaders = FALSE;
      if (STRCASEEQUAL(env, "FALSE"))
        lr_data.opts.useHeaders = FALSE;
    }

    result = TRUE;
  }
fin:
  MUTEX_UNLOCK(&lr_data.mutex);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_data_close()
{
  if (!lr_data.ok)
    return TRUE;

  MUTEX_LOCK(&lr_data.mutex);
  if (lr_data.ok)
  {
    (void) jbt_destroy(&lr_data.lrbt);

    FREE(lr_data.fname);
    lr_data.ok = FALSE;
  }
  MUTEX_UNLOCK(&lr_data.mutex);
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_data_show_conf()
{
  if (!lr_data.ok)
    return TRUE;

  MUTEX_LOCK(&lr_data.mutex);
  if (lr_data.ok)
  {
    ZE_MessageInfo(10, "Modele (fname)                 %s", lr_data.fname);
#if 0

    ZE_MessageInfo("Token length                   %7d", lr_data.tlen);
    ZE_MessageInfo("Token type                     %7d", lr_data.ttype);
    int                 tlen;   /* token len */
    int                 ttype;  /* token type */

    int                 ns;     /* count of spams */
    int                 nh;     /* count of hams */

    int                 nsu;    /* count of (unbalanced) spams */
    int                 nhu;    /* count of (unbalanced) hams */

    bool                useRawMsg;
    size_t              rawLength;

    bool                useHeaders;
    bool                useBody;
    size_t              bodyLength;
    bool                cleanUpHeaders;
    bool                cleanUpDates;

    double              lrate;
    lr_lrate_F          lrate_function;

    bool                active_learning;
    double              active_threshold;
#endif
  }
  MUTEX_UNLOCK(&lr_data.mutex);
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
lr_browse_dump(vtok, varg)
     void               *vtok;
     void               *varg;
{
  lrtok_T            *tok = (lrtok_T *) vtok;
  FILE               *fout = varg;
#if 0
  int                 tlen = lr_data.opts.tok_len;
#endif
  fprintf(fout, "%08lx %lg %5d %5d %5d\n", (long unsigned int) tok->tok.utok, tok->weight,
          tok->nb, tok->nbs, tok->nbh);

  return 1;
}

bool
lr_data_dump(fname)
     char               *fname;
{
  if (!lr_data.ok)
    return FALSE;

  MUTEX_LOCK(&lr_data.mutex);
  if (lr_data.ok)
  {
    FILE               *fout = NULL;
    int                 nb;
    JBT_T              *bt = &lr_data.lrbt;
    time_t              now;

    fout = fopen(fname, "w");
    if (fout == NULL)
      goto fin;

    now = time(NULL);

    fprintf(fout, "<HEAD>\n");
    fprintf(fout, "date=%ld\n", now);
    fprintf(fout, "toktype=%d\n", 1);
    fprintf(fout, "toklength=%d\n", 4);
    fprintf(fout, "count=%d\n", jbt_count(bt));
    fprintf(fout, "spams=%d\n", lr_data.ns);
    fprintf(fout, "hams=%d\n", lr_data.nh);
    fprintf(fout, "spamsu=%d\n", lr_data.nsu);
    fprintf(fout, "hamsu=%d\n", lr_data.nhu);
    fprintf(fout, "usebody=%s\n", STRBOOL(lr_data.opts.useBody, "YES", "NO"));
    fprintf(fout, "useheaders=%s\n",
            STRBOOL(lr_data.opts.useHeaders, "YES", "NO"));
    fprintf(fout, "bodylength=%ld\n", lr_data.opts.bodyLength);
    fprintf(fout, "</HEAD>\n");
    fprintf(fout, "<DATA>\n");
    nb = jbt_browse(bt, lr_browse_dump, fout);
    fprintf(fout, "</DATA>\n");
    fclose(fout);
  fin:
    ;
  }
  MUTEX_UNLOCK(&lr_data.mutex);
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if 0
static char        *uheaders[] = {
  "xxx-Message-ID",
  "xxx-Date",
  "xxx-To",
  "X-ze-filter-score",
  "X-ze-filter-status",
  "X-Miltered",
  "X-Sieve",
  "X-DKIM",
  "DKIM-Signature",
  "DomainKey-Signature",
  "Authentication-Results",
  "xxx-MIME-Version",
  "Received-SPF",
  "X-Greylist",
  "X-Virus-Scanned",
  "X-Virus-Status",
  "xxx-X-Spam-Status",
  NULL
};
#else
static char        *uheaders[] = {
  "Message-ID",
  "Date",
  "Toxxxxxxx",
  "X-ze-filter-score",
  "X-ze-filter-status",
  "X-ze-filter-Enveloppe",
  "X-ze-filter-file",
  "X-Miltered",
  "X-Sieve",
  "X-DKIM",
  "DKIM-Signature",
  "DomainKey-Signature",
  "Authentication-Results",
  "MIME-Version",
  "Received-SPF",
  "X-Greylist",
  "X-Virus-Scanned",
  "X-Virus-Status",
  "X-Antivirus",
  "X-Antivirus-Status",
  "X-Spam-Status",
  "X-DSPAM-Check",
  "X-DSPAM-Confidence",
  "X-DSPAM-Factors",
  "X-DSPAM-Improbability",
  "X-DSPAM-Probability",
  "X-DSPAM-Processed",
  "X-DSPAM-Result",
  "X-DSPAM-Signature",
  NULL
};
#endif

static char        *mymtas[] = {
  "by .*.ensmp.fr",
  "by .*.mines-paristech.fr",
  "from .*.ensmp.fr",
  "from .*.mines-paristech.fr",
  "from .*.cru.fr",
  "by .*.cru.fr",
  "from .*.renater.fr",
  "by .*.renater.fr",
  NULL
};

#define DATE_EXPR     "(Sun|Mon|Tue|Wed|Thu|Fri|Sat)?,? +[0-9]+ " \
  "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) +20[0-9]{2}" \
  " +([0-9]{1,2}:[0-9]{2}:[0-9]{2} +([-+]?[0-9]{2}00)?)?"


static              bool
scan_msg_part(bt, s)
     JBT_T              *bt;
     char               *s;
{
  int                 i, slen, tlen = 4;

  slen = strlen(s);

  strlcat(s, "    ", 4);
  for (i = 0; i <= slen - tlen; i++)
  {
    lrtok_T             token, *t;

    uint32_t            tok;

    tok = 0;
    tok += ((uint32_t) s[i]) << 24;
    tok += ((uint32_t) s[i + 1]) << 16;
    tok += ((uint32_t) s[i + 2]) << 8;
    tok += ((uint32_t) s[i + 3]);
    memset(&token, 0, sizeof (token));
    token.tok.utok = tok;

    token.nb = 1;
    if ((t = jbt_get(bt, &token)) != NULL)
      t->nb++;
    else
      (void) jbt_add(bt, &token);
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
bt_browse_classify(void *vtok, void *varg)
{
  double             *score = (double *) varg;
  lrtok_T            *ptok = vtok;
#if 0
  lrtok_T             tok;

  tok = *((lrtok_T *) vtok);
#endif
  if ((ptok = jbt_get(&lr_data.lrbt, vtok)) != NULL)
    *score += ptok->weight;

  return 1;
}


static int
bt_browse_dump_tokens(void *vtok, void *varg)
{
  lrtok_T            *tok = (lrtok_T *) vtok;
  char               *id = (char *) varg;

  id = (id != NULL ? id : "000");

  printf("%-14s %08lx\n", id, (long unsigned int) tok->tok.utok);
  return 1;
}


static int
bt_browse_adjust(void *vtok, void *varg)
{
  lrtok_T            *ptok = vtok, tok;
  double             *delta = (double *) varg;

  tok = *((lrtok_T *) vtok);
  if ((ptok = jbt_get(&lr_data.lrbt, &tok)) == NULL)
  {
    tok.weight += *delta;
    tok.nb = 1;
    if (*delta > 0)
      tok.nbs++;
    else
      tok.nbh++;
    if (!jbt_add(&lr_data.lrbt, &tok))
    {
    }
  } else
  {
    ptok->weight += *delta;
    ptok->nb++;
    if (*delta > 0)
      ptok->nbs++;
    else
      ptok->nbh++;
  }

  return 1;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static              bool
tokens_mime_part(buf, size, id, level, type, arg, mpart)
     char               *buf;
     size_t              size;
     char               *id;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mpart;
{
  DATA_T             *data = (DATA_T *) arg;
#if 0
  char               *mtype = "TEXT";
#endif
  rfc2822_hdr_T      *h;
  char                vbuf[1024];

  bool                clHeaders = TRUE;
  bool                clDates = TRUE;

  if (data == NULL)
    return FALSE;

  {
    char               *env = NULL;

    if ((env = getenv("LR_CLEANUP_HEADERS")) != NULL)
    {
      clHeaders = strexpr(env, "yes|true", NULL, NULL, TRUE);
    }
    if ((env = getenv("LR_CLEANUP_DATES")) != NULL)
    {
      clDates = strexpr(env, "yes|true", NULL, NULL, TRUE);
    }
  }

  if (lr_data.opts.useHeaders)
  {
    for (h = mpart->hdrs; h != NULL; h = h->next)
    {
      char              **s = NULL;
      long                pi, pf;

      if (h->value == NULL || strlen(h->value) == 0)
	continue;

      snprintf(vbuf, sizeof (vbuf), "%s: %s", h->key, h->value);
      if (clHeaders)
      {
        for (s = uheaders; s != NULL && *s != NULL; s++)
        {
          if (STRCASEEQUAL(*s, h->key))
            break;
        }
        if (*s != NULL)
          continue;

        if (STRCASEEQUAL("Received", h->key))
        {
          for (s = mymtas; s != NULL && *s != NULL; s++)
          {
#if 1
            if (strexpr(h->value, *s, NULL, NULL, TRUE))
#else
            if (strstr(h->value, *s))
#endif
              break;
          }
          if (*s != NULL)
            continue;
        }

        if (STRCASEEQUAL("X-ze-filter-Enveloppe", h->key))
        {
          for (s = mymtas; s != NULL && *s != NULL; s++)
          {
#if 1
            if (strexpr(h->value, *s, NULL, NULL, TRUE))
#else
            if (strstr(h->value, *s))
#endif
              break;
          }
          if (*s != NULL)
            continue;
        }
      }
      snprintf(vbuf, sizeof (vbuf), "%s: %s", h->key, h->value);

      if (clDates)
      {
        while (strexpr(vbuf, DATE_EXPR, &pi, &pf, TRUE))
        {
          int                 i, lm;
          char               *p;

          lm = strlen(vbuf);
          for (i = 0, p = vbuf; i < lm; i++)
          {
            if (i < pi || i > pf)
              *p++ = vbuf[i];
          }
          *p = '\0';
        }
      }

      scan_msg_part(&data->bt, vbuf);
    }
  }

  if (lr_data.opts.useBody)
  {
    if (type != MIME_TYPE_TEXT)
      return TRUE;

    if (lr_data.opts.bodyLength > 0)
    {
      char               *pb = buf;

      strlcpy(vbuf, pb, lr_data.opts.bodyLength);
      scan_msg_part(&data->bt, vbuf);
    }
  }

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
lr_task(id, fname, cargs, margs, mscore, task, learn, spam)
     char               *id;
     char               *fname;
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
     test_score_T       *mscore;
     int                 task;
     bool                learn;
     bool                spam;
{
  DATA_T              data = DATA_INIT;
  double              score = 0.0, prob = 0.5;
  int                 nb;
  bool                result = FALSE;
  double              ptarget, delta;

  if (!lr_data.ok)
  {
    static int          nbw = 0;

    if (nbw++ < 5)
      ZE_MessageWarning(10, "lr data not yet initialized");
    return FALSE;
  }

  id = STREMPTY(id, "NOID");
  if (!jbt_init(&data.bt, sizeof (lrtok_T), lrtokcmp))
    goto fin;

  if (lr_data.opts.useRawMsg)
  {
    int                 fd = -1;
    char                buf[8192];
    size_t              nc;

    fd = open(fname, O_RDONLY);
    if (fd < 0)
    {
      ZE_LogSysError("Error opening %s", fname);
      goto fin;
    }
    memset(buf, 0, sizeof (buf));
    nc = read(fd, buf, 2500);
    if (nc < 0)
    {
      ZE_LogSysError("Error reading %s", fname);
      close(fd);
      goto fin;
    }
    close(fd);
    scan_msg_part(&data.bt, buf);
  } else
  {
    if (!decode_mime_file(id, fname, NULL, tokens_mime_part, &data))
      goto fin;
  }

  if (task == LR_TASK_EXTRACT) {
    nb = jbt_browse(&data.bt, bt_browse_dump_tokens, id);
    goto fin;
  }

  nb = jbt_browse(&data.bt, bt_browse_classify, &score);
  if (nb <= 0)
    goto fin;
  prob = 1. / (1. + exp(-score));

  if (mscore != NULL)
  {
    mscore->actif = TRUE;
    mscore->value = prob;
    mscore->odds = score;
  }
  if (margs != NULL)
  {
    margs->score.actif = TRUE;
    margs->score.value = prob;
    margs->score.odds = score;
  }

  result = TRUE;

#if 1
  if (task == LR_TASK_LEARN)
#else
  if (learn)
#endif
  {
    double              dp;

#if 0
    if (lr_data.learn_callback != NULL)
    {
      (void) lr_data.learn_callback(lr_data.nsu + lr_data.nhu, cargs, margs);
    }
#endif

    ptarget = spam ? 1.0 : 0.0;
    dp = ptarget - prob;
    if (lr_data.opts.active_learning) {
      if ((fabs(prob - 0.5) > lr_data.opts.active_margin) && (fabs(dp) < 0.5))
	goto fin;
    }

    if (margs != NULL)
      margs->query = TRUE;

    {
      double              lrate;

      lrate = lr_data.opts.lrate;
      if (lr_data.learn_callback != NULL)
      {
        lrate = lr_data.learn_callback(lr_data.nsu + lr_data.nhu, cargs, margs);
        lr_data.opts.lrate = lrate;
      }
      delta = dp * lr_data.opts.lrate;
      /* si apprentissage, correct weights */
      nb = jbt_browse(&data.bt, bt_browse_adjust, &delta);
    }

    if (spam)
      lr_data.ns++;
    else
      lr_data.nh++;

    if (margs != NULL)
    {
      if (!margs->resample)
      {
        if (spam)
          lr_data.nsu++;
        else
          lr_data.nhu++;
      }
      margs->learnt = TRUE;
      if (cargs != NULL)
        cargs->nFeatures = jbt_count(&lr_data.lrbt);
    }
    if (nb <= 0)
    {
      result = FALSE;
      goto fin;
    }
  }

fin:
  (void) jbt_destroy(&data.bt);
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_extract(id, fname, cargs, margs)
     char               *id;
     char               *fname;
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
{
  return lr_task(id, fname, cargs, margs, NULL, LR_TASK_EXTRACT, FALSE, FALSE);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_classify(id, fname, cargs, margs, mscore)
     char               *id;
     char               *fname;
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
     test_score_T       *mscore;
{
  return lr_task(id, fname, cargs, margs, mscore, LR_TASK_CLASSIFY, FALSE, FALSE);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_learn(id, fname, cargs, margs, mscore, spam)
     char               *id;
     char               *fname;
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
     test_score_T       *mscore;
     bool                spam;
{
  return lr_task(id, fname, cargs, margs, mscore, LR_TASK_LEARN, TRUE, spam);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_learn_options(active, margin)
     bool                active;
     double              margin;
{
  lr_data.opts.active_learning = active;
  lr_data.opts.active_margin = margin;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_set_options(opts)
     lr_opts_T          *opts;
{
  if (opts == NULL)
    return FALSE;

  lr_data.opts = *opts;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_get_options(opts)
     lr_opts_T          *opts;
{
  if (opts == NULL)
    return FALSE;

  *opts = lr_data.opts;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
lr_print_options(opts)
     lr_opts_T          *opts;
{
  lr_opts_T *popts = opts;

  popts = (opts != NULL ? opts : &lr_data.opts);

  printf("# lrate                 %7.3f\n"
	 "# resample              %d\n"
	 "# useRawMsg             %d\n"
	 "# rawLength             %ld\n"
	 "# bodyLength            %ld\n"
	 "# useBody               %d\n"
	 "# useHeaders            %d\n"
	 "# cleanUpHeaders        %d\n"
	 "# cleanUpDates          %d\n"
	 "# tok_type              %d\n"
	 "# tok_len               %d\n"
	 "# active_learning       %d\n"
         "# active_margin         %.3f\n"
	 "#\n",
	 popts->lrate, popts->resample, popts->useRawMsg, (long) popts->rawLength,
	 (long) popts->bodyLength, popts->useBody, popts->useHeaders,
	 popts->cleanUpHeaders, popts->cleanUpDates, popts->tok_type,
	 popts->tok_len, popts->active_learning, popts->active_margin);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
lr_set_learn_callback(funct)
     lr_callback_F       funct;
{
  lr_data.learn_callback = funct;

  return TRUE;
}
