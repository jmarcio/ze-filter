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


#include <j-sys.h>
#include <j-chkmail.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define             DEF_TUPLE         "NET,HOST,FULL"
#define             DEF_TIMES         "0,1000,0,1000"

struct conf_T
{
  char               *workdir;
  int                 count;

  char               *times;
  char               *tuples;
};

typedef struct conf_T conf_T;

#define CONF_INITIALIZER   {"/tmp/j-greyd", 10000, DEF_TIMES, DEF_TUPLE}

static char        *workdir = "/tmp/jgreyd";

static char        *ntuple = NULL;

static char        *tconst = NULL;

static int          do_test(conf_T * cargs);

static void         usage(char *);


int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 i;

  conf_T              cargs = CONF_INITIALIZER;

  set_log_output(TRUE, TRUE);

  log_level = 7;

  /*
   ** 1. Read configuration parameters
   */
  {
    const char         *args = "ht:dvw:n:l:c:";
    int                 c;

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
          /* help */
        case 'h':
          usage(argv[0]);
          exit(0);
          break;

          /* time constants */
        case 't':
          tconst = optarg;
          break;

          /* verbose */
        case 'v':
          log_level++;
          break;

          /* group */
        case 'l':
          log_level = atoi(optarg);
          break;

        case 'c':
          cargs.count = atoi(optarg);
          break;

          /* Work directory */
        case 'w':
          workdir = optarg;
          break;

        case 'n':
          ntuple = optarg;
          break;

        default:
          usage(argv[0]);
          printf("Error ... \n");
          exit(0);
      }
    }

  }


  if (!grey_init(workdir, FALSE, GREY_SERVER))
  {

    return 1;
  }

  ntuple = STRNULL(ntuple, DEF_TUPLE);
  if (ntuple != NULL)
  {
#define NTP   3
    int                 argc;
    char               *argv[NTP];
    char               *s = NULL;

    if ((s = strdup(ntuple)) != NULL)
    {
      memset(argv, 0, sizeof (argv));
      argc = str2tokens(s, NTP, argv, ",");
      for (i = 0; i < NTP; i++)
        argv[i] = STRNULL(argv[i], "");
      (void) grey_set_tuples(argv[0], argv[1], argv[2]);
    }
  }

  tconst = STRNULL(tconst, DEF_TIMES);
  if (tconst != NULL)
  {
#define NTC   4
    int                 argc;
    char               *argv[NTC];
    time_t              tc[NTC];
    char               *s = NULL;

    if ((s = strdup(tconst)) != NULL)
    {
      memset(argv, 0, sizeof (argv));
      memset(tc, 0, sizeof (tc));
      argc = str2tokens(s, NTC, argv, ",");
      for (i = 0; i < NTC; i++)
      {
        argv[i] = STRNULL(argv[i], "0");
        tc[i] = (time_t) str2time(argv[i], NULL, 0);
      }
      (void) grey_set_delays(tc[0], tc[1], tc[2], tc[3]);
    }
  }

  do_test(&cargs);

  return 0;
}



/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static int
do_test(conf_T * cargs)
{
  char               *ip = "", *from = "", *to = "", *hostname = "";
  int                 n = 0;
  char                buf[256];

  time_t              ti, tf;

  ip = "1.1.1.1";
  from = "nobody@foss.jose-marcio.org";
  hostname = "nowhere.foss.jose-marcio.org";
  to = "grey@foss.jose-marcio.org";

  ti = time_ms();
  for (n = 0; n < cargs->count; n++)
  {
    int                 r = GREY_OK;
    char               *s = NULL;
    bool                new = FALSE;
    bool                can_validate = TRUE;

    snprintf(buf, sizeof (buf), "grey-%d@foss.jose-marcio.org", n);
    to = buf;

    r = grey_check(ip, from, to, hostname, &new, can_validate);
    switch (r)
    {
      case GREY_OK:
        s = "OK";
        break;
      case GREY_WAIT:
        s = "WAIT";
        break;
      case GREY_ERROR:
        s = "ERROR";
        break;
      case GREY_REJECT:
        s = "REJECT";
        break;
    }
  }
  tf = time_ms();

  MESSAGE_INFO(1, "Entries checked : %5d\n", n);
  MESSAGE_INFO(1, "Time elapsed    : %5d ms", tf - ti);

  return 0;
}



/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
usage(arg)
     char               *arg;
{
  printf("Usage : %s options\n"
         "  %s\n" "  Compiled on %s\n", arg, PACKAGE, __DATE__ " " __TIME__);
}
