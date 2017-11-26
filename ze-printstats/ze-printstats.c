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

extern int          log_level;


void                usage();



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
main(int argc, char **argv)
{
  const char         *args = "hpgatdcfvqxl:m:n:";
  int                 c;
  int                 jp = 0, jg = 0;
  int                 jt = 0;
  int                 jq = 0, jx = 0;

  int                 verbose = 0;
  int                 hostnames = 0;
  bool                newformat = FALSE;
  uint32_t            flags = 0;

  int                 nbRecs = 0;
  int                 summary_type = H_SUMMARY;

  char                host[256];

  long                dt = 3600;

  set_log_output(FALSE, TRUE);

  log_level = 9;
  memset(host, 0, sizeof (host));

  while ((c = getopt(argc, argv, args)) != -1)
  {
    switch (c)
    {
      case 'h':
        usage();
        exit(0);
        break;
      case 'p':
        jp = 1;
        jg = 0;
        break;
      case 'g':
        jp = 0;
        jg = 1;
        break;
      case 'a':
        jp = jg = 1;
        break;
      case 't':
        jt++;
        break;
      case 'c':
	cf_opt.arg_c = optarg;
        break;
      case 'd':
        hostnames = TRUE;
        break;
      case 'v':
        verbose = 1;
        break;
      case 'f':
        newformat = TRUE;
        break;
      case 'q':
        jq = 1;
        break;
      case 'x':
        jx = 1;
        break;
      case 'l':
        dt = str2time(optarg, NULL, 3600);
        break;
      case 'm':
        if (jq > 0)
        {
          if (strcmp("s", optarg) == 0)
          {
            summary_type = H_SUMMARY;
            break;
          }
          if (strcmp("e", optarg) == 0)
          {
            summary_type = H_EMPTY;
            break;
          }
          if (strcmp("re", optarg) == 0)
          {
            summary_type = H_REJ_EMPTY;
            break;
          }
          if (strcmp("rg", optarg) == 0)
          {
            summary_type = H_REJ_GREY;
            break;
          }
          if (strcmp("b", optarg) == 0)
          {
            summary_type = H_BADRCPT;
            break;
          }
          if (strcmp("rb", optarg) == 0)
          {
            summary_type = H_REJ_BADRCPT;
            break;
          }
          if (strcmp("rm", optarg) == 0)
          {
            summary_type = H_REJ_BADMX;
            break;
          }
          if (strcmp("ro", optarg) == 0)
          {
            summary_type = H_REJ_OPEN;
            break;
          }
          if (strcmp("t", optarg) == 0)
          {
            summary_type = H_REJ_THROTTLE;
            break;
          }
          if (strcmp("rt", optarg) == 0)
          {
            summary_type = H_REJ_THROTTLE;
            break;
          }
          if (strcmp("r", optarg) == 0)
          {
            summary_type = H_RESOLVE;
            break;
          }
          if (strcmp("rr", optarg) == 0)
          {
            summary_type = H_REJ_RESOLVE;
            break;
          }
          if (strcmp("c", optarg) == 0)
          {
            summary_type = H_REJ_REGEX;
            break;
          }
          if (strcmp("x", optarg) == 0)
          {
            summary_type = H_XFILES;
            break;
          }
          if (strcmp("st", optarg) == 0)
          {
            summary_type = H_SPAMTRAP;
            break;
          }
          summary_type = H_SUMMARY;
          break;
        }
        if (jt > 0)
        {
	  flags |= smtprate_str2flags(optarg);
          break;
        }
        break;
      case 'n':
        nbRecs = atoi(optarg);
        break;
      default:
        printf("Error ... \n");
        exit(0);
    }
  }
  if (optind < argc && strlen(argv[optind]) > 0)
    snprintf(host, sizeof (host), "%s", argv[optind]);

  if (!(jp || jt || jg || jq || jx))
  {
    usage();
    exit(1);
  }

  init_default_file_extensions();

  if (cf_opt.arg_c != NULL)
    conf_file = cf_opt.arg_c;

  configure("ze-printstats", conf_file, FALSE);

  if (jp > 0 || jg > 0)
  {
    if (dump_state(STDOUT_FILENO, jp, jg, verbose, newformat) > 0)
      printf("ERROR\n");
  }

  if (0)
  {
    char                dbdir[256];
    char               *dbenv = getenv("DBENV");

    char               *dir;

    if (dbenv != NULL && STRCASEEQUAL(dbenv, "yes"))
    {
      dir = cf_get_str(CF_WDBDIR);

      memset(dbdir, 0, sizeof (dbdir));
      if (dir != NULL && strlen(dir) > 0)
        snprintf(dbdir, sizeof (dbdir), "%s/%s", dir, "db");

      if (!open_work_db_env(dbdir, ZE_WDBDIR, TRUE))
      {
      }
    }
  }

  resolve_cache_init(NULL, RESOLVE_CACHE_RD);

  if (jt > 0)
  {
    if (dt == 0)
      dt = 10 MINUTES;
    if (flags == 0)
    {
      SET_BIT(flags, RATE_CONN);
      SET_BIT(flags, RATE_RCPT);
    }
    smtprate_read_table(NULL);
    smtprate_update_table(3600);
    smtprate_print_table(STDOUT_FILENO, jt - 1, verbose, hostnames, dt, flags, nbRecs);
  }

  if (jq > 0)
  {
    char               *p = NULL, *name = NULL;
    char                ip[256];

    *ip = '\0';
    if (strlen(host) > 0)
    {
      if (get_hostbyname(host, ip, sizeof (ip)))
      {
        name = host;
        p = ip;
      } else
        p = host;
    }

    printf("%-30s : %s\n", "Version", PACKAGE);

    (void) raw_history_open(TRUE);
    res_history_update(NULL, p, 0, dt, verbose);
    res_history_summary(NULL, p, 0, dt, verbose, hostnames, summary_type, nbRecs);
    (void) raw_history_close();
  }

  if (!(jp || jt || jg || jq || jx))
    usage();

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
usage()
{
  printf("Usage : ze-printstats options\n"
         "    ze-printstats -c conf_file\n"
         "    ze-printstats -a | -p | -g\n"
         "        -p   : print running process ze-filter counters\n"
         "        -g   : print ze-filter counters from last reset\n"
         "        -a   : print both counters\n"
         "    ze-printstats -t[td]\n"
         "        -t   : throttle data (summary)\n"
         "        -tt  : throttle data (detail)\n"
         "        -d   : resolve IP addresses\n"
         "        -m x1,x2,...\n"
         "               all,conn,rcpt,bounce,msgs,vol,svc\n"
         "        -l   : period of interest - default value : 10m\n"
         "               period shall be smaller than 20m - default unit : s\n"
         "        -n N : max number of records to print\n"         
         "    ze-printstats -q [-l dt [s|m|h|d]] [[-v | ip | hostname] | [-m x]]\n"
         "        -q   : query gateway activity\n"
         "        -l   : period of interest - default unit : secs\n"
         "        -v   : verbose - meaninful only if gateway not specified\n"
         "               prints summary for each client gateway\n"
         "        -m x : select type of summary\n"
         "               x = s  : Connection summary\n"
         "               x = e  : Client doing empty connections\n"
         "               x = re : Client doing empty connections - reject\n"
         "               x = ro : Too much open connections - reject\n"
         "               x = rt : Connection rate too high - reject\n"
         "               x = rb : Too many bad recipients - reject\n"
         "               x = rg : Clients with messages rejected by greylisting\n"
         "               x = r  : Clients with bad DNS resolution\n"
         "               x = rr : Clients with bad DNS resolution - reject\n"
         "               x = c  : Clients being rejected by content checking\n"
         "               x = x  : Clients sending X-Files or Virus\n"
         "               x = rm : Messages with BAD MX for sender domain\n"
         "               x = st : Clients sending messages to SpamTraps\n"
         "        -n N : max number of records to print\n\n"
         "  %s\n"
         "  Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz - (C) 2001, 2002, 2003\n"
         "  Compiled on %s %s\n\n", PACKAGE, __DATE__, __TIME__);
}
