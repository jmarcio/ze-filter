/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Thu May 28 17:51:54 CEST 2009
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


#ifndef ZE_LR_FUNCS_H

#define               LR_CLASS_UNKNOWN   -1
#define               LR_CLASS_HAM        0
#define               LR_CLASS_SPAM       1

#define               LR_CMD_CLASS              0
#define               LR_CMD_LEARN              1
#define               LR_CMD_LEARN_RESAMPLE     2
#define               LR_CMD_LEARN_FEEDBACK     3
#define               LR_CMD_EXTRACT            4


#define               LR_TASK_CLASSIFY          1
#define               LR_TASK_LEARN             2
#define               LR_TASK_EXTRACT           4

typedef struct
{
  /* learn only */
  double              lrate;

  /* resample */
  bool                resample;

  /* learn or classify */
  bool                useRawMsg;
  size_t              rawLength;

  size_t              bodyLength;
  bool                useBody;
  bool                useHeaders;

  bool                cleanUpHeaders;
  bool                cleanUpDates;

  int                 tok_type; /* token type */
  int                 tok_len;  /* token len */

  /* active learning */
  bool                active_learning;
  double              active_margin;
} lr_opts_T;

#define LR_OPTS_INITIALIZER				\
  {							\
    0.004, FALSE,					\
      FALSE, 2500, 256, TRUE, TRUE, TRUE, TRUE,		\
      0, 4,						\
      FALSE, 0.25					\
      }

typedef struct
{
  char               *args;

  int                 nmsg;
  int                 nbml;     /* # of messages learnt */
  int                 nbmc;     /* # of messages classified */

  /* number of messages learnt per class */
  long                nSpam;
  long                nHam;

  /* number of features */
  long                nFeatures;

#if 1
  /* miss probability */
  double              pmiss;    /* feedback missing probability */
#endif
} lr_cargs_T;

typedef struct
{
  int                 cmd;
  char               *args;
  int                 class;
  /* */
  bool                resample;
  /* */
  bool                learnt;   /* this message was learnt */
  bool                query;    /* this message was queried */
#if 1
  bool                miss;     /* feedback missed */
#endif
  test_score_T        score;    /* message score record */
} lr_margs_T;


#if 0
typedef struct
{
  uint32_t            signature;
  char               *fname;
  bool                ok;
} lrh_T;

#define LRH_INITIALIZER {SIGNATURE, NULL, FALSE}
#endif

bool                lr_data_open(char *fname);
bool                lr_data_reopen();
bool                lr_data_close();
bool                lr_data_dump(char *fname);


bool                lr_classify(char *id, char *fname,
                                lr_cargs_T * cargs, lr_margs_T * margs,
                                test_score_T * score);

bool                lr_learn(char *id, char *fname,
                             lr_cargs_T * cargs, lr_margs_T * margs,
                             test_score_T * score, bool spam);

bool                lr_extract(char *id, char *fname,
			       lr_cargs_T * cargs, lr_margs_T * margs);

bool                lr_learn_options(bool active, double threshold);

bool                lr_set_options(lr_opts_T * opts);
bool                lr_get_options(lr_opts_T * opts);
void                lr_print_options(lr_opts_T * opts);


typedef double      (*lr_callback_F) (int, lr_cargs_T *, lr_margs_T *);
bool                lr_set_learn_callback(lr_callback_F);

# define ZE_LR_FUNCS_H    1
#endif             /* J_LR_FUNCS_H */
