
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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
#include <libze.h>
#include <libml.h>
#include "ze-filter.h"



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define LR_CLASS2LABEL(class)				\
  ((class) == LR_CLASS_HAM ? "ham" :			\
   (class) == LR_CLASS_SPAM ? "spam" : "unknown")

#define LR_LABEL2CLASS(label)				\
  (STRCASEEQUAL((label), "spam") ? LR_CLASS_SPAM :	\
   STRCASEEQUAL((label), "ham") ? LR_CLASS_HAM :	\
   LR_CLASS_UNKNOWN)

#define DHIST   100000

typedef struct {
  int                 cmd;
  time_t              date;
  long                serial;
  char                fname[128];
  int                 class;
  bool                ok;
} learn_evt_T;


static void
lr_evt_fill(evt, cmd, date, class, fname)
     learn_evt_T        *evt;
     int                 cmd;
     time_t              date;
     int                 class;
     char               *fname;
{
  if (evt != NULL) {
    memset(evt, 0, sizeof (*evt));
    evt->date = date;
    evt->class = class;
    strlcpy(evt->fname, fname, sizeof (evt->fname));
    evt->ok = TRUE;
    evt->cmd = cmd;
  }
}

#define LR_EVT_FILL(evt, cmd, date, class, fname)		\
  do {								\
    if ((evt) != NULL) {					\
      memset((evt), 0, sizeof(*(evt)));				\
      (evt)->date = (date);					\
      (evt)->class = (class);					\
      strlcpy((evt)->fname, (fname), sizeof((evt)->fname));	\
      (evt)->ok = TRUE;						\
      (evt)->cmd = (cmd);					\
    }								\
  } while (0)


#define DPILE     20000

typedef struct {
  int                 n;
  learn_evt_T         p[DPILE];
  long                serial;
} pile_T;

#define PILE_INIT(p)				\
  do {						\
    if (p != NULL) 				\
      memset(p, 0, sizeof(p));			\
  } while(0)


static bool         pile_push(pile_T * pile, learn_evt_T * evt);
static bool         pile_pop(pile_T * pile, learn_evt_T * evt);
static bool         pile_shift(pile_T * pile, learn_evt_T * evt);

static bool         pile_check_top(pile_T * pile, learn_evt_T * evt);
static bool         pile_check_bottom(pile_T * pile, learn_evt_T * evt);

static bool         pile_sort(pile_T * pile);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */


#define CLI_NONE       0
#define CLI_CLASSIFY   1
#define CLI_LEARN      2
#define CLI_ONLINE     3

#define CLI_DELAY_FN_CTE    0
#define CLI_DELAY_FN_EXP    1
#define CLI_DELAY_FN_FIX    2

typedef struct {
  /*
   * resampling to equilibrate classes 
   */
  bool                resample;

  /*
   */
  time_t              delay;
  int                 delay_fn;

  /*
   * (M/m) probability of active learning feedback miss 
   */
  bool                miss_enable;
  double              pmiss;

  /*
   * (N) feedback error rate 
   */
  bool                noise_enable;
  double              pnoise;

  /*
   * (e) ask for error feedback 
   */
  bool                error_feedback;

  time_t              tstart;
  time_t              tend;
} cli_opts_T;

#define CLI_OPT_INIT {FALSE, 7200, CLI_DELAY_FN_CTE, FALSE, -1., FALSE, -1., FALSE, 0, 0}

void                usage(char *);

static bool         decode_lr_options(lr_opts_T * lrOpt, cli_opts_T * cliOpt,
                                      char *optarg);

static int          cli_lr_classify(char *fileIn, char *dataFile);

static int          cli_lr_learn(char *fileIn, char *dataFile, cli_opts_T * opt,
                                 lr_opts_T * lrOpts);

static int          cli_lr_simul(char *fileIn, char *dataFile, cli_opts_T * opt,
                                 lr_opts_T * lrOpts);

static int          cli_lr_extract(char *fileIn, char *dataFile,
                                   cli_opts_T * opt, lr_opts_T * lrOpts);

