
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

#include "ze-filter.h"



static void         usage();

static ZEDB_T       zdbh = ZEDB_INITIALIZER;

#define             MDB_NONE      0
#define             MDB_UPDATE    1
#define             MDB_SKIP      2
#define             MDB_ERASE     4

static int          db_updt_mode = MDB_NONE;
static int          add_db_rec(void *, void *);

static long         nb_ok, nb_ko, nb_t;

static bool         lk = TRUE, lv = TRUE;


int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  const char         *args = "h12b:rvdfk:t:m:cw:eC:X";
  int                 c;
  int                 verbose = 0;
  char               *dbname = NULL;
  int                 dbattrnb = 2;
  bool                dbreverse = FALSE;
  bool                dbdump = FALSE;
  bool                dbtext = FALSE;
  bool                dbcount = FALSE;

  bool                dbtype = TRUE;

  char               *dbstartkey = NULL;

  int                 kw = 40;
  bool                dbexport = TRUE;

#ifndef USE_BerkeleyDB
  fprintf(stderr, " -> ze-filter wasn't compiled with BerkeleyDB...\n");
  fprintf(stderr, " ->   Can't use dabatases without BerkeleyDB !\n");
  exit(1);
#endif

  zeLog_SetOutput(FALSE, TRUE);

  while ((c = getopt(argc, argv, args)) != -1) {
    switch (c) {
      case 'f':
        dbtext = TRUE;
        break;
      case 'h':
        usage();
        exit(0);
        break;
      case '1':
        dbattrnb = 1;
        break;
      case '2':
        dbattrnb = 2;
        break;
      case 't':
        switch (tolower(*optarg)) {
          case 'b':
            dbtype = TRUE;
            break;
          case 'h':
            dbtype = FALSE;
            break;
          default:
            ZE_MessageInfo(0, "Error : optarg ...");
            usage();
            exit(1);
            break;
        }
        break;
      case 'w':
        {
          int                 w = atoi(optarg);

          if (w > 0)
            kw = w;
        }
        break;
      case 'e':
        dbexport = !dbexport;
        break;
      case 'm':
        switch (tolower(*optarg)) {
          case 'e':
            db_updt_mode |= MDB_ERASE;
            break;
          case 'u':
            db_updt_mode |= MDB_UPDATE;
            break;
          case 's':
            db_updt_mode |= MDB_SKIP;
            break;
          default:
            ZE_MessageInfo(0, "Error : optarg ...");
            usage();
            exit(1);
        }
        break;
      case 'b':
        if (optarg == NULL || *optarg == '\0') {
          fprintf(stderr, "Error %s\n", optarg ? optarg : "");
          exit(1);
        }
        if (dbname != NULL) {
          fprintf(stderr, "Error : only one -b option allowed\n");
          exit(1);
        }
        if ((dbname = strdup(optarg)) == NULL) {
          fprintf(stderr, "FATAL ERROR - memory allocation dbname\n");
          exit(1);
        }
        break;
      case 'c':
        dbcount = TRUE;
        dbdump = TRUE;
        break;
      case 'r':
        dbreverse = TRUE;
        break;
      case 'v':
        verbose = 1;
        break;
      case 'd':
        dbdump = TRUE;
        break;
      case 'k':
        if (optarg == NULL || *optarg == '\0') {
          fprintf(stderr, "Error %s\n", optarg ? optarg : "");
          exit(1);
        }
        FREE(dbstartkey);
        if ((dbstartkey = strdup(optarg)) == NULL) {
          fprintf(stderr, "FATAL ERROR - memory allocation dbname\n");
          exit(1);
        }
        break;
      case 'C':
        if (optarg != NULL) {
          char               *p;

          for (p = optarg; *p != '\0'; p++) {
            switch (*p) {
              case 'k':
                break;
              case 'K':
                break;
              case 'v':
                break;
              case 'V':
                break;
            }
          }
        }
        break;
      default:
        fprintf(stderr, "\nInvalid option\n");
        usage();
        exit(1);
    }
  }

  if (db_updt_mode == MDB_NONE)
    db_updt_mode = MDB_ERASE | MDB_SKIP;

  if ((db_updt_mode & MDB_UPDATE) != 0 && (db_updt_mode & MDB_SKIP) != 0) {
    fprintf(stderr, "\n  Can't set both SKIP and UPDATE modes\n");
    usage();
    exit(EX_SOFTWARE);
  }

  if ((db_updt_mode & (MDB_UPDATE | MDB_SKIP)) == 0)
    db_updt_mode |= MDB_SKIP;

  ze_logLevel = 8;
  dbstartkey = STRNULL(dbstartkey, "");

  nb_t = nb_ok = nb_ko = 0;

  if (dbtext) {
    zm_RdTextFile(NULL, dbattrnb, dbreverse, "", NULL);
    exit(0);
  }

  if (dbname == NULL) {
    usage();
    fprintf(stderr, "Missing -b option\n");
    exit(1);
  }

  if (*dbname == '-') {
    usage();
    fprintf(stderr, "dbname not valid : %s\n", dbname);
    exit(1);
  }


  /*
   *  ####    ####   #    #  #    #   #####
   * #    #  #    #  #    #  ##   #     #
   * #       #    #  #    #  # #  #     #
   * #       #    #  #    #  #  # #     #
   * #    #  #    #  #    #  #   ##     #
   *  ####    ####    ####   #    #     #
   */
  if (dbcount) {
    if (zeDb_Open(&zdbh, NULL, dbname, 0444, TRUE, dbtype, 0)) {
      long                nb = 0;

      ZEDB_STAT_T        *st;

      if (!zeDb_Stat(&zdbh, &st))
        exit(1);
      printf(" ** %7ld records found\n", (long int) st->st.btree_st.bt_ndata);
      FREE(st);
      zeDb_Close(&zdbh);
    }
    exit(0);
  }

  /*
   * #####   #    #  #    #  #####
   * #    #  #    #  ##  ##  #    #
   * #    #  #    #  # ## #  #    #
   * #    #  #    #  #    #  #####
   * #    #  #    #  #    #  #
   * #####    ####   #    #  #
   */
  if (dbdump) {
    if (zeDb_Open(&zdbh, NULL, dbname, 0444, TRUE, dbtype, 0)) {
      long                nb = 0;

      if (zeDb_CursorOpen(&zdbh, TRUE)) {
        char                key[256], data[256];
        int                 j;
        time_t              now;

        memset(key, 0, sizeof (key));
        memset(data, 0, sizeof (data));

        snprintf(key, sizeof (key), "%s", dbstartkey);

        now = time(NULL);

        snprintf(key, sizeof (key), "%s", dbstartkey);

        if (zeDb_CursorGetFirst(&zdbh, key, sizeof (key), data, sizeof (data))) {
          DB_BTREE_SEQ_START();
          do {
            char                format[64];

            DB_BTREE_SEQ_CHECK(key, zdbh.database);

            if (strncmp(key, dbstartkey, strlen(dbstartkey)) != 0)
              break;
            if (!dbcount) {
              if (!dbexport)
                snprintf(format, sizeof (format),
                         " ** KEY : %%-%ds - DATA : %%s\n", kw);
              else
                snprintf(format, sizeof (format), "%%-%ds %%s\n", kw);
              printf(format, key, data);
            }
            nb++;
          } while (zeDb_CursorGetNext
                   (&zdbh, key, sizeof (key), data, sizeof (data)));

          DB_BTREE_SEQ_END();
        }

        (void) zeDb_CursorClose(&zdbh);
      }
      zeDb_Close(&zdbh);

      if (!dbexport || dbcount)
        printf(" ** %7ld records found\n", nb);
    }

    exit(0);
  }

  /*
   * #    #  #####   #####     ##     #####  ######
   * #    #  #    #  #    #   #  #      #    #
   * #    #  #    #  #    #  #    #     #    #####
   * #    #  #####   #    #  ######     #    #
   * #    #  #       #    #  #    #     #    # 
   *  ####   #       #####   #    #     #    ######
   */
  {
    if (zeDb_Open(&zdbh, NULL, dbname, 0644, FALSE, dbtype, 0)) {

      if ((db_updt_mode & MDB_ERASE) != 0)
        (void) zeDb_Empty(&zdbh);

      (void) zm_RdTextFile(NULL, dbattrnb, dbreverse, "", add_db_rec);

      zeDb_Close(&zdbh);
    }
    printf("* Total : %6ld records read\n", nb_t);
    printf("          %6ld records added\n", nb_ok);
    printf("          %6ld errors\n", nb_ko);
  }

  return 0;
}


