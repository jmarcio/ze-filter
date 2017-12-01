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
 *  Creation     : december 2005
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
#include <ze-bfilter.h>
#include <ze-bcheck.h>

static void         usage(char *);

/*
** MY_BDATA
*/


/* *******************/


static void         print_histogram(int *histo, int dim, double step);

static int          print_db_info(void *k, void *v, void *arg);

typedef struct
{
  int                 ns;
  int                 nh;
  int                 nfs;
  int                 nfh;
  int                 nm;
} db_info_T;



static int          sfilter_token_cmp(void *a, void *b);
static void         group_token_files(int argc, char **argv, int msgMin,
                                      char *crypt);
static void         agregate_tokens(int argc, char **argv, bool multinomial);

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
/*
** General options
** 
** - h   : help
**
** - l   : learn
**   - s : spam
**   - h : ham
**
** - c   : check
**    
*/

#define    OPTSTR_INIT      "hiclga"
#define    OPTSTR_HELP      "x"
#define    OPTSTR_INFO      "x"
#define    OPTSTR_CHECK     "x"
#define    OPTSTR_LEARN     "x"
#define    OPTSTR_GROUP     "x"
#define    OPTSTR_AGGREG    "x"

#define    OPTSTR_GENERAL   "vpM:"


#define    OPT_INIT       0
#define    OPT_HELP       1
#define    OPT_INFO       5
#define    OPT_CHECK      3
#define    OPT_LEARN      2
#define    OPT_GROUP      4
#define    OPT_AGGREG     5

#define    SET_STATE(new)			\
  do {						\
    opt_state = new;				\
    switch (opt_state) {			\
      case OPT_INIT:				\
	opt_str = OPTSTR_GENERAL OPTSTR_INIT;	\
	  break;				\
      case OPT_HELP:				\
	opt_str = OPTSTR_GENERAL OPTSTR_HELP;	\
	break;					\
      case OPT_INFO:				\
	opt_str = OPTSTR_GENERAL OPTSTR_INFO;   \
	break;					\
      case OPT_CHECK:				\
	opt_str = OPTSTR_GENERAL OPTSTR_CHECK;	\
	break;					\
      case OPT_LEARN:				\
        opt_str = OPTSTR_GENERAL OPTSTR_LEARN;	\
	break;					\
      case OPT_GROUP:				\
	opt_str = OPTSTR_GENERAL OPTSTR_GROUP;	\
	break;					\
    }						\
  } while (0)



