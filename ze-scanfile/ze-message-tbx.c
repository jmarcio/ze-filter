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

#include "ze-filter.h"

extern int          ze_logLevel;

static void         usage();

static bool         cli_handle_message(char *fname, int msgNb, void *arg);

typedef struct
{
  int                 nb;
  int                 html_flags[32];
  int                 plain_flags[32];
  int                 mime_flags[32];
  int                 msg_flags[32];

  int                 checks;

  bool                spam_judgement;
  double              spam_threshold;

  double              score;
  char                header[256];

  int                 argc;
  char              **argv;
} msgtbx_T;

static bool         launch_workers(int n, char *fname, msgtbx_T * mstatp);

static bool         cli_toolbox(msgtbx_T * mstatp);


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static bool         get_msg_headers(char *, spamchk_T *);
int                 add_header(char *, spamchk_T *);

extern bool         mailregexlog2file;

#define             M_MBOX               0
#define             M_MAILDIR            1
#define             M_FILE               2
#define             M_LIST               3
#define             M_INTERACTIVE        4

int
main(int argc, char **argv)
{
  const char         *args = "hvl:m:t:b:p:c:rs:S";
  int                 c;
  int                 verbose = 0;
  char                fname[256];
  int                 level = 9;
  int                 argi;
  int                 mtype = M_MBOX;
  char               *bayesdb = NULL;

  msgtbx_T            mstat;
  int                 docheck = 0;

  char               *checks = "all";

  int                 nthreads = 1;

  double              spam_threshold = 0.75;
  bool                spam_judgement = FALSE;

  zeLog_SetOutput(FALSE, TRUE);

  mailregexlog2file = FALSE;
  ze_logLevel = 0;
  memset(fname, 0, sizeof (fname));

  while ((c = getopt(argc, argv, args)) != -1)
  {
    switch (c)
    {
      case 'h':
        usage();
        exit(0);
        break;
      case 'c':
        if (optarg != NULL && strlen(optarg) > 0)
          conf_file = optarg;
        break;
      case 'v':
        verbose++;
        break;
      case 'r':
        mailregexlog2file = TRUE;
        break;
      case 'l':
        level = atoi(optarg);
        break;
      case 'm':
        checks = optarg;
        break;
      case 'N':
        nthreads = atoi(optarg);
        if (nthreads < 1)
          nthreads = 1;
        break;
      case 'b':
        bayesdb = optarg;
        break;
      case 's':
        {
          double              v = 0;

          errno = 0;
          v = strtod(optarg, NULL);
          if (errno == 0)
            spam_threshold = v;
        }
        break;
      case 'S':
        spam_judgement = TRUE;
        break;
      case 't':
        if (STRCASEEQUAL(optarg, "mbox"))
        {
          mtype = M_MBOX;
          break;
        }
        if (STRNCASEEQUAL(optarg, "file", strlen("file")))
        {
          mtype = M_FILE;
          break;
        }
        if (STRCASEEQUAL(optarg, "maildir"))
        {
          mtype = M_MAILDIR;
          break;
        }
        if (STRNCASEEQUAL(optarg, "dir", strlen("dir")))
        {
          mtype = M_MAILDIR;
          break;
        }
        if (STRNCASEEQUAL(optarg, "list", strlen("list")))
        {
          mtype = M_LIST;
          break;
        }
        if (STRNCASEEQUAL(optarg, "interactive", strlen("interactive")))
        {
          mtype = M_INTERACTIVE;
          break;
        }
        mtype = M_MBOX;
        break;
      default:
        printf("Error ... \n");
        exit(0);
    }
  }

  if (FALSE)
  {
    usage();
    exit(1);
  }

  init_default_file_extensions();

  if (cf_opt.arg_c != NULL)
    conf_file = cf_opt.arg_c;

  configure("ze-message-toolbox", conf_file, FALSE);

  ze_logLevel = level + verbose;

  ze_logLevel = level;

  while (optind < argc && *argv[optind] == '-')
    optind++;

  if (checks != NULL && strlen(checks) > 0)
  {
    char               *p = NULL;
    int                 iargc;
    char               *iargv[8];

    if ((p = strdup(checks)) == NULL)
    {
      exit(EX_SOFTWARE);
    }

    docheck = 0;

    iargc = zeStr2Tokens(p, 8, iargv, ";, ");
    while (iargc-- > 0)
    {
      if (strncasecmp(iargv[iargc], "all", strlen(iargv[iargc])) == 0)
      {
        docheck = 0xFFFF;
        continue;
      }
      if (strncasecmp(iargv[iargc], "oracle", strlen(iargv[iargc])) == 0)
      {
        SET_BIT(docheck, 0);
        continue;
      }
      if (strncasecmp(iargv[iargc], "regex", strlen(iargv[iargc])) == 0)
      {
        SET_BIT(docheck, 1);
        continue;
      }
      if (strncasecmp(iargv[iargc], "urlbl", strlen(iargv[iargc])) == 0)
      {
        SET_BIT(docheck, 2);
        continue;
      }
      if (strncasecmp(iargv[iargc], "bayes", strlen(iargv[iargc])) == 0)
      {
        SET_BIT(docheck, 3);
        continue;
      }
    }
    FREE(p);
  }

  if (docheck == 0)
  {
    fprintf(stderr, "No checks defined...\n");
    usage();
    exit(1);
  }

  if (GET_BIT(docheck, 3))
  {
    char                path[1024];
    char               *dbname = ZE_CDBDIR "/ze-bayes.db";
    char               *cfdir;

    size_t              msgSize, partSize;
    double              rhs = 1.;

    dbname = cf_get_str(CF_DB_BAYES);
    dbname = STRNULL(bayesdb, dbname);
    ZE_MessageInfo(1, " Will open bayes database %s", dbname);
    memset(path, 0, sizeof (path));
    cfdir = cf_get_str(CF_CDBDIR);
    ADJUST_FILENAME(path, dbname, cfdir, "ze-bayes.db");
    ZE_MessageInfo(1, "           database path  %s", path);

    if (strlen(path) > 0 && !bfilter_init(path))
      ZE_MessageInfo(2, "Error while opening %s database", path);

    msgSize = cf_get_int(CF_BAYES_MAX_MESSAGE_SIZE);
    if (msgSize < 10000)
      msgSize = 400000;
    partSize = cf_get_int(CF_BAYES_MAX_PART_SIZE);
    if (partSize < 10000)
      partSize = 40000;

    (void) set_bfilter_max_sizes(msgSize, partSize);

    rhs = ((double) cf_get_int(CF_BAYES_HAM_SPAM_RATIO) / 1000);
    if (rhs < 0.1 || rhs > 10.)
      rhs = 1;

    (void) set_bfilter_ham_spam_ratio(rhs);
  }

  (void) configure_msg_eval_function(NULL);

  memset(&mstat, 0, sizeof (mstat));
  mstat.checks = docheck;
  mstat.spam_threshold = spam_threshold;
  mstat.spam_judgement = spam_judgement;

  {
    int                 nb;

    switch (mtype)
    {
      case M_MBOX:
        for (argi = optind; argi < argc; argi++)
          nb += mbox_handle(argv[argi], cli_handle_message, &mstat);
        break;
      case M_MAILDIR:
        for (argi = optind; argi < argc; argi++)
          nb += maildir_handle(argv[argi], cli_handle_message, &mstat);
        break;
      case M_FILE:
        for (argi = optind; argi < argc; argi++)
          nb += cli_handle_message(argv[argi], nb, &mstat);
        break;
      case M_LIST:
        for (argi = optind; argi < argc; argi++)
          launch_workers(nthreads, argv[argi], &mstat);
        break;
      case M_INTERACTIVE:
        cli_toolbox(&mstat);
        break;
    }
  }


  if (0 && GET_BIT(docheck, 0))
  {
    int                 i, j;
    char               *p;

    ZE_MessageInfo(8, "      MSG.. HTML. PLAIN (%d messages)\n", mstat.nb);
    for (i = 0; i < 32; i++)
      ZE_MessageInfo(7, "%3d : %5ld %5ld %5ld\n", i,
                   mstat.msg_flags[i], mstat.html_flags[i],
                   mstat.plain_flags[i]);

    for (j = ORACLE_TYPE_MSG; j <= ORACLE_TYPE_PLAIN; j++)
    {
      ZE_MessageInfo(8, "");
      for (i = 0; i < 32; i++)
      {
        if ((p = oracle_get_label(j, i)) != NULL && strlen(p) > 0)
        {
          double              v = oracle_get_score(j, i);

          ZE_MessageInfo(8, "%3d %3d : %5.2f %s", j, i, v, p);
        }
      }
    }
  }

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
inline int
add_header(h, spam)
     char               *h;
     spamchk_T          *spam;
{
  char               *f, *v;

  if ((h == NULL) || (strlen(h) == 0))
    return 0;

  f = h;
  v = h + strcspn(h, ":");
  *v = '\0';
  v++;
  v += strspn(v, " \t");

  ZE_MessageInfo(20, "Header - %s", h);

  (void) add_to_msgheader_list(&spam->hdrs, f, v);

  return 1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
get_msg_headers(fname, spam)
     char               *fname;
     spamchk_T          *spam;
{
  int                 nl = 0, nh = 0;
  char                line[1024];
  char                header[0x8000];
  FILE               *fin;

  if ((fname == NULL) || (strlen(fname) == 0))
    return FALSE;

  if ((fin = fopen(fname, "r")) == NULL)
  {
    ZE_LogSysError("fopen(%s)", fname);
    return FALSE;
  }

  memset(header, 0, sizeof (header));
  while (fgets(line, sizeof (line), fin) != NULL)
  {
    char               *s;

    if (strspn(line, "\r\n") == strlen(line))
      break;

    if ((nl == 0) && (strncasecmp("From ", line, strlen("From ")) == 0))
      continue;

    zeStrChomp(line);

    s = line + strspn(line, "\r\n");
    if ((*s != ' ') && (*s != '\t'))
    {
      nh += add_header(header, spam);
      memset(header, 0, sizeof (header));
      snprintf(header, sizeof (header), "%s", s);
    } else
    {
      char               *p = header + strlen(header);

      strncpy(p, line, strlen(s));
    }
    nl++;
  }
  nh += add_header(header, spam);

  ZE_MessageInfo(19, "Header has %d lines and %d headers", nl, nh);

  fclose(fin);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
free_msg_headers(spam)
     spamchk_T          *spam;
{
  (void) clear_msgheader_list(spam->hdrs);

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
cli_handle_message(fname, msgNb, arg)
     char               *fname;
     int                 msgNb;
     void               *arg;
{
  char               *ip = "0.0.0.0";
  char               *id = "00000000.000";

  spamchk_T           spam;
  msg_flags_T         flags;
  msg_scores_T        rScores;

  size_t              maxsize = 0x20000;

  char                bid[32];

  msgtbx_T           *mstat = (msgtbx_T *) arg;

  SHOW_CURSOR(FALSE);

  mstat->nb++;

  snprintf(bid, sizeof (bid), "%08X.000", msgNb);
  id = bid;
  id = STREMPTY(fname, id);

  memset(&spam, 0, sizeof (spam));
  memset(&flags, 0, sizeof (flags));
  memset(&rScores, 0, sizeof (rScores));

  rScores.bayes = 0.5;

  maxsize = cf_get_int(CF_SPAM_REGEX_MAX_MSG_SIZE);

  spam.ip = ip;

  if (GET_BIT(mstat->checks, 0))
    rScores.do_oracle = TRUE;
  if (GET_BIT(mstat->checks, 1))
    rScores.do_regex = TRUE;
  if (GET_BIT(mstat->checks, 2))
    rScores.do_urlbl = TRUE;
  if (GET_BIT(mstat->checks, 3))
    rScores.do_bayes = TRUE;

  fill_msg_scale(&rScores.scale);
  if (rScores.do_bayes)
  {
    sfilter_vsm_T       bcheck;

    memset(&bcheck, 0, sizeof (bcheck));

    bcheck.nbt = cf_get_int(CF_BAYES_NB_TOKENS);
    if (bcheck.nbt <= 0)
      bcheck.nbt = 21;
    rScores.bayes = sfilter_check_message(id, fname, &bcheck);

    if (rScores.bayes >= 0.)
      ZE_MessageInfo(9, "%s Bayes filter score : %6.3f", id, rScores.bayes);
    else
      ZE_MessageInfo(9, "%s Bayes filter score : Unchecked", id);
  }

  if (!get_msg_headers(fname, &spam))
  {

  }

  if (rScores.do_oracle || rScores.do_regex || rScores.do_urlbl)
  {
    /* check header contents */
    if (rScores.do_regex)
    {
      header_T           *h = spam.hdrs;

      int                 score = 0;

      for (h = spam.hdrs; h != NULL; h = h->next)
      {
        int                 where = MAIL_HEADERS;

        if (strcasecmp(h->attr, "subject") == 0)
          where |= MAIL_SUBJECT;
        if (strcasecmp(h->attr, "from") == 0)
          where |= MAIL_FROM;

        score = check_regex(id, ip, h->value, where);

        rScores.headers += score;
      }
    }

    spam.scores = rScores;
    (void) scan_body_contents(id, ip, fname, maxsize, &spam, &flags, &rScores);
    rScores = spam.scores;
  }

  {
    char                sout[256];
    double              score = 0.;
    header_T           *h;
    size_t              size = 0;

    memset(mstat->header, 0, sizeof (mstat->header));
    mstat->score = score;

    score = compute_msg_score(&rScores);
    (void) create_msg_score_header(sout, sizeof (sout), fname, NULL, &rScores);
    ZE_MessageInfo(8, "%s", sout);

    strlcpy(mstat->header, sout, sizeof(mstat->header));

    if ((h = get_msgheader(spam.hdrs, "Subject")) != NULL)
    {
      snprintf(sout, 80, "%s", h->value);
      ZE_MessageInfo(9, "MSGID : %s SUBJECT : %s", id, sout);
    }
    if ((h = get_msgheader(spam.hdrs, "From")) != NULL)
    {
      snprintf(sout, 80, "%s", h->value);
      ZE_MessageInfo(9, "MSGID : %s FROM    : %s", id, sout);
    }
    size = zeGetFileSize(fname);
    ZE_MessageInfo(9, "MSGID : %s SIZE    : %7d", id, size);

    if (mstat->spam_judgement)
    {
      char    buf[512];
      int     i;
      double lscore = 0.0;

      ZE_MessageInfo(8, "MSGID : %s CLASS   : %-5s %7.3f %s", id,
                   STRBOOL(score > mstat->spam_threshold, "SPAM", "HAM"),
                   score, fname);

      for (i = 2; i < mstat->argc; i++) {
	strlcat(buf, " ", sizeof (buf));
	strlcat(buf, mstat->argv[i], sizeof (buf));
      }
      lscore = rScores.bayes;
      ZE_MessageInfo(8, "%s %s score=%-9.6f prob=%-9.6f class=%-5s",
		   fname, buf, logit(lscore), lscore, 
		   STRBOOL(lscore > mstat->spam_threshold, "spam", "ham"));
    }
    /* 1225493245 joe-0812/spam/msg.0000003 op=class judge=spam class=spam 
       learn=false query=true  miss=false noisy=false features=911 
       score=0.303917 prob=0.575400
    */
  }

  ZE_MessageInfo(10, "");

  free_msg_headers(&spam);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  FILE               *fin;
  char               *fname;
  int                 i;
  pthread_t           tid;
  msgtbx_T            mstat;
} msg_worker_T;

static              bool
get_next_message_file(fin, buf, sz)
     FILE               *fin;
     char               *buf;
     size_t              sz;
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  bool                r = FALSE;

  if (fin == NULL)
  {
    sleep(1);
    return r;
  }

  MUTEX_LOCK(&mutex);

  if (fgets(buf, sz, fin) != NULL)
  {
    char               *p;

    if ((p = strchr(buf, '\n')) != NULL)
      *p = '\0';
    r = TRUE;
  }
  MUTEX_UNLOCK(&mutex);

  return r;
}

static void        *
worker_check_file(arg)
     void               *arg;
{
  char                fname[256];
  int                 nb = 0;
  msg_worker_T       *worker = (msg_worker_T *) arg;

  while (get_next_message_file(worker->fin, fname, sizeof (fname)))
  {
    if (zeGetFileSize(fname) > 150000)
      continue;

    nb += cli_handle_message(fname, nb, &worker->mstat);
  }

  return NULL;
}

static              bool
launch_workers(n, fname, mstatp)
     int                 n;
     char               *fname;
     msgtbx_T           *mstatp;
{
  int                 i;
  msg_worker_T        worker[256];
  FILE               *fin = NULL;

  memset(&worker, 0, sizeof (worker));

  if ((fin = fopen(fname, "r")) == NULL)
  {
    ZE_LogSysError("Error opening %s file", fname);
    return FALSE;
  }

  for (i = 0; i < n; i++)
  {
    int                 r;

    worker[i].i = i;
    worker[i].fname = fname;
    worker[i].fin = fin;
    worker[i].tid = (pthread_t) - 1;
    if (mstatp != NULL)
      worker[i].mstat = *mstatp;

    r = pthread_create(&worker[i].tid, NULL, worker_check_file, &worker[i]);
    if (r != 0)
    {
      worker[i].tid = (pthread_t) - 1;
      ZE_LogSysError("Error launching worker");
      break;
    }
  }
  for (i = 0; i < n; i++)
  {
    int                 r;

    if (worker[i].tid < 0)
      continue;

    ZE_MessageInfo(10, "Waiting thread %d", i);

    r = pthread_join(worker[i].tid, NULL);
    worker[i].tid = (pthread_t) - 1;
    if (r != 0)
    {
      ZE_LogSysError("Error launching worker");
    }
  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
cli_toolbox(mstat)
     msgtbx_T           *mstat;
{
#define NARG   32
  char                line[1024];
  int                 nb = 0;

  ze_logLevel = 8;
  ZE_MessageInfo(7, "Beginning...");
  while (fgets(line, sizeof (line), stdin) != NULL)
  {
    int                 argc;
    char               *argv[NARG];

    zeStrChomp(line);
    ZE_MessageInfo(12, "Read : %s !", line);

    argc = zeStr2Tokens(line, NARG, argv, " ");
    if (argc == 0)
      break;

    if (STRCASEEQUAL(argv[0], "QUIT"))
    {
      ZE_MessageInfo(7, "200 Exiting...");
      break;
    }

    if (STRCASEEQUAL(argv[0], "REOPEN"))
    {
      bool                ok;

      ok = bfilter_db_reopen();

      ZE_MessageInfo(7, "200 Database reopened : %s", STRBOOL(ok, "OK", "KO"));
      continue;
    }

    if (STRCASEEQUAL(argv[0], "JUDGE"))
    {
      if (argc < 2)
      {
        ZE_MessageInfo(7, "%s : Error...", argv[0]);
        continue;
      }

      mstat->spam_judgement = FALSE;
      if (STRCASEEQUAL(argv[1], "ON") || STRCASEEQUAL(argv[1], "YES"))
	mstat->spam_judgement = TRUE;

      continue;
    }

    if (STRCASEEQUAL(argv[0], "THRESHOLD"))
    {
      if (argc < 2)
      {
        ZE_MessageInfo(7, "%s : Error...", argv[0]);
        continue;
      }

      {
        double              v = 0.;

        errno = 0;
        v = strtod(argv[1], NULL);
        if (errno == 0)
          mstat->spam_threshold = v;
      }
      continue;
    }

    if (STRCASEEQUAL(argv[0], "LOGLEVEL"))
    {
      int                 l;

      if (argc < 2)
      {
        ZE_MessageInfo(7, "%s : Error...", argv[0]);
        continue;
      }

      {
        int                 l;

        l = atoi(argv[1]);
        if (l < 0 || l > 15)
        {
          ZE_MessageInfo(7, "%s %s : Error...", argv[0], argv[1]);
          continue;
        }
        ze_logLevel = l;
      }

      continue;
    }

    if (STRCASEEQUAL(argv[0], "TRAIN"))
    {
      ZE_MessageInfo(7, "%s not yet implemented...", argv[0]);
      continue;
    }

    if (STRCASEEQUAL(argv[0], "CLASSIFY"))
    {
      size_t              size;

      if (argc < 2)
      {
        ZE_MessageInfo(7, "%s : Error...", argv[0]);
        continue;
      }

      size = zeGetFileSize(argv[1]);
      if (size == 0)
      {
        ZE_MessageInfo(8, "%s File %s not found", argv[0], argv[1]);
        continue;
      }

      mstat->argc = argc;
      mstat->argv = argv;
      nb += cli_handle_message(argv[1], nb, mstat);
      continue;
    }

  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
usage()
{
  printf("Usage : ze-message-tbx options\n"
         " -h \n"
         " -c file\n"
         "    configuration file (default : %s)\n"
         " -v \n"
         " -r \n"
         " -s \n"
         " -S \n"
         " -N \n"
         " -b \n"
         " -m checks\n"
         "    where checks is a list of comma separated checks :\n"
         "       oracle, regex, urlbl, bayes or all\n"
         " -l ze_logLevel\n"
         " -t file type\n"
         "    where file type tells how messages are arranged inside args\n"
         "       mbox    - each argument is a mailbox file with many messages\n"
         "       file    - each argument is a file with one message\n"
         "       maildir - each argument is a directory with files inside\n"
         "       dir     - the same as maildir\n"
         "       list    - each argument is a file with a list of file names\n"
         "                 each file contains a single message\n", ZE_CONF_FILE);

  printf("\n%s - Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz - (C) 2001-2007\n",
         PACKAGE);
  printf("  Compiled on %s %s\n\n", __DATE__, __TIME__);
}
