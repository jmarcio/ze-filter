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


#ifndef __ZE_BTREE_H

/** @addtogroup DataStruct
*
* @{
*/

typedef struct ZEBT_T ZEBT_T;
typedef struct ZEBTREC_T ZEBTREC_T;

typedef int    (*ZEBT_CMP_F) (void *, void *);
typedef bool   (*ZEBT_SEL_F) (void *, void *);
typedef int    (*ZEBT_BROWSE_F) (void *, void *);

bool            zeBTree_Lock (ZEBT_T *);
bool            zeBTree_unLock (ZEBT_T *);

bool            zeBTree_Init (ZEBT_T *, size_t, ZEBT_CMP_F);

bool            zeBTree_Set_BTree_Size(ZEBT_T *, bool, int);

bool            zeBTree_Destroy (ZEBT_T *);

bool            zeBTree_Clear (ZEBT_T *);

int             zeBTree_Count(ZEBT_T *);

int             zeBTree_Browse (ZEBT_T *, ZEBT_BROWSE_F, void *);

bool            zeBTree_Cpy(ZEBT_T *, ZEBT_T *, ZEBT_SEL_F, void *);

bool            zeBTree_Cleanup(ZEBT_T *, ZEBT_SEL_F, void *);

void           *zeBTree_Get (ZEBT_T *, void *);

bool            zeBTree_Add (ZEBT_T *, void *);

bool            zeBTree_Del (ZEBT_T *, void *);


#ifndef MAX_BTNODES
# define MAX_BTNODES    0x800000
#endif

#ifndef NB_BTCLEANUP
# define NB_BTCLEANUP    ((4 * MAX_BTNODES) / 5)
#endif

struct ZEBT_T {
  uint32_t        signature;
  int             count;
  size_t          size;
  ZEBTREC_T       *root;
  ZEBT_CMP_F       reccmp;
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

/** @} */

#define __ZE_BTREE_H
#endif

