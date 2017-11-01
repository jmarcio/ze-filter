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
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#include <j-sys.h>

#include "ze-filter.h"

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void                usage();

#define RES_OK         0
#define RES_SCAN_ERROR 1
#define RES_XFILE      2
#define RES_ERROR      3

static bool         scan_mbox(char *fname, int msgNb, void *arg);

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static name2id_T    labels[] = {
  {"Content-Type", CT_TYPE},
  {"Content-Disposition", CT_DISP},
  {"UUencoded file", CT_UUFILE},
  {NULL, -1}
};

int
main(int argc, char **argv)
{
  char               *fname = "VIRUS_SIRCAM";
  FILE               *fin;

  const char         *args = "hvc:";
  int                 c;
  int                 io;
  int                 main_result = RES_OK;

  openlog("j-scanfile", LOG_PID | LOG_NOWAIT | LOG_NDELAY, LOG_LOCAL5);
  if (argc > 1)
    fname = argv[1];

  set_log_output(FALSE, TRUE);

  init_default_file_extensions();

  cf_opt.arg_v = 0;

  while ((c = getopt(argc, argv, args)) != -1)
  {
    switch (c)
    {
      case 'h':                /* OK */
        cf_opt.arg_h = TRUE;
        usage();
        exit(RES_OK);
        break;
      case 'v':
        cf_opt.arg_v++;
        break;
        /*  */
      case 'c':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Erreur %s\n", optarg);
          exit(RES_ERROR);
        }
        if (cf_opt.arg_c != NULL)
        {
          MESSAGE_INFO(0, "Only one c option, please");
          exit(RES_ERROR);
        }
        if ((cf_opt.arg_c = strdup(optarg)) == NULL)
        {
          LOG_SYS_ERROR("FATAL ERROR - memory allocation cf_opt.arg_c");
          exit(RES_ERROR);
        }
        conf_file = cf_opt.arg_c;
        break;
    }
  }

  io = optind;

  while (io < argc && *argv[io] == '-')
    io++;

  configure("j-scanfile", conf_file, FALSE);

  {
    int                 nbf = 0;

    while (io < argc)
    {
      fname = argv[io++];
      printf(" HANDLING %s\n", fname);
      nbf += mbox_handle(fname, scan_mbox, NULL);
    }
  }

  return main_result;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static              bool
scan_mbox(fname, msgNb, arg)
     char               *fname;
     int                 msgNb;
     void               *arg;
{
  FILE               *fin = NULL;

  char                chunk[0x10001];
  char                old[10000];

  int                 state = 0;
  int                 nb;

  int                 result = RES_OK;
  content_field_T     content;
  content_field_T    *list, *p;

  char               *ip = "0.0.0.0";
  char               *id = "00000000.000";
  char                bid[32];

  size_t              fsize = 0;

  SHOW_CURSOR(FALSE);

  if ((fin = fopen(fname, "r")) == NULL)
    return RES_SCAN_ERROR;

  fsize = get_file_size(fname);

  snprintf(bid, sizeof (bid), "%08X.000", msgNb);
  id = bid;

  memset(&content, 0, sizeof (content));
  list = NULL;

  memset(chunk, 0, sizeof (chunk));
  memset(old, 0, sizeof (old));
  while ((nb = fread(chunk, 1, sizeof (chunk) - 1, fin)) > 0)
  {
    int                 res;

    res =
      scan_block(NULL, old, sizeof (old) - 1, chunk, nb, &state, &content,
                 &list);

    if (res != 0)
    {
      printf(" scan_block res = %d\n", res);
#if 1
      goto fin;
#else
      exit(RES_SCAN_ERROR);
#endif
    }
  }

  if (cf_opt.arg_v > 1)
  {
    p = list;

    while (p != NULL)
    {
      int                 i;
      char               *label[] = { "", "Content-Type",
        "Content-Disposition", "UUencoded file"
      };

      printf("%s *** FIELD     : %s\n", id, label[p->field_type]);
      printf("%s     VALEUR    : %s\n", id, p->value ? p->value : "");
      for (i = 0; i < NB_ATTR; i++)
      {
        if (p->attr[i].name)
        {
          printf("%s     ATTR[%2d]  : %s\n", id, i, p->attr[i].name);
          if (p->attr[i].value)
            printf("%s     DATA[%2d]  : %s\n", id, i, p->attr[i].value);
        }
      }
      p = p->next;
    }
  }

  {
    int                 i = 0, nb = 0;
    attachment_T       *ahead = NULL, *p;

    extract_attachments(list, &ahead);
    p = ahead;

    while (p)
    {
      char               *svirus;

      nb++;
      if (!p->xfile)
        p->xfile = check_xfiles(p->name, p->mimetype, 0, NULL, 0);

      svirus = STRBOOL(p->xfile, "XFILE", ".....");
      if (p->xfile)
        i++;

      if (cf_opt.arg_v > 0 || p->xfile)
      {
        printf("%s ATTACHED FILE (%7d) (%-5s) : %-10s %-30s %-15s\n",
               id, fsize, svirus,
               STREMPTY(p->disposition, "..."),
               STREMPTY(p->mimetype, "..."), p->name);
      }
      p = p->next;

    }
    if (i > 0)
      result = RES_XFILE;
    if (cf_opt.arg_v > 1)
      printf("%s %4d FILES - %4d XFILES\n\n", id, nb, i++);
  }
  free_content_field_list(list);
  list = NULL;

fin:
  fclose(fin);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
usage()
{
  printf("Usage : j-scanfile [-h] [-c] [-v] file file file...\n"
         "  %s\n"
         "  Compiled on %s %s\n"
         "        -h : help\n"
         "        -c : configuration file\n"
         "        -v : verbose\n"
         "\n%s - Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz - (C) 2002\n\n",
         PACKAGE, __DATE__, __TIME__, PACKAGE);
}
