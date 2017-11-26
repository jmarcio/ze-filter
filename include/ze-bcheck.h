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
 *  Creation     : Mon Jun 19 17:24:56 CEST 2006
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


#ifndef ZE_BCHECK_H

#define MAX_TOK          128

typedef struct sfilter_vsm_T
{
  sfilter_token_T     tok[MAX_TOK];
  int                 nbt;
  int                 nb;
} sfilter_vsm_T;

#define SFILTER_VSM_INIT(bc,nbt,prob)		\
  do {						\
    int i;					\
    memset(bc, 0, sizeof(sfilter_vsm_T));	\
    for (i = 0; i < MAX_TOK; i++)		\
      bc->tok[i].prob = prob;			\
    bc->nbt = nbt;				\
    bc->nb = 0;					\
  } while (0)



typedef struct sfilter_cli_T
{
  size_t              maxSize;  /* max message size */
  bool                check;    /* general - option */
  bool                verbose;  /* general - option */
  bool                progress; /* general - option */
  char               *dbname;   /* database - general */

  char               *id;       /* message ID -> learn and check */

  bool                spam;     /* spam / ham - learn */
  char               *timestr;  /* date -> learn */

  sfilter_vsm_T       bcheck;   /* pertinent tokens - check */
  int                 histo[21];  /* histogram  - check */
  bool                histogram;  /* do histogram ? - check */
  int                 nbt;      /* number of pertinent tokens - check */
  double              uprob;    /* unknown tokens probability */
} sfilter_cli_T;

bool                sfilter_cli_handle_message(char *, int, void *);
double              sfilter_check_message(char *id, char *fname,
                                          sfilter_vsm_T * bcheck);


# define ZE_BCHECK_H    1
#endif             /* J_BCHECK_H */
