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


#ifndef __JBTREE_H__

typedef struct JBT_T JBT_T;
typedef struct JBTREC_T JBTREC_T;

typedef int    (*JBT_CMP_F) (void *, void *);
typedef bool   (*JBT_SEL_F) (void *, void *);
typedef int    (*JBT_BROWSE_F) (void *, void *);

bool            jbt_lock (JBT_T *);
bool            jbt_unlock (JBT_T *);

bool            jbt_init (JBT_T *, size_t, JBT_CMP_F);

bool            jbt_set_btree_size(JBT_T *, bool, int);

bool            jbt_destroy (JBT_T *);

bool            jbt_clear (JBT_T *);

int             jbt_count(JBT_T *);

int             jbt_browse (JBT_T *, JBT_BROWSE_F, void *);

bool            jbt_cpy(JBT_T *, JBT_T *, JBT_SEL_F, void *);

bool            jbt_cleanup(JBT_T *, JBT_SEL_F, void *);

void           *jbt_get (JBT_T *, void *);

bool            jbt_add (JBT_T *, void *);

bool            jbt_del (JBT_T *, void *);


#ifndef MAX_BTNODES
# define MAX_BTNODES    0x800000
#endif

#ifndef NB_BTCLEANUP
# define NB_BTCLEANUP    ((4 * MAX_BTNODES) / 5)
#endif

struct JBT_T {
  uint32_t        signature;
  int             count;
  size_t          size;
  JBTREC_T       *root;
  JBT_CMP_F       reccmp;
  bool            chkCount;
  int             maxCount;
  int             nbErr;
  pthread_mutex_t mutex;
};

#define JBT_INITIALIZER                                 \
      {                                                 \
        SIGNATURE, 0, 0, NULL, NULL,                    \
        FALSE, MAX_BTNODES, 0, PTHREAD_MUTEX_INITIALIZER \
      }

#define __JBTREE_H__
#endif