static double       learn_callback(int i, lr_cargs_T * carg, lr_margs_T * marg);
static double       lrate = 0.004;

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  char               *fileIn = NULL;
  char               *fileData = "/tmp/lr.txt";

  int                 mode = CLI_NONE;

  cli_opts_T          cliOpt = CLI_OPT_INIT;

  lr_opts_T           lrOpts = LR_OPTS_INITIALIZER;

  char               *xmode = "simul";

  {
    const char         *args = "hi:d:lR:L:x:o:";
    int                 c;
    int                 io;

    while ((c = getopt(argc, argv, args)) != -1) {
      switch (c) {
        case 'h':
          usage(argv[0]);
          exit(0);
          break;

        case 'i':
          fileIn = optarg;
          break;
        case 'd':
          fileData = optarg;
          break;


        case 'R':
        case 'L':
          lrate = atof(optarg);
          break;
        case 'x':
          xmode = optarg;
          break;

        case 'o':
          if (!decode_lr_options(&lrOpts, &cliOpt, optarg));
          break;

        default:
          break;
      }
    }

    io = optind;

    while (io < argc && *argv[io] == '-')
      io++;

    if (io < argc) {
      /*
       * fname = argv[io++]; 
       */
    }
  }

  zeLog_SetOutput(FALSE, TRUE);
  ze_logLevel = 10;

  if (0)
    configure("ze-lr", conf_file, FALSE);
  set_mime_debug(FALSE);

  if (xmode != NULL) {
    char               *tag = NULL;

    tag = "learn";
    if (STRNCASEEQUAL(xmode, tag, strlen(tag))) {
      (void) cli_lr_learn(fileIn, fileData, &cliOpt, &lrOpts);
      goto fin;
    }

    tag = "class";
    if (STRNCASEEQUAL(xmode, tag, strlen(tag))) {
      (void) cli_lr_classify(fileIn, fileData);
      goto fin;
    }

    tag = "simul";
    if (STRNCASEEQUAL(xmode, tag, strlen(tag))) {
      (void) cli_lr_simul(fileIn, fileData, &cliOpt, &lrOpts);
      goto fin;
    }

    tag = "extract";
    if (STRNCASEEQUAL(xmode, tag, strlen(tag))) {
      (void) cli_lr_extract(fileIn, fileData, &cliOpt, &lrOpts);
      goto fin;
    }
  }


fin:
  exit(0);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static int
