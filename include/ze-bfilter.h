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
 *  Creation     : Thu Jun 15 13:41:01 CEST 2006
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


#ifndef J_BFILTER_H

#define BFLAG_TRFTOK     2

#define BFLAG_DEFAULT    (BFLAG_TRFTOK)

#define BFHISTODIM     21

typedef struct
{
  uint32_t            signature;
  bool                ok;

  pthread_mutex_t     mutex;
  JDB_T               bdb;
  char               *dbname;
  size_t              cachesize;

  long                nbMsgsSpam;
  long                nbMsgsHam;

  double              kms;
  double              kmh;

  long                nbTokensSpam;
  long                nbTokensHam;

  double              kts;
  double              kth;

  long                nbFeaturesSpam;
  long                nbFeaturesHam;

  double              kfs;
  double              kfh;

  long                nbFeatures;

  double              rhs;
  double              ut_prob;

  int                 nbt;

  int                 crypt;

  uint32_t            flags;

  size_t              maxMsgSize;
  size_t              maxPartSize;

  bool                logit;

  bool                segRecurse;
  bool                segDouble;

  int                 clType;
  int                 clFSel;

  int                 histo[BFHISTODIM];
} bfilter_T;


#define  CL_TYPE_NP                             0
#define  CL_TYPE_NB                             1

#define  CL_FS_EP                               0
#define  CL_FS_MI                               1
#define  CF_FS_TFIDF                            2


#define BFILTER_INITIALIZER			\
  {						\
    SIGNATURE,					\
      FALSE,					\
      PTHREAD_MUTEX_INITIALIZER,		\
      JDB_INITIALIZER,				\
      NULL, 0,					\
      0, 0, 0., 0.,				\
      0, 0, 0., 0.,				\
      0, 0, 0., 0., 0,				\
      1.0,					\
      UT_PROB,					\
      64,					\
      HASH_PLAIN,				\
      BFLAG_DEFAULT,				\
      400000,					\
      40000,					\
      FALSE,					\
      TRUE,					\
      TRUE,					\
      CL_TYPE_NP,				\
      CL_FS_EP,					\
      0						\
  }

bfilter_T          *bfilter_ptr();

bool                bfilter_init(char *dbname);

bool                bfilter_db_open();

bool                bfilter_db_reopen();

bool                bfilter_close();

bool                bfilter_ok();

uint32_t            set_bfilter_flags(uint32_t flags);

bool                set_bfilter_logit(bool enable);

bool                set_bfilter_ham_spam_ratio(double ratio);
bool                set_bfilter_unknown_token_prob(double prob);
bool                set_bfilter_nb_tokens(int nbt);
bool                set_bfilter_max_sizes(size_t msg, size_t mime);
bool                set_bfilter_db_crypt(int crypt);

uint32_t            get_bfilter_flags();
double              get_bfilter_ham_spam_ratio();
double              get_bfilter_unknown_token_prob();
int                 get_bfilter_nb_tokens();
size_t              get_bfilter_max_sizes();
int                 get_bfilter_db_crypt();

typedef struct sfilter_token_T
{
  char                token[128];
  int                 nb;
  double              prob;

  double              nts;
  double              nth;

  /* XXX int version needed by learning phase */
  int                 nbs;
  int                 nbh;

  double              value;

  bool                ok;
} sfilter_token_T;

typedef int         (*btsm_browse_F) (void *, void *);

typedef int         (*smodel_db_browse_F) (void *, void *, void *);

bool                smodel_db_check_token(char *key, sfilter_token_T * token);
void                smodel_db_info(char *prefix, smodel_db_browse_F func,
                                   void *arg);

void                set_tokconf_active(char *tag, bool active);

bool                bfilter_handle_message(char *id, char *fname,
                                           btsm_browse_F func, void *arg);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define UT_PROB    0.5


# define J_BFILTER_H    1
#endif             /* J_BFILTER_H */