int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  char               *fname;
  sfilter_cli_T       data;
  int                 argi = 0;
  int                 nb = 0;

  char               *opts = "";

  bool                info = FALSE;

  bool                group = FALSE;
  bool                agregate = FALSE;

  char               *crypt = NULL;

  int                 nbMsgMin = 4;

  int                 opt_state = OPT_INIT;
  char               *opt_str = OPTSTR_INIT;

  memset(&data, 0, sizeof data);
  data.bcheck.nbt = 21;
  data.spam = FALSE;
  data.maxSize = 100000;
  data.check = TRUE;
  data.verbose = FALSE;
  data.histogram = FALSE;
  data.progress = TRUE;
  data.dbname = ZE_CDBDIR "/ze-bayes.db";
  data.nbt = 21;
  data.uprob = 0.5;

  if (access("ze-bayes.db", R_OK) == 0)
    data.dbname = "ze-bayes.db";

  zeLog_SetOutput(FALSE, TRUE);

  ze_logLevel = 5;

  SET_STATE(OPT_INIT);
  {
    int                 c;
    int                 optlevel = 0;

    opts = "lchsvn:xpM:t:u:b:igdae:m:";

    while ((c = getopt(argc, argv, opts)) != -1)
    {

#if 0
      /* First choice - what to do */
      if (opt_state == OPT_INIT)
      {
        switch (c)
        {
            /* help */
          case 'h':
            SET_STATE(OPT_HELP);
            usage(argv[0]);
            exit(0);
            break;
            /* get info */
          case 'i':
            SET_STATE(OPT_INFO);
            info = TRUE;
            break;

            /* Check options */
          case 'c':
            SET_STATE(OPT_CHECK);
            data.check = TRUE;
            break;

            /* learn options */
          case 'l':
            SET_STATE(OPT_LEARN);
            data.check = FALSE;
            break;

            /* default */
          default:
            opt_state = OPT_INIT;
            usage(argv[0]);
            printf("Error ... \n");
            exit(0);
        }

        continue;
      }

      /* Learning sub-options */
      if (opt_state == OPT_LEARN)
      {

        continue;
      }

      /* Message checking sub-options */
      if (opt_state == OPT_CHECK)
      {

      }

      /* Message checking sub-options */
      if (opt_state == OPT_INFO)
      {

      }

      /* Message checking sub-options */
      if (opt_state == OPT_GROUP)
      {

      }
#else

      switch (c)
      {
          /* help */
        case 'h':
          usage(argv[0]);
          exit(0);
          break;

          /* get info */
        case 'i':
          info = TRUE;
          break;

          /* Check options */
        case 'c':
          data.check = TRUE;
          break;
        case 'x':
          data.histogram = TRUE;
          break;
        case 'b':
          data.dbname = optarg;
          break;
        case 't':
          data.nbt = atoi(optarg);
          break;
        case 'u':
          data.uprob = atof(optarg);
          break;

          /* learn options */
        case 'e':
          crypt = optarg;
          break;

        case 'l':
          data.check = FALSE;
          break;

        case 's':
          data.spam = TRUE;
          break;

          /* general options */
        case 'v':
          data.verbose = TRUE;
          ze_logLevel++;
          break;
        case 'p':
          data.progress = !data.progress;
          break;
        case 'M':
          data.maxSize = atoi(optarg);
          break;

        case 'g':
          group = TRUE;
          break;
        case 'm':
          nbMsgMin = atoi(optarg);
          break;

        case 'a':
          agregate = TRUE;
          break;

          /* default */
        default:
          usage(argv[0]);
          printf("Error ... \n");
          exit(0);
      }
#endif
    }
  }

  set_mime_debug(FALSE);

  argi = optind;

  {
    char               *dbname = NULL;

    if ((info || data.check) && !agregate && !group)
      dbname = data.dbname;

    if (!bfilter_init(dbname))
    {
      fprintf(stderr, "Error while opening %s database\n",
              STRNULL(dbname, "null"));
      return 1;
    }
  }

  if (!data.check)
  {
    set_bfilter_db_crypt(HASH_PLAIN);
    if (crypt != NULL)
    {
      zeStr2Upper(crypt);
      if (STRCASEEQUAL(crypt, "PLAIN"))
        set_bfilter_db_crypt(HASH_PLAIN);
      if (STRCASEEQUAL(crypt, "MD5"))
        set_bfilter_db_crypt(HASH_MD5);
      if (STRCASEEQUAL(crypt, "SHA1"))
        set_bfilter_db_crypt(HASH_SHA1);
    }
  }

  while (argi < argc && *argv[argi] == '-')
    argi++;

  if (info)
  {
    db_info_T           db_info = { 0, 0, 0 };

    memset(&db_info, 0, sizeof (db_info));
    (void) smodel_db_info("msgs:", print_db_info, &db_info);
    printf("\n");

    memset(&db_info, 0, sizeof (db_info));
    (void) smodel_db_info("info:", print_db_info, &db_info);
    printf("\n");
    printf("** TOTAL :\n");
    printf("   Spams mbox  %6d\n", db_info.nfs);
    printf("   Spams       %6d\n", db_info.ns);
    printf("   Hams  mbox  %6d\n", db_info.nfh);
    printf("   Hams        %6d\n", db_info.nh);
    printf("\n");
    exit(0);
  }

  if (group)
  {
    argc -= argi;
    argv += argi;

    crypt = STRNULL(crypt, "PLAIN");

    group_token_files(argc, argv, nbMsgMin, crypt);

    exit(0);
  }

  if (agregate)
  {
    argc -= argi;
    argv += argi;

    agregate_tokens(argc, argv, FALSE);

    exit(0);
  }

  /* learning or checking */
  if (argi < argc)
  {
    char               *fname = NULL;

    while (argi < argc)
    {
      fname = argv[argi++];

      printf("# Checking mailbox %s\n", fname);

      memset(data.histo, 0, sizeof (data.histo));
      if (!data.check)
      {
        char                hostname[256];
        char                date[256];
        time_t              now;
        char               *p;

        printf("__BEGIN__\n");
        printf("FILE  %ld %c %s\n", time(NULL) / 86400, (data.spam ? 'S' : 'H'),
               fname);

        memset(hostname, 0, sizeof (hostname));
        if (gethostname(hostname, sizeof (hostname)) < 0)
          strlcpy(hostname, "(unknown)", sizeof (hostname));
        now = time(NULL);
        CTIME_R(&now, date);
        if ((p = strchr(date, '\n')) != NULL)
          *p = '\0';
        printf("CRYPT %s\n", STRNULL(crypt, "PLAIN"));
        printf
          ("INFO  %s type=(%s) time=(%ld) date=(%s) hostname=(%s) crypt=(%s)\n",
           fname, (data.spam ? "Spam" : "Ham"), now, date, hostname,
           STRNULL(crypt, "PLAIN"));
      }

      nb += mbox_handle(fname, sfilter_cli_handle_message, &data);

      if (!data.check)
      {
        printf("MSGS  %ld %-10s %c %6d\n", time(NULL), "NOID",
               (data.spam ? 'S' : 'H'), nb);
        printf("__END__\n");
      }
      if (data.check && data.histogram)
        print_histogram(data.histo, 20, 0.05);
    }
  } else
  {
    usage(argv[0]);
    exit(1);
  }

  if (data.check)
    bfilter_close();

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static char        *vtags[] = {
  "type",
  "time",
  "date",
  "hostname",
  "count",
  "crypt",
  NULL
};