/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
#define BFSZ              1024

static int
add_db_rec(vk, vv)
     void               *vk;
     void               *vv;
{
  char               *k = (char *) vk;
  char               *v = (char *) vv;
  bool                res = TRUE;

  nb_t++;

  if (k == NULL || strlen(k) == 0)
    res = FALSE;

  if (res) {
    if (v == NULL)
      v = "";

    (void) zeStr2Lower(k);
    (void) zeStr2Lower(v);

    if ((db_updt_mode & MDB_SKIP) != 0) {
      char                buf[BFSZ];

      if (zeDb_GetRec(&zdbh, k, buf, sizeof (buf)))
        return 1;
    }

    res = zeDb_AddRec(&zdbh, k, v, strlen(v) + 1);
  }

  if (res)
    nb_ok++;
  else
    nb_ko++;

  return (res ? 1 : 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
usage()
{
  printf("Usage : ze-makemap options\n"
         "  %s\n"
         "  Compiled on %s %s\n" "        -b table : specifies table name\n"
         "        -d       : dump instead of update\n"
         "        -c       : count records on database\n"
         "        -k key   : matching key\n"
         "        -w NN    : column width when dumping database\n"
         "        -t       : database type\n"
         "                   btree\n"
         "                   hash\n"
         "        -m mode  : how to insert records\n"
         "                     e - clear database before update\n"
         "                     s - skip existent records\n"
         "                     u - new records update existent records\n"
         "                   - can be defined multiple times\n"
         "                   - default mode : erase and skim\n"
         "        -r       : reverse key/value order (value/key)\n"
         "        -1       : single column (key only)\n"
         "        -2       : two columns (key / value) - default\n"
         "        -h       : help (print this and exits)\n"
         "                   default mode : erase and skim\n"
         "\n%s - %s\n\n", PACKAGE, __DATE__, __TIME__, PACKAGE, COPYRIGHT);
}