cli_lr_extract(fileIn, dataFile, cliopt, lropts)
     char               *fileIn;
     char               *dataFile;
     cli_opts_T         *cliopt;
     lr_opts_T          *lropts;
{
  char               *fname = NULL;
  FILE               *fdata = NULL, *fin = NULL;

  fdata = stdout;
  fin = stdin;

  /*
   ** Handle messages
   */
  if (fileIn != NULL) {
    fin = fopen(fileIn, "r");
    if (fin == NULL) {
      ZE_LogSysError("Error opening %s", fileIn);;
      goto fin;
    }
  }

  lr_data_open(dataFile);

  if (fin != NULL) {
    char                stime[32];
    char                sclass[32];
    char                sfile[512];

    int                 nham = 0, nspam = 0;
    bool                spam = FALSE;

    lr_cargs_T          cargs;
    lr_margs_T          margs;

    int                 nid = 0;

    memset(&cargs, 0, sizeof (cargs));
    memset(&margs, 0, sizeof (margs));

    memset(stime, 0, sizeof (stime));
    memset(sclass, 0, sizeof (sclass));
    memset(sfile, 0, sizeof (sfile));

    while (fin != NULL && fscanf(fin, "%s %s %s", stime, sclass, sfile) == 3) {
      char                id[256];

      spam = STRCASEEQUAL(sclass, "spam");
      fname = sfile;

      snprintf(id, sizeof (id), "%08d", nid++);
      ZE_MessageInfo(13, "# %-4s : %s", sclass, sfile);

      margs.cmd = LR_CMD_EXTRACT;
      margs.class = LR_CLASS_UNKNOWN;
      lr_extract(id, fname, &cargs, &margs);
    }
  }

  lr_data_close();

fin:
  return 0;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static int
cli_lr_classify(fileIn, dataFile)
     char               *fileIn;
     char               *dataFile;
{
  char               *id = "000000.000";
  char               *fname = NULL;
  FILE               *fdata = NULL, *fin = NULL;

  fdata = stdout;
  fin = stdin;

  /*
   ** Handle messages
   */
  if (fileIn != NULL) {
    fin = fopen(fileIn, "r");
    if (fin == NULL) {
      ZE_LogSysError("Error opening %s", fileIn);;
      goto fin;
    }
  }

  lr_data_open(dataFile);

  if (fin != NULL) {
    char                stime[32];
    char                sclass[32];
    char                sfile[512];

    int                 nham = 0, nspam = 0;
    bool                spam = FALSE;

    test_score_T        mscore;
    lr_cargs_T          cargs;
    lr_margs_T          margs;

    memset(&cargs, 0, sizeof (cargs));
    memset(&margs, 0, sizeof (margs));
    memset(&mscore, 0, sizeof (mscore));

    memset(stime, 0, sizeof (stime));
    memset(sclass, 0, sizeof (sclass));
    memset(sfile, 0, sizeof (sfile));

    while (fin != NULL && fscanf(fin, "%s %s %s", stime, sclass, sfile) == 3) {
      spam = STRCASEEQUAL(sclass, "spam");
      fname = sfile;
      ZE_MessageInfo(13, "%-4s : %s", sclass, sfile);

      margs.cmd = LR_CMD_CLASS;
      margs.class = LR_CLASS_UNKNOWN;
      lr_classify(id, fname, &cargs, &margs, &mscore);

      ZE_MessageInfo(10,
                     "%s %s op=%s truth=%-4s class=%-4s score=%.4f prob=%.4f",
                     stime, fname, "class", spam ? "spam" : "ham",
                     mscore.odds > 0.0 ? "spam" : "ham", mscore.odds,
                     mscore.value);
    }
  }

  lr_data_close();

fin:
  return 0;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int
cli_lr_learn(fileIn, dataFile, cliopt, lropts)
     char               *fileIn;
     char               *dataFile;
     cli_opts_T         *cliopt;
     lr_opts_T          *lropts;
{
  char               *id = "000000.000";
  char               *fname = NULL;
  FILE               *fdata = NULL, *fin = NULL;

  bool                resample = cliopt->resample;

  fdata = stdout;
  fin = stdin;

  lr_learn_options(lropts->active_learning, lropts->active_margin);

  /*
   ** Handle messages
   */
  if (fileIn != NULL) {
    fin = fopen(fileIn, "r");
    if (fin == NULL) {
      ZE_LogSysError("Error opening %s", fileIn);
      goto fin;
    }
  }

  lr_data_open(dataFile);

  if (!lr_set_learn_callback(learn_callback)) {

  }

  if (fin != NULL) {
    char                stime[32];
    char                sclass[32];
    char                sfile[512];

    static learn_evt_T  eSpam[DHIST];
    static learn_evt_T  eHam[DHIST];

    int                 nham = 0, nspam = 0;
    int                 pham = 0, pspam = 0;
    int                 nbMax = DHIST;

    bool                spam = FALSE;
    test_score_T        mscore;
    lr_cargs_T          cargs;
    lr_margs_T          margs;
    time_t              t_last, t_now;

    long                nbl = 0;

    nbMax = 32768;
    {
      char               *env = NULL;

      if ((env = getenv("LRRESAMPLEWIN")) != NULL) {
        int                 n;

        n = atof(env);
        if (n >= 0)
          nbMax = n;
      }
    }

    memset(&mscore, 0, sizeof (mscore));
    memset(&cargs, 0, sizeof (cargs));
    memset(&margs, 0, sizeof (margs));

    memset(stime, 0, sizeof (stime));
    memset(sclass, 0, sizeof (sclass));
    memset(sfile, 0, sizeof (sfile));

    while (TRUE) {
      int                 i;
      bool                ok;
      time_t              date;

      learn_evt_T         evt;

      if (fscanf(fin, "%s %s %s", stime, sclass, sfile) != 3)
        break;

      date = zeStr2time(stime, NULL, (time_t) 0);
      spam = spam = STRCASEEQUAL(sclass, "spam");
      fname = sfile;

      ZE_MessageInfo(10, "%-4s : %s", sclass, sfile);

      margs.query = FALSE;
      margs.learnt = FALSE;
      margs.resample = FALSE;

      cargs.nmsg++;

      cargs.nbml++;
      ok = lr_learn(id, fname, &cargs, &margs, &mscore, spam);
      ok = TRUE;

      ZE_MessageInfo(10,
                     "%s classification : %8.4f %.8f truth=%-4s class=%-4s"
                     " learn=%s query=%s features=%d"
                     " score=%g prob=%.6f",
                     sfile,
                     mscore.odds, mscore.value, STRBOOL(spam, "spam", "ham"),
                     STRBOOL(mscore.odds > 0.0, "spam", "ham"),
                     STRBOOL(margs.learnt, "true", "false"),
                     STRBOOL(margs.query, "true", "false"),
                     cargs.nFeatures, mscore.odds, mscore.value);

      if (!ok)
        continue;

      memset(&evt, 0, sizeof (evt));
      strlcpy(evt.fname, fname, sizeof (evt.fname));
      evt.class = spam;
      evt.date = date;
      evt.ok = TRUE;

      pspam = nspam % nbMax;
      pham = nham % nbMax;
      if (spam) {
        eSpam[pspam] = evt;
        nspam++;
      } else {
        eHam[pham] = evt;
        nham++;
      }

      if (!resample)
        continue;

      if (nspam == 0 || nham == 0)
        continue;

      spam = !spam;
      if (spam) {
        if (nspam > nbMax)
          i = random() % nbMax;
        else
          i = random() % nspam;
        fname = eSpam[i].fname;
      } else {
        if (nham > nbMax)
          i = random() % nbMax;
        else
          i = random() % nham;
        fname = eHam[i].fname;
      }

      margs.resample = TRUE;
      lr_learn(id, fname, &cargs, &margs, &mscore, spam);
    }

    fclose(fin);

    /*
     ** save learned data...
     */
    lr_data_dump(dataFile);
  }

  /*
   ** close...
   */
  lr_data_close();

fin:
  return 0;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static              bool
cli_simul_handle(cargs, margs, pile, evt)
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
     pile_T             *pile;
     learn_evt_T        *evt;
{
  test_score_T        mscore;
  char               *fname = NULL;
  bool                ok;

  char               *id = "000000.000";

  memset(&mscore, 0, sizeof (mscore));

  memset(margs, 0, sizeof (*margs));
  /*
   ** margs->miss = FALSE;
   ** margs->query = FALSE;
   ** margs->learnt = FALSE;
   ** margs->resample = FALSE;
   */
  margs->cmd = evt->cmd;
  margs->class = evt->class;
  fname = evt->fname;

  ZE_MessageInfo(17, "* Handle %3d %s", evt->cmd, fname);
  switch (evt->cmd) {
    case LR_CMD_CLASS:
      ok = lr_classify(id, fname, cargs, margs, &mscore);
      cargs->nbmc++;
      break;
    case LR_CMD_LEARN:
      ok =
        lr_learn(id, fname, cargs, margs, &mscore, evt->class == LR_CLASS_SPAM);
      cargs->nbml++;
      break;
    case LR_CMD_LEARN_RESAMPLE:
      ok =
        lr_learn(id, fname, cargs, margs, &mscore, evt->class == LR_CLASS_SPAM);
      break;
    case LR_CMD_LEARN_FEEDBACK:
      ok =
        lr_learn(id, fname, cargs, margs, &mscore, evt->class == LR_CLASS_SPAM);
      cargs->nbml++;
      break;
  }
  margs->score = mscore;

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

typedef struct {
  bool                query;
  bool                error;
  float               loss;
} stcl_T;

#define    ST_DIM     0x10000

static int
cli_lr_simul(fileIn, dataFile, cliopt, lropts)
     char               *fileIn;
     char               *dataFile;
     cli_opts_T         *cliopt;
     lr_opts_T          *lropts;
{
  char               *id = "000000.000";
  char               *fname = NULL;
  FILE               *fdata = NULL, *fin = NULL;

  bool                resample = cliopt->resample;

  bool                miss_enable = cliopt->miss_enable;
  double              pmiss = cliopt->pmiss;

  double              risk = 0.;
  double              qrate = 0.;
  double              erate = 0.;
  int                 stn = 0;
  int                 st_dim = ST_DIM;

  stcl_T              stcl[ST_DIM];

  st_dim = 1000;
  memset(stcl, 0, sizeof (stcl));
  {
    char               *env;

    if ((env = getenv("STDIM")) != NULL) {
      st_dim = zeStr2long(env, NULL, st_dim);
      st_dim = MIN(st_dim, ST_DIM);
    }
  }

  fdata = stdout;
  fin = stdin;

  lr_learn_options(lropts->active_learning, lropts->active_margin);

  /*
   ** Handle messages
   */
  if (fileIn != NULL) {
    fin = fopen(fileIn, "r");
    if (fin == NULL) {
      ZE_LogSysError("Error opening %s", fileIn);
      goto fin;
    }
  }

  lr_data_open(dataFile);

  if (!lr_set_learn_callback(learn_callback)) {

  }

  if (fin != NULL) {
    char                stime[32];
    char                sclass[32];
    char                sfile[512];

    static learn_evt_T  eSpam[DHIST];
    static learn_evt_T  eHam[DHIST];

    int                 nham = 0, nspam = 0;
    int                 pham = 0, pspam = 0;
    int                 nbMax = DHIST;

    lr_cargs_T          cargs;
    lr_margs_T          margs;

    long                nbl = 0;

    pile_T              pile;
    learn_evt_T         pevt, fevt;

    PILE_INIT(&pile);

    nbMax = 32768;
    {
      char               *env = NULL;

      if ((env = getenv("LRRESAMPLEWIN")) != NULL) {
        int                 n;

        n = atof(env);
        if (n >= 0)
          nbMax = n;
      }
    }

    memset(&cargs, 0, sizeof (cargs));
    memset(&margs, 0, sizeof (margs));

    memset(stime, 0, sizeof (stime));
    memset(sclass, 0, sizeof (sclass));
    memset(sfile, 0, sizeof (sfile));

    while (TRUE) {
      int                 i;
      bool                ok;
      time_t              date;

      memset(&fevt, 0, sizeof (fevt));
      if (fscanf(fin, "%s %s %s", stime, sclass, sfile) == 3) {
        int                 class;
        bool                spam = FALSE;

        cargs.nmsg++;

        date = zeStr2time(stime, NULL, (time_t) 0);
        class = LR_LABEL2CLASS(sclass);
        spam = (class == LR_CLASS_SPAM);
        fname = sfile;

        if (cliopt->tend > 0 && date > cliopt->tend)
          break;

#if 0
        if (cliopt->tstart > 0 && date < cliopt->tstart)
          continue;
#endif

        ZE_MessageInfo(17, "* Read task %s", sfile);
        (void) lr_evt_fill(&fevt, LR_CMD_CLASS, date, class, fname);

        if (spam) {
          pspam = nspam % nbMax;
          eSpam[pspam] = fevt;
          nspam++;
        } else {
          pham = nham % nbMax;
          eHam[pham] = fevt;
          nham++;
        }
      } else
        break;

      /*
       ** Handle feedback events
       */
      while (pile_check_bottom(&pile, &pevt)) {
        bool                spam = FALSE;

        if (fevt.date < pevt.date)
          break;

        (void) pile_shift(&pile, &pevt);

        spam = (pevt.class == LR_CLASS_SPAM);

        ZE_MessageInfo(10, "* %ld : Handle pile event %s", pevt.date,
                       pevt.fname);
        /*
         ** Should I miss it ???
         */

        /*
         ** handle pevt
         */
        cli_simul_handle(&cargs, &margs, &pile, &pevt);

        ZE_MessageInfo(10,
                       "%ld %s op=%s truth=%-4s class=%-4s"
                       " learn=%-5s query=%-5s miss=%-5s noisy=%-5s features=%d"
                       " score=%g prob=%.6f",
                       pevt.date,
                       pevt.fname,
                       "feedback",
                       STRBOOL(spam, "spam", "ham"),
                       STRBOOL(margs.score.odds > 0.0, "spam", "ham"),
                       STRBOOL(margs.learnt, "true", "false"),
                       STRBOOL(margs.query, "true", "false"),
                       STRBOOL(FALSE, "true", "false"),
                       STRBOOL(FALSE, "true", "false"),
                       cargs.nFeatures, margs.score.odds, margs.score.value);
      }

      /*
       ** Handle file events
       */
      if (fevt.ok) {
        bool                spam = FALSE;
        bool                query = FALSE;
        bool                noisy = FALSE;
        bool                miss = FALSE;

        int                 stp;

        stp = stn % st_dim;
        /*
         * memset(&stcl[stp], 0, sizeof(stcl_T)); 
         */
        stcl[stp].error = FALSE;
        stcl[stp].query = FALSE;
        stcl[stp].loss = 0.;

        ZE_MessageInfo(10, "* %ld : Handle file event %s", fevt.date,
                       fevt.fname);
        cli_simul_handle(&cargs, &margs, &pile, &fevt);

        spam = (fevt.class == LR_CLASS_SPAM);

        /*
         * decide whether add it to the pile 
         */
        {
          double              loss;

          loss = fabs((spam ? 1. : 0.) - margs.score.value);

          stcl[stp].loss = loss;

          /*
           * query golden class ? add it to the pile 
           */
          if (spam != (margs.score.odds > 0.0)) {
            query = TRUE;
            stcl[stp].error = TRUE;
          }

          /*
           * query as it's inside margin 
           */
          if (fabs(margs.score.value - 0.5) < lropts->active_margin) {
            query = TRUE;
            stcl[stp].query = TRUE;
          }

          if (query && cliopt->miss_enable && cliopt->pmiss > 0.0) {
            double              r;

            margs.miss = FALSE;

            r = drand48();
            if ((r = drand48()) < cliopt->pmiss) {
              miss = margs.miss = TRUE;
              query = FALSE;
            }
          }

          /*
           * update error and query rates 
           */
          {
            int                 i, nq, ne;
            int                 stmax;

            stn++;
            stmax = MIN(stn, st_dim);

            for (i = nq = ne = 0, risk = 0.; i < stmax; i++) {
              if (stcl[i].query)
                nq++;
              if (stcl[i].error)
                ne++;
              risk += stcl[i].loss;
            }
            erate = ((double) ne) / stmax;
            qrate = ((double) nq) / stmax;
            risk = risk / stmax;
            ZE_MessageInfo(10,
                           "STATS: %7d QRATE %8.5f ERATE: %8.5f RISK: %8.5f",
                           stn, qrate, erate, risk);
          }

          if (query) {
            learn_evt_T         evt;

            ZE_MessageInfo(17, "* Add to pile %s spam : %d %8.3f ",
                           fevt.fname, spam, margs.score.odds);
            evt = fevt;
            evt.cmd = LR_CMD_LEARN_FEEDBACK;

            if (cliopt->noise_enable && cliopt->pnoise > 0) {
              double              r;

              if ((r = drand48()) < cliopt->pnoise) {
                evt.class = (1 - evt.class);
                noisy = TRUE;
              }
            }

            /*
             * add one hour delay 
             */
            if (cargs.nmsg >= 2000) {
              switch (cliopt->delay_fn) {
                case CLI_DELAY_FN_CTE:
                  evt.date += cliopt->delay;
                  break;
                case CLI_DELAY_FN_EXP:
                  {
                    double              x;

                    x = -cliopt->delay * log(1 - drand48() + 1.e-10);
                    x = MAX(x, 600);
                    x = MIN(x, (30 * 86400));
                    evt.date += x;
                  }
                  break;
                case CLI_DELAY_FN_FIX:
                  evt.date += 86400 - (evt.date % 86400);
                  break;
                default:
                  evt.date += cliopt->delay;
                  break;
              }
            }
            (void) pile_push(&pile, &evt);
          }
        }

        /*
         * print results 
         */

        ZE_MessageInfo(10,
                       "%ld %s op=%s truth=%-4s class=%-4s"
                       " learn=%-5s query=%-5s miss=%-5s noisy=%-5s features=%d"
                       " score=%g prob=%.6f",
                       fevt.date,
                       fevt.fname,
                       "class",
                       STRBOOL(spam, "spam", "ham"),
                       STRBOOL(margs.score.odds > 0.0, "spam", "ham"),
                       STRBOOL(margs.learnt, "true", "false"),
                       STRBOOL(query, "true", "false"),
                       STRBOOL(miss, "true", "false"),
                       STRBOOL(noisy, "true", "false"),
                       cargs.nFeatures, margs.score.odds, margs.score.value);

        /*
         * resample ? randomly choose old message and learn it 
         */
#if 0
        if (resample && nspam > 1 && nham > 1) {
          int                 m;
          int                 class;

          spam = !spam;
          if (spam) {
            m = (nspam > nbMax) ? nbMax : nspam;
            i = random() % m;
            fname = eSpam[i].fname;
          } else {
            m = (nham > nbMax) ? nbMax : nham;
            i = random() % m;
            fname = eHam[i].fname;
          }
          class = (spam ? LR_CLASS_SPAM : LR_CLASS_HAM);
          (void) lr_evt_fill(&fevt, LR_CMD_LEARN_RESAMPLE, date, class, fname);
          cli_simul_handle(&cargs, &margs, &pile, &fevt);
        }
#endif
      }

    }
    fclose(fin);
    /*
     ** save learned data...
     */
    lr_data_dump(dataFile);
  }

  /*
   ** close...
   */
  lr_data_close();

fin:
  return 0;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static double
learn_callback(i, cargs, margs)
     int                 i;
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
{
  double              r = lrate;

  static bool         ok = FALSE;
  static int          srate = 2;
  static double       ri = 0.5, rf = 0.004, teta = -0.040;

  if (!ok) {
    char               *s = NULL;

    if ((s = getenv("LRATEDEFS")) != NULL) {
      char                ebuf[256];
      char               *argv[8];
      int                 argc;

      memset(argv, 0, sizeof (argv));
      strlcpy(ebuf, s, sizeof (ebuf));
      argc = zeStr2Tokens(ebuf, 8, argv, ",; ");
      if (argv[0] != NULL)
        srate = atof(argv[0]);
      if (argv[1] != NULL)
        ri = atoi(argv[1]);
      if (argv[2] != NULL) {
        rf = atof(argv[2]);
        lrate = rf;
      }
      if (argv[3] != NULL)
        teta = atof(argv[3]);
    }
    ok = TRUE;
  }

  switch (srate) {
    case 0:
      r = lrate;
      break;
    case 1:
      r = ri / sqrt(i + 1);
      if (r < rf)
        r = rf;
      break;
    case 2:
      r = (ri - rf) * exp(teta * i) + rf;
      break;
  }

  ZE_MessageInfo(10, "* learning rate  : %7d %8.5f", i, r);

  return r;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define DPILE     20000


static              bool
pile_push(pile, evt)
     pile_T             *pile;
     learn_evt_T        *evt;
{
  if (pile == NULL)
    return FALSE;

  if (pile->n >= DPILE - 1)
    return FALSE;

  pile->serial++;
  evt->serial = pile->serial;
  pile->p[pile->n] = *evt;
  pile->n++;

  return TRUE;
}

static              bool
pile_pop(pile, evt)
     pile_T             *pile;
     learn_evt_T        *evt;
{
  if (pile == NULL)
    return FALSE;

  if (pile->n <= 0)
    return FALSE;

  *evt = pile->p[pile->n - 1];
  pile->n--;

  return TRUE;
}

static              bool
pile_shift(pile, evt)
     pile_T             *pile;
     learn_evt_T        *evt;
{
  if (pile == NULL)
    return FALSE;

  if (pile->n <= 0)
    return FALSE;

  *evt = pile->p[0];
  {
    int                 i;

    for (i = 0; i < pile->n - 1; i++)
      pile->p[i] = pile->p[i + 1];
  }
  pile->n--;

  return TRUE;
}

static              bool
pile_check_top(pile, evt)
     pile_T             *pile;
     learn_evt_T        *evt;
{
  if (pile == NULL)
    return FALSE;

  if (pile->n <= 0)
    return FALSE;

  *evt = pile->p[pile->n - 1];

  return TRUE;
}

static              bool
pile_check_bottom(pile, evt)
     pile_T             *pile;
     learn_evt_T        *evt;
{
  if (pile == NULL)
    return FALSE;

  if (pile->n <= 0)
    return FALSE;

  *evt = pile->p[0];

  return TRUE;
}

static int
lrevtcmp(const void *ea, const void *eb)
{
  learn_evt_T        *pea = (learn_evt_T *) ea;
  learn_evt_T        *peb = (learn_evt_T *) eb;

  if (pea->date > peb->date)
    return 1;
  if (pea->date < peb->date)
    return -1;

  return (pea->serial - peb->serial);
}

static              bool
pile_sort(pile)
     pile_T             *pile;
{
  if (pile == NULL)
    return FALSE;

  if (pile->n > 1)
    qsort(pile->p, pile->n, sizeof (learn_evt_T), lrevtcmp);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define OPTION_BOOL(args, v)					\
  {								\
    if (zeStrRegex((args), "^yes|true|ok$", NULL, NULL, TRUE))	\
      (v) = TRUE;						\
    if (zeStrRegex((args), "^no|false|ko$", NULL, NULL, TRUE))	\
      (v) = FALSE;						\
  }

#define OPTION_INT(args, v)					\
  {								\
    (v) = zeStr2long(args, NULL, (v));				\
  }

#define OPTION_DOUBLE(args, v)					\
  {								\
    (v) = zeStr2double(args, NULL, (v));				\
  }

static              bool
decode_lr_options(lrOpts, cliOpt, optarg)
     lr_opts_T          *lrOpts;
     cli_opts_T         *cliOpt;
     char               *optarg;
{
  char                buf[1024];
  char               *argv[4];
  int                 argc;

  if (optarg == NULL)
    return FALSE;

  strlcpy(buf, optarg, sizeof buf);
  argc = zeStr2Tokens(buf, 4, argv, "=");
  if (argc < 2)
    return FALSE;

  if (STRCASEEQUAL(argv[0], "LR_LRATE")) {
    OPTION_DOUBLE(argv[1], lrOpts->lrate);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_RESAMPLE")) {
    OPTION_BOOL(argv[1], lrOpts->resample);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_USE_RAW_MSG")) {
    OPTION_BOOL(argv[1], lrOpts->useRawMsg);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_RAW_LENGTH")) {
    OPTION_INT(argv[1], lrOpts->rawLength);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_BODY_LENGTH")) {
    OPTION_INT(argv[1], lrOpts->bodyLength);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_USE_BODY")) {
    OPTION_BOOL(argv[1], lrOpts->useBody);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_USE_HEADERS")) {
    OPTION_BOOL(argv[1], lrOpts->useHeaders);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_CLEAN_UP_HEADERS")) {
    OPTION_BOOL(argv[1], lrOpts->cleanUpHeaders);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_CLEANUP_DATES")) {
    OPTION_BOOL(argv[1], lrOpts->cleanUpDates);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_ACTIVE_LEARNING")) {
    OPTION_BOOL(argv[1], lrOpts->active_learning);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "LR_ACTIVE_MARGIN")) {
    OPTION_DOUBLE(argv[1], lrOpts->active_margin);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_DELAY")) {
    OPTION_INT(argv[1], cliOpt->delay);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_DELAY_FUNCTION")) {
    cliOpt->delay_fn = CLI_DELAY_FN_CTE;

    if (STRCASEEQUAL(argv[1], "cte")) {
      cliOpt->delay_fn = CLI_DELAY_FN_CTE;
      return TRUE;
    }
    if (STRCASEEQUAL(argv[1], "exp")) {
      cliOpt->delay_fn = CLI_DELAY_FN_EXP;
      return TRUE;
    }
    if (STRCASEEQUAL(argv[1], "fix")) {
      cliOpt->delay_fn = CLI_DELAY_FN_FIX;
      return TRUE;
    }
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_NOISE")) {
    OPTION_BOOL(argv[1], cliOpt->noise_enable);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_PROB_NOISE")) {
    OPTION_DOUBLE(argv[1], cliOpt->pnoise);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_MISS")) {
    OPTION_BOOL(argv[1], cliOpt->miss_enable);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_PROB_MISS")) {
    OPTION_DOUBLE(argv[1], cliOpt->pmiss);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_TSTART")) {
    OPTION_INT(argv[1], cliOpt->tstart);
    return TRUE;
  }

  if (STRCASEEQUAL(argv[0], "OPT_TEND")) {
    OPTION_INT(argv[1], cliOpt->tend);
    return TRUE;
  }

  return FALSE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
usage(arg)
     char               *arg;
{
  printf("Usage : %s options\n"
         "        -h  : this message\n"
         "        -l  : toggle learn option\n"
         "        -i  : input file with commands\n"
         "        -d  : data file\n"
         "        -a  : active learning\n"
         "        -t  : active learning threshold (0.5 - margin)\n"
         "        -r  : resample\n"
         "        -m  : feedback miss probability ([0.0,1.0])\n"
         "        -R  : asymptotic learning rate\n"
         "        -o  : option=value \n"
         "        -x mode : where mode in learn, class, simulate\n", arg);
  printf("\n     %s\n     %s\n\n", PACKAGE, COPYRIGHT);
}
