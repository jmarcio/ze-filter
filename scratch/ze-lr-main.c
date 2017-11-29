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
#include <zeLibs.h>
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

typedef struct
{
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
  if (evt != NULL)
  {
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

typedef struct
{
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

typedef struct
{
  /* resampling to equilibrate classes */
  bool                resample;
  /* (a) active learning */
  bool                al_enable;
  /* (A) active learning margin */
  double              al_margin;
  /* (t) active learning threshold */
  double              al_threshold;
  /* (D) active learning parameters */
  char               *al_defs;
  /* (M/m) probability of active learning feedback miss */
  double              pmiss;
  /* (N) feedback error rate */
  double              noise;
  /* (e) ask for error feedback */
  bool                error_feedback;
} cli_opt_T;

#define CLI_OPT_INIT {FALSE, FALSE, 0.2, 0.05, NULL, -1., -1., FALSE}

void                usage(char *);

static bool         decode_lr_options(lr_opts_T * lrOpts, char *optarg);

static int          cli_lr_learn(char *fileIn, char *dataFile, cli_opt_T * opt);
static int          cli_lr_classify(char *fileIn, char *dataFile);

static int          cli_lr_simul(char *fileIn, char *dataFile, cli_opt_T * opt);

static double       learn_callback(int i, lr_cargs_T * carg, lr_margs_T * marg);
static double       lrate = 0.004;

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  char               *fileIn = NULL;
  char               *fileData = "/tmp/lr.txt";

  bool                learn = FALSE;
  int                 mode = CLI_NONE;

  cli_opt_T           cliopt = CLI_OPT_INIT;

  lr_opts_T           lrOpts = LR_OPTS_INITIALIZER;

  char               *xmode = NULL;

  {
    const char         *args = "hi:d:lat:rR:m:x:o:";
    int                 c;
    int                 io;

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
        case 'h':
          usage(argv[0]);
          exit(0);
          break;

        case 'l':
          learn = !learn;
          break;
        case 'i':
          fileIn = optarg;
          break;
        case 'd':
          fileData = optarg;
          break;

        case 'a':
          cliopt.al_enable = TRUE;
          break;
        case 't':
          cliopt.al_threshold = atof(optarg);
          break;
        case 'r':
          cliopt.resample = TRUE;
          break;
        case 'm':
        case 'M':
          cliopt.pmiss = atof(optarg);
          break;
        case 'R':
        case 'L':
          lrate = atof(optarg);
          break;
        case 'x':
          xmode = optarg;
          break;

        case 'o':
          if (!decode_lr_options(&lrOpts, optarg))
            ;
          break;

        default:
          break;
      }
    }

    io = optind;

    while (io < argc && *argv[io] == '-')
      io++;

    if (io < argc)
    {
      /* fname = argv[io++]; */
    }
  }

  zeLog_SetOutput(FALSE, TRUE);
  ze_logLevel = 10;

  if (0)
    configure("ze-lr", conf_file, FALSE);
  set_mime_debug(FALSE);

  if (xmode != NULL)
  {
    char               *tag = NULL;

    tag = "learn";
    if (STRNCASEEQUAL(xmode, tag, strlen(tag)))
    {
      return cli_lr_learn(fileIn, fileData, &cliopt);
      goto fin;
    }

    tag = "class";
    if (STRNCASEEQUAL(xmode, tag, strlen(tag)))
    {
      return cli_lr_classify(fileIn, fileData);
      goto fin;
    }

  }

  if (learn)
  {
    return cli_lr_learn(fileIn, fileData, &cliopt);
  } else
    return cli_lr_classify(fileIn, fileData);

fin:
  exit(0);
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
  if (fileIn != NULL)
  {
    fin = fopen(fileIn, "r");
    if (fin == NULL)
    {
      ZE_LogSysError("Error opening %s", fileIn);;
      goto fin;
    }
  }

  lr_data_open(dataFile);

  if (fin != NULL)
  {
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

    while (fin != NULL && fscanf(fin, "%s %s %s", stime, sclass, sfile) == 3)
    {
      spam = STRCASEEQUAL(sclass, "spam");
      fname = sfile;
      ZE_MessageInfo(13, "%-4s : %s", sclass, sfile);

      margs.cmd = LR_CMD_CLASS;
      margs.class = LR_CLASS_UNKNOWN;
      lr_classify(id, fname, &cargs, &margs, &mscore);

      ZE_MessageInfo(10, "%s judge=%-4s class=%-4s score=%.4f prob=%.4f",
                   fname,
                   spam ? "spam" : "ham",
                   mscore.odds > 0.0 ? "spam" : "ham",
                   mscore.odds, mscore.value);
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
cli_lr_learn(fileIn, dataFile, cliopt)
     char               *fileIn;
     char               *dataFile;
     cli_opt_T          *cliopt;
{
  char               *id = "000000.000";
  char               *fname = NULL;
  FILE               *fdata = NULL, *fin = NULL;

  bool                resample = cliopt->resample;
  double              pmiss = cliopt->pmiss;

  double              marge;

  fdata = stdout;
  fin = stdin;

  marge = 0.5 - cliopt->al_threshold;
  if (marge <= 0)
    marge = 0.0001;
  lr_learn_options(cliopt->al_enable, marge);

  /*
   ** Handle messages
   */
  if (fileIn != NULL)
  {
    fin = fopen(fileIn, "r");
    if (fin == NULL)
    {
      ZE_LogSysError("Error opening %s", fileIn);
      goto fin;
    }
  }

  lr_data_open(dataFile);

  if (!lr_set_learn_callback(learn_callback))
  {

  }

  if (fin != NULL)
  {
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

      if ((env = getenv("LRRESAMPLEWIN")) != NULL)
      {
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

    while (TRUE)
    {
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
                   "%s classification : %8.4f %.8f judge=%-4s class=%-4s"
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
      if (spam)
      {
        eSpam[pspam] = evt;
        nspam++;
      } else
      {
        eHam[pham] = evt;
        nham++;
      }

      if (!resample)
        continue;

      if (nspam == 0 || nham == 0)
        continue;

      spam = !spam;
      if (spam)
      {
        if (nspam > nbMax)
          i = random() % nbMax;
        else
          i = random() % nspam;
        fname = eSpam[i].fname;
      } else
      {
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

static double
learn_callback(i, cargs, margs)
     int                 i;
     lr_cargs_T         *cargs;
     lr_margs_T         *margs;
{
  double              r = lrate;

  static bool         ok = FALSE;
  static int          srate = 2;
  static double       ri = 0.5, rf = 0.004, teta = -0.005;

  if (!ok)
  {
    char               *s = NULL;

    if ((s = getenv("LRATEDEFS")) != NULL)
    {
      char                ebuf[256];
      char               *argv[8];
      int                 argc;

      memset(argv, 0, sizeof (argv));
      strlcpy(ebuf, s, sizeof (ebuf));
      argc = str2tokens(ebuf, 8, argv, ",; ");
      if (argv[0] != NULL)
        srate = atof(argv[0]);
      if (argv[1] != NULL)
        ri = atoi(argv[1]);
      if (argv[2] != NULL)
      {
        rf = atof(argv[2]);
        lrate = rf;
      }
      if (argv[3] != NULL)
        teta = atof(argv[3]);
    }
    ok = TRUE;
  }

  switch (srate)
  {
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
static              bool
decode_lr_options(lrOpts, optarg)
     lr_opts_T          *lrOpts;
     char               *optarg;
{
  char                buf[1024];
  char               *argv[4];
  int                 argc;

  if (optarg == NULL)
    return FALSE;

  strlcpy(buf, optarg, sizeof buf);
  argc = str2tokens(buf, 4, argv, "=");
  if (argc < 2)
    return FALSE;

  if (STRCASEEQUAL(argv[0], "LR_LRATE"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_USE_RAW_MSG"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_RAW_LENGTH"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_BODY_LENGTH"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_USE_BODY"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_USE_HEADERS"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_CLEAN_UP_HEADERS"))
  {

    return TRUE;
  }
  if (STRCASEEQUAL(argv[0], "LR_CLEANUP_DATES"))
  {

    return TRUE;
  }

  return TRUE;
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
         "        -m  : feedback miss probability ([0,1])\n"
         "        -R  : asymptotic learning rate\n"
         "        -x mode : where mode in learn, class, simulate\n", arg);
  printf("\n     %s\n     %s\n\n",
         PACKAGE, "Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz");
}