static int
print_db_info(k, v, arg)
     void               *k;
     void               *v;
     void               *arg;
{
  db_info_T          *info = (db_info_T *) arg;

  ASSERT(k != NULL);
  ASSERT(v != NULL);

  /* printf(" ** %-32s : %s\n", k, v); */
  if (STRNCASEEQUAL(k, "info:", strlen("info:")))
  {
    char               *p = NULL;
    char              **tag;

    if ((p = strchr(k, ':')) != NULL)
      *p++ = '\0';
    printf("** FILE : %s\n", p);

    for (tag = vtags; *tag != NULL; tag++)
    {
      long                pf, pi;
      char                buf[256];

      snprintf(buf, sizeof (buf), "%s=\\([^\\)]*\\)", *tag);
      if (zeStrRegex(v, buf, &pi, &pf, TRUE))
      {
        char               *s, *t;

        zeSafeStrnCpy(buf, sizeof (buf), (char *) v + pi, pf - pi);

        if ((t = strrchr(buf, ')')) != NULL)
          *t = '\0';
        if ((t = strchr(buf, '=')) != NULL)
          *t++ = '\0';
        if (*t == '(')
          t++;

        if (STRCASEEQUAL(*tag, "count"))
        {
          int                 s, h;

          if (sscanf(t, "%d %d", &s, &h) == 2)
          {
            printf("   %-12s Spams/Hams = %5d/%5d\n", buf, s, h);
            info->ns += s;
            info->nh += h;
          }
          continue;
        }
        if (STRCASEEQUAL(*tag, "type"))
        {
          if (STRCASEEQUAL(t, "spam"))
            info->nfs++;
          else
            info->nfh++;
        }
        printf("   %-12s %s\n", buf, t);
      }
    }
  }

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
print_histogram(histo, nmax, step)
     int                *histo;
     int                 nmax;
     double              step;
{
  char                line[128];
  int                 i, nm;

  nm = 0;
  for (i = 0; i <= nmax; i++)
  {
    int                 nb;

    nb = histo[i];
    nm += nb;
    if (nb > 80)
      nb = 80;
    zeStrSet(line, '*', nb);
    if (nb > 80)
      strlcat(line, "->", sizeof (line));
    printf("%3d : %6.3f %5d %s\n", i, i * step, histo[i], line);
  }
  printf("    :        %5d Messages\n", nm);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int
sfilter_token_cmp(void *a, void *b)
{
  sfilter_token_T    *ta = (sfilter_token_T *) a;
  sfilter_token_T    *tb = (sfilter_token_T *) b;

  return strcmp(ta->token, tb->token);
}

typedef struct
{
  int                 nbmin;

  int                 nts;
  int                 nth;

  int                 nfspam;
  int                 nfham;

  int                 nfshar;
  int                 nftot;
} btok_arg_T;

static int
btok_browse(void *data, void *arg)
{
  sfilter_token_T    *tok = (sfilter_token_T *) data;
  btok_arg_T         *btok_arg = (btok_arg_T *) arg;
  int                 nbmin = btok_arg->nbmin;

  if (tok->nbs >= nbmin || tok->nbh >= nbmin)
  {
    btok_arg->nts += tok->nbs;
    btok_arg->nth += tok->nbh;

    if (tok->nbs > 0)
      btok_arg->nfspam++;
    if (tok->nbh > 0)
      btok_arg->nfham++;

    if (tok->nbs > 0 && tok->nbh > 0)
      btok_arg->nfshar++;
    btok_arg->nftot++;

    printf("%-40s %d %d\n", tok->token, tok->nbs, tok->nbh);
    return 1;
  }
  return 0;
}


static void
group_token_files(argc, argv, msgMin, scli_crypt)
     int                 argc;
     char              **argv;
     int                 msgMin;
     char               *scli_crypt;
{
  JBT_T               bt = JBT_INITIALIZER;
  LISTR_T            *list = NULL, *plist;
  char               *fname = NULL;
  int                 i, nl = 0, nt = 0;
  int                 icli_crypt = HASH_PLAIN;
  int                 file_crypt = HASH_PLAIN;

  if (!jbt_init(&bt, sizeof (sfilter_token_T), sfilter_token_cmp))
    goto fin;

  icli_crypt = hash_label2code(scli_crypt);
  scli_crypt = hash_code2label(icli_crypt);

  for (i = 0; i < argc; i++)
  {
    FILE               *fin = NULL;

    fname = argv[i];

    printf("# Grouping %s\n", fname);

    if ((fin = fopen(fname, "r")) != NULL)
    {
      char                buf[1024];

      while (fgets(buf, sizeof (buf), fin) != NULL)
      {
        char               *p;
        int                 tok_crypt = HASH_PLAIN;

        nl++;
        if ((p = strchr(buf, '\n')) != NULL)
          *p = '\0';

        if (STRNCASEEQUAL(buf, "msgs:", strlen("msgs:")))
        {
          if (zeStrRegex(buf, "^msgs:[^ ]*[.]dtok[ ]+", NULL, NULL, TRUE))
            list = zeLinkedList_Add(list, buf, 1, NULL, 0);
        }

        if (STRNCASEEQUAL(buf, "crypt:", strlen("crypt:")))
        {
          char               *p;

          list = zeLinkedList_Add(list, buf, 1, NULL, 0);

          p = buf;
          p += strcspn(p, " \t");
          p += strspn(p, " \t");

          file_crypt = hash_label2code(p);

          if (i == 0)
          {
            if (icli_crypt == HASH_PLAIN && file_crypt != HASH_PLAIN)
            {
              ZE_MessageInfo(8, "# Changing global crypt option %s -> %s %d\n",
                           scli_crypt, hash_code2label(file_crypt));
              icli_crypt = file_crypt;
              scli_crypt = hash_code2label(icli_crypt);
            }
          }

          if (icli_crypt != HASH_PLAIN &&
              file_crypt != HASH_PLAIN && icli_crypt != file_crypt)
          {
            ZE_MessageWarning(9,
                            "Warning : skipping file %s : encoding incompatibility",
                            fname);
            break;
          }
        }

        if (STRNCASEEQUAL(buf, "info:", strlen("info:")))
        {
          list = zeLinkedList_Add(list, buf, 1, NULL, 0);
        }

        if (STRNCASEEQUAL(buf, "token:", strlen("token:")))
        {
          sfilter_token_T     tok, *t;
          char               *k, *v, *p;
          int                 ns, nh;

          k = buf;
          v = NULL;
          ns = nh = 0;

          if ((p = strchr(buf, ' ')) != NULL)
          {
            v = p + strspn(p, " \t");
            *p = '\0';
          }

          if (v == NULL)
          {
            continue;
          }

          if (sscanf(v, "%d %d", &ns, &nh) < 2)
          {
            ZE_LogMsgWarning(0, "Error : %s %s", k, v);

            continue;
          }

          memset(&tok, 0, sizeof (tok));
          if (icli_crypt != HASH_PLAIN && file_crypt == HASH_PLAIN)
          {
            char                dig[64];

            p = strchr(buf, ':');
            if (p != NULL && *p != '\0')
              p++;

            switch (icli_crypt)
            {
              case HASH_MD5:
              case HASH_SHA1:
                str2hash2hex(icli_crypt, dig, p, sizeof (dig));
                snprintf(tok.token, sizeof (tok.token), "TOKEN:%s", dig);
                break;
              default:
                break;
            }
          } else
            strlcpy(tok.token, buf, sizeof (tok.token));
          tok.nbs = ns;
          tok.nbh = nh;
          if ((t = jbt_get(&bt, &tok)) == NULL)
          {
            if (!jbt_add(&bt, &tok))
            {
              ZE_LogMsgError(0, "ERROR inserting new token");
              continue;
            }
          } else
          {
            t->nbs += ns;
            t->nbh += nh;
          }

          nt++;
          continue;
        }
      }
      fclose(fin);
    }
  }

  printf("# Tokens added : %d\n", nt);

  {
    int                 nts, nth;
    btok_arg_T          arg;

    nts = nth = 0;

    for (plist = list; plist != NULL; plist = plist->next)
    {
      char               *v;
      int                 ns, nh;

      printf("%s\n", plist->key);

      if (STRNCASEEQUAL(plist->key, "msgs:", strlen("msgs:")))
      {
        v = plist->key;
        v += strcspn(v, " \t");
        v += strspn(v, " \t");

        if (sscanf(v, "%d %d", &ns, &nh) < 2)
        {
          ZE_LogMsgWarning(0, "Error : %s", v);

          continue;
        }
        nts += ns;
        nth += nh;
      }
    }
    printf("CRYPT:%-32s %s\n", "TOKENS", scli_crypt);
    printf("MSGS:%-32s %d %d\n", "__TOTAL__", nts, nth);
    printf("MSGS:%-32s %d %d\n", "total-tokens", nts, nth);
    printf("MSGS:%-32s %d %d\n", "total-msgs", nts, nth);

    memset(&arg, 0, sizeof (arg));
    arg.nbmin = msgMin;

    nt = jbt_browse(&bt, btok_browse, &arg);

    printf("Count:%-32s %d %d\n", "msgs", nts, nth);
    printf("Count:%-32s %d %d\n", "features", arg.nfspam, arg.nfham);
    printf("Count:%-32s %d %d\n", "Features-shared-total", arg.nfshar,
           arg.nftot);
    printf("Count:%-32s %d %d\n", "tokens", arg.nts, arg.nth);
  }

  jbt_destroy(&bt);
  zeLinkedList_Clear(list, NULL);

  printf("# Tokens browsed : %d\n", nt);

fin:
  exit(0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
dtok_browse(void *data, void *arg)
{
  sfilter_token_T    *tok = (sfilter_token_T *) data;
  int                *nbmin = (int *) arg;

  printf("TOKEN:%-40s %d %d\n", tok->token, tok->nbs, tok->nbh);

  return 1;
}


static void
agregate_tokens(argc, argv, multinomial)
     int                 argc;
     char              **argv;
     bool                multinomial;
{
  JBT_T               bt = JBT_INITIALIZER;

  LISTR_T            *list = NULL, *plist;
  char               *fname = NULL;
  int                 i, nl = 0, nt = 0;

  /*char               *crypt = NULL; */

  for (i = 0; i < argc; i++)
  {
    FILE               *fin = NULL;
    int                 nts, nth;

    if (!jbt_init(&bt, sizeof (sfilter_token_T), sfilter_token_cmp))
      goto fin;

    fname = argv[i];

    printf("# Aggregating tokens from file : %s\n", fname);

    if ((fin = fopen(fname, "r")) != NULL)
    {
      char                buf[1024];
      bool                spam = FALSE;
      int                 msgs = 0;
      char                info[1024];
      char                crypt[16];
      char                id[64];

      memset(crypt, 0, sizeof (crypt));
      memset(info, 0, sizeof (info));
      memset(id, 0, sizeof (id));
      while (fgets(buf, sizeof (buf), fin) != NULL)
      {
        char               *p;

        nl++;
        if ((p = strchr(buf, '\n')) != NULL)
          *p = '\0';

        if (STRNCASEEQUAL(buf, "file ", strlen("file ")))
        {
          list = zeLinkedList_Add(list, buf, 1, NULL, 0);
        }

        if (STRNCASEEQUAL(buf, "crypt ", strlen("crypt ")))
        {
          char               *p;

          p = buf;
          if (strlen(p) > 0)
            p += strcspn(p, " \t");
          if (strlen(p) > 0)
            p += strspn(p, " \t");

          strlcpy(crypt, p, sizeof (crypt));
          list = zeLinkedList_Add(list, buf, 1, NULL, 0);
        }

        if (STRNCASEEQUAL(buf, "info ", strlen("info ")))
        {
          char               *p;

          p = buf;
          if (strlen(p) > 0)
            p += strcspn(p, " \t");
          if (strlen(p) > 0)
            p += strspn(p, " \t");
          if (strlen(p) > 0)
            p += strcspn(p, " \t");
          if (strlen(p) > 0)
            p += strspn(p, " \t");

          if (strlen(p) > 0)
          {
            strlcpy(info, p, sizeof (info));
            list = zeLinkedList_Add(list, p, 1, NULL, 0);
          }
        }

        if (STRNCASEEQUAL(buf, "msgs ", strlen("msgs ")))
        {
          int                 bargc = 0;
          char               *bargv[8];
          int                 n = 0;

          bargc = zeStr2Tokens(buf, 8, bargv, " ");
          if (bargc == 0)
            continue;

          spam = STRCASEEQUAL(bargv[3], "s");
          errno = 0;
          n = zeStr2long(bargv[4], NULL, 0);
          if (errno == 0 && n > 0)
            msgs = n;

          msgs = atoi(bargv[4]);

          list = zeLinkedList_Add(list, buf, 1, NULL, 0);
        }

        if (STRNCASEEQUAL(buf, "token ", strlen("token ")))
        {
          sfilter_token_T     tok, *t;
          char               *k, *v, *p;
          int                 ns, nh;

          int                 bargc = 0;
          char               *bargv[8];
          int                 nb = 1;

          bargc = zeStr2Tokens(buf, 8, bargv, " ");
          if (bargc == 0)
            continue;

          spam = STRCASEEQUAL(bargv[3], "s");
          memset(&tok, 0, sizeof (tok));

          /* XXX encode : PLAIN / MD5 / SHA1 */
          strlcpy(tok.token, bargv[5], sizeof (tok.token));

          if (multinomial)
            nb = atoi(bargv[4]);

          if (spam)
            tok.nbs = nb;
          else
            tok.nbh = nb;
          if ((t = jbt_get(&bt, &tok)) == NULL)
          {
            if (!jbt_add(&bt, &tok))
            {
              ZE_LogMsgError(0, "ERROR inserting new token");
              continue;
            }
          } else
          {
            if (spam)
              t->nbs += nb;
            else
              t->nbh += nb;
          }

          nt++;
          continue;
        }
      }
      fclose(fin);

      printf("__BEGIN__\n");
      nts = nth = 0;

      if (spam)
        nts = msgs;
      else
        nth = msgs;

      {
        time_t              now;
        char                buf[64], *p;

        now = time(NULL);
        CTIME_R(&now, buf);
        if ((p = strchr(buf, '\n')) != NULL)
          *p = '\0';

        printf("DATE:%-40s %ld date=(%s)\n", fname, now, buf);

        printf("MSGS:%-40s %7ld %7ld\n", fname, nts, nth);
        printf("CRYPT:%-40s %s\n", fname, crypt);

        printf("INFO:%-40s %s count=(%d %d)\n", fname, info, nts, nth);
      }

      nt = jbt_browse(&bt, dtok_browse, NULL);
      jbt_clear(&bt);

      printf("__END__\n");
    }
    zeLinkedList_Clear(list, NULL);
  }

  jbt_destroy(&bt);

  printf("# Tokens browsed : %d\n", nt);

fin:
  exit(0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
usage(app)
     char               *app;
{
  char               *acpy = NULL, *appname;

  acpy = strdup(app);
  appname = basename(acpy);

  printf("Usage : %s [ -h | -c options | -l options]\n"
         "  -h   : help message (you're reading it...)\n"
         "  -c   : check message/mailbox spamicity\n"
         "         Check options\n"
         "         -b file       : tokens database\n"
         "         -t N          : number of tokens\n"
         "         -u prob       : probability associated to unknown tokens\n"
         "         -x            : show histogram of scores\n"
         "  -l   : learn message/mailbox\n"
         "         Learn options\n"
         "         -s            : message/mbox is spam\n"
         "         General options\n"
         "         -f flag,flag  : tokenizer flags\n"
         "         -p            : don't show progress\n"
         "         -v            : verbose\n"
         "         -M size       : max single message size\n"
         "  -i \n"
         "  -e plain | md5 | sha1\n"
         "  -g group .dtok -> .tok\n"
         "  -a group .tok  -> .txt\n"
         "  -m minimum messages count\n"
         "\n"
         "\n"
         "  %s\n"
         "  Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz - (C) 2001 ... 2006\n"
         "  Compiled on %s %s\n\n", appname, PACKAGE, __DATE__, __TIME__);

  FREE(acpy);
}
