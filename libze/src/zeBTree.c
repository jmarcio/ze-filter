
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

#include <zeBTree.h>

#include "libze.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define   _DB_LH   -1
#define   _DB_EH    0
#define   _DB_RH    1

struct ZEBTREC_T {
  ZEBTREC_T          *left;
  ZEBTREC_T          *right;
  int                 balance;
  void               *data;
};

static bool         zeBTree_Destroy_tree(ZEBTREC_T *);

static bool         zeBTree_Add_node(ZEBT_T *, void *, ZEBTREC_T **, bool *);

static ZEBTREC_T   *zeBTree_Del_node(ZEBT_T *, void *, ZEBTREC_T *, bool *);

static ZEBTREC_T   *zeBTree_Get_node(ZEBT_T *, void *, ZEBTREC_T *);

static int          zeBTree_Browse_tree(ZEBT_T *, ZEBT_BROWSE_F, ZEBTREC_T *,
                                        void *);
static bool         zeBTree_Cpy_tree(ZEBT_T *, ZEBTREC_T *, ZEBT_SEL_F, void *);

static ZEBTREC_T   *zeBTree_Node_Alloc(ZEBT_T *, void *);

static ZEBTREC_T   *zeBTree_Rotate_Left(ZEBTREC_T *);
static ZEBTREC_T   *zeBTree_Rotate_Right(ZEBTREC_T *);

static ZEBTREC_T   *zeBTree_Left_Balance(ZEBTREC_T *);
static ZEBTREC_T   *zeBTree_Right_Balance(ZEBTREC_T *);

static int          jlog2(int);

int                 zeBTree_Max_Height(ZEBT_T *);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define     JBT_LOCK(mutex)        MUTEX_LOCK(mutex)
#define     JBT_UNLOCK(mutex)      MUTEX_UNLOCK(mutex)

#define     JBT_ZERO(jbth)             \
  do {				       \
    jbth->signature = SIGNATURE;       \
    jbth->count = 0;		       \
    jbth->size = 0;		       \
    jbth->reccmp = NULL;	       \
    jbth->root = NULL;		       \
    jbth->chkCount = FALSE;	       \
    jbth->maxCount = MAX_BTNODES;      \
    jbth->nbErr = 0;		       \
  } while (0)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Init(jdbh, size, reccmp)
     ZEBT_T             *jdbh;
     size_t              size;
     ZEBT_CMP_F          reccmp;
{
  if (jdbh == NULL)
    return FALSE;

  if (size <= 0)
    return FALSE;

  if (reccmp == NULL)
    return FALSE;

  if (jdbh->signature == SIGNATURE)
    zeBTree_Destroy(jdbh);
  else
    memset(jdbh, 0, sizeof (ZEBT_T));

  jdbh->signature = SIGNATURE;
  jdbh->count = 0;
  jdbh->size = size;
  jdbh->reccmp = reccmp;
  jdbh->root = NULL;
  jdbh->chkCount = FALSE;
  jdbh->maxCount = MAX_BTNODES;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Lock(jdbh)
     ZEBT_T             *jdbh;
{
  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  MUTEX_LOCK(&jdbh->mutex);

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_unLock(jdbh)
     ZEBT_T             *jdbh;
{
  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  MUTEX_UNLOCK(&jdbh->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Set_BTree_Size(jdbh, chkCount, maxCount)
     ZEBT_T             *jdbh;
     bool                chkCount;
     int                 maxCount;
{
  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  jdbh->chkCount = chkCount;
  jdbh->maxCount = maxCount;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Destroy(jdbh)
     ZEBT_T             *jdbh;
{

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  (void) zeBTree_Destroy_tree(jdbh->root);

  jdbh->size = 0;
  jdbh->count = 0;
  jdbh->reccmp = NULL;
  jdbh->root = NULL;

  memset(jdbh, 0, sizeof (ZEBT_T));

  jdbh->chkCount = FALSE;
  jdbh->maxCount = MAX_BTNODES;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Clear(jdbh)
     ZEBT_T             *jdbh;
{

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  (void) zeBTree_Destroy_tree(jdbh->root);

  jdbh->count = 0;
  jdbh->root = NULL;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeBTree_Count(jdbh)
     ZEBT_T             *jdbh;
{
  if (jdbh == NULL)
    return 0;

  if (jdbh->signature != SIGNATURE)
    return 0;

  return jdbh->count;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeBTree_Browse(jdbh, func, data)
     ZEBT_T             *jdbh;
     ZEBT_BROWSE_F       func;
     void               *data;
{
  if (jdbh == NULL)
    return 0;

  if (jdbh->signature != SIGNATURE)
    return 0;

  return zeBTree_Browse_tree(jdbh, func, jdbh->root, data);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
zeBTree_Get(jdbh, data)
     ZEBT_T             *jdbh;
     void               *data;
{
  ZEBTREC_T          *node;

  if (jdbh == NULL)
    return NULL;
  if (data == NULL)
    return NULL;

  if (jdbh->signature != SIGNATURE)
    return NULL;

  node = zeBTree_Get_node(jdbh, data, jdbh->root);
  if (node != NULL)
    return node->data;

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define   MAX_BTERR     16

bool
zeBTree_Add(jdbh, data)
     ZEBT_T             *jdbh;
     void               *data;
{
  bool                ok = FALSE;

  if (jdbh == NULL)
    return FALSE;
  if (data == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  if (jdbh->chkCount && (jdbh->count >= MAX_BTNODES)) {
    if (jdbh->nbErr < MAX_BTERR)
      ZE_LogMsgError(0, "Too much nodes in btree : %ld", (long) jdbh->count);
    jdbh->nbErr++;
    return FALSE;
  }
  jdbh->nbErr = 0;

  ZE_MessageInfo(19, " Adding...");

  {
    bool                taller = FALSE;

    ok = zeBTree_Add_node(jdbh, data, &(jdbh->root), &taller);
    if (!ok)
      ZE_LogMsgNotice(0, "zeBTree_Add_node returned NULL...");
  }

  ZE_MessageInfo(19, " OK ... %d", jdbh->count);

  return ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Del(jdbh, data)
     ZEBT_T             *jdbh;
     void               *data;
{
  ZEBTREC_T          *root = NULL;
  bool                shorter = FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  root = zeBTree_Del_node(jdbh, data, jdbh->root, &shorter);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
zeBTree_Destroy_tree(root)
     ZEBTREC_T          *root;
{
  if (root == NULL)
    return TRUE;

  zeBTree_Destroy_tree(root->left);
  root->left = NULL;

  FREE(root->data);

  zeBTree_Destroy_tree(root->right);
  root->right = NULL;
  FREE(root);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 * TO DO ***                                                                  *
 **************************************************************************** */
static ZEBTREC_T   *
zeBTree_Del_node(jdbh, data, root, shorter)
     ZEBT_T             *jdbh;
     void               *data;
     ZEBTREC_T          *root;
     bool               *shorter;
{
  int                 cmp;
  ZEBTREC_T          *res = root;

  if (jdbh == NULL) {
    /*
     * JOE - ERROR 
     */
    return NULL;
  }
  if (data == NULL) {
    /*
     * JOE - ERROR 
     */
    return NULL;
  }

  if (root == NULL) {
    /*
     * XXX 
     */
    return root;
  }

  if ((cmp = (*jdbh->reccmp) (data, root->data)) == 0) {
    /*
     * XXX 
     */
    return NULL;
  }
  if (cmp < 0) {
  }
  if (cmp > 0) {
  }
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
ZEBTREC_T          *
zeBTree_Get_node(jdbh, data, root)
     ZEBT_T             *jdbh;
     void               *data;
     ZEBTREC_T          *root;
{
  int                 res = 0;

  if (jdbh == NULL)
    return NULL;
  if (data == NULL)
    return NULL;
  if (root == NULL)
    return NULL;

  if (root->data == NULL) {
    ZE_LogMsgWarning(0, "root not NULL but root->data is NULL...");
    return NULL;
  }

  res = jdbh->reccmp(data, root->data);

  if (res == 0)
    return root;
  if (res < 0)
    return zeBTree_Get_node(jdbh, data, root->left);
  if (res > 0)
    return zeBTree_Get_node(jdbh, data, root->right);

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeBTree_Browse_tree(jdbh, func, root, data)
     ZEBT_T             *jdbh;
     ZEBT_BROWSE_F       func;
     ZEBTREC_T          *root;
     void               *data;
{
  int                 n = 0;

  if (jdbh == NULL)
    return 0;

  if (root == NULL)
    return 0;

  if (jdbh->signature != SIGNATURE)
    return 0;

  if (root->left != NULL)
    n += zeBTree_Browse_tree(jdbh, func, root->left, data);

  if (func != NULL)
    n += func(root->data, data);

  if (root->right != NULL)
    n += zeBTree_Browse_tree(jdbh, func, root->right, data);

  return n;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Cpy(dst, org, getit, arg)
     ZEBT_T             *dst;
     ZEBT_T             *org;
     ZEBT_SEL_F          getit;
     void               *arg;
{
  if (dst == NULL || org == NULL)
    return FALSE;

  if (dst->signature != SIGNATURE || org->signature != SIGNATURE)
    return FALSE;

  return zeBTree_Cpy_tree(dst, org->root, getit, arg);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
zeBTree_Cpy_tree(jdbh, root, getit, arg)
     ZEBT_T             *jdbh;
     ZEBTREC_T          *root;
     ZEBT_SEL_F          getit;
     void               *arg;
{
  if (root == NULL)
    return FALSE;

  (void) zeBTree_Cpy_tree(jdbh, root->left, getit, arg);
  if (root->data != NULL) {
    if (getit(root->data, arg)) {
      if (!zeBTree_Add(jdbh, root->data));
    }
  } else;

  (void) zeBTree_Cpy_tree(jdbh, root->right, getit, arg);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
zeBTree_Cleanup(jdbh, getit, arg)
     ZEBT_T             *jdbh;
     ZEBT_SEL_F          getit;
     void               *arg;
{
  ZEBTREC_T          *oroot = NULL;

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  oroot = jdbh->root;
  jdbh->root = NULL;
  jdbh->count = 0;

  if (zeBTree_Cpy_tree(jdbh, oroot, getit, arg)) {
    zeBTree_Destroy_tree(oroot);
  }

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static ZEBTREC_T   *
zeBTree_Rotate_Left(root)
     ZEBTREC_T          *root;
{
  ZEBTREC_T          *tmp;

  if (root == NULL)
    return NULL;

  if (root->right == NULL)
    return root;

  tmp = root->right;
  root->right = tmp->left;
  tmp->left = root;
  root = tmp;

  return root;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static ZEBTREC_T   *
zeBTree_Rotate_Right(root)
     ZEBTREC_T          *root;
{
  ZEBTREC_T          *tmp;

  if (root == NULL)
    return NULL;

  if (root->left == NULL)
    return root;

  tmp = root->left;
  root->left = tmp->right;
  tmp->right = root;
  root = tmp;

  return root;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static ZEBTREC_T   *
zeBTree_Right_Balance(root)
     ZEBTREC_T          *root;
{
  ZEBTREC_T          *x, *w;

  x = root->right;

  switch (x->balance) {
    case _DB_RH:
      root->balance = _DB_EH;
      x->balance = _DB_EH;
      root = zeBTree_Rotate_Left(root);
      break;
    case _DB_EH:
      ZE_MessageWarning(10, "zeBTree_Right_Balance x->balance == _DB_EH ???");
      break;
    case _DB_LH:
      w = x->left;
      switch (w->balance) {
        case _DB_EH:
          root->balance = _DB_EH;
          x->balance = _DB_EH;
          break;
        case _DB_LH:
          root->balance = _DB_EH;
          x->balance = _DB_RH;
          break;
        case _DB_RH:
          root->balance = _DB_LH;
          x->balance = _DB_EH;
          break;
      }
      w->balance = _DB_EH;
      x = zeBTree_Rotate_Right(x);
      root->right = x;
      root = zeBTree_Rotate_Left(root);
      break;
  }

  return root;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static ZEBTREC_T   *
zeBTree_Left_Balance(root)
     ZEBTREC_T          *root;
{
  ZEBTREC_T          *x, *w;

  x = root->left;

  switch (x->balance) {
    case _DB_LH:
      root->balance = _DB_EH;
      x->balance = _DB_EH;
      root = zeBTree_Rotate_Right(root);
      break;
    case _DB_EH:
      ZE_MessageWarning(10, "zeBTree_Left_Balance x->balance == _DB_EH ???");
      break;
    case _DB_RH:
      w = x->right;
      switch (w->balance) {
        case _DB_EH:
          root->balance = _DB_EH;
          x->balance = _DB_EH;
          break;
        case _DB_RH:
          root->balance = _DB_EH;
          /*
           * R -> L 
           */
          x->balance = _DB_LH;
          break;
        case _DB_LH:
          root->balance = _DB_RH;
          x->balance = _DB_EH;
          break;
      }
      w->balance = _DB_EH;
      x = zeBTree_Rotate_Left(x);
      root->left = x;
      root = zeBTree_Rotate_Right(root);
      break;
  }

  return root;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static ZEBTREC_T   *
zeBTree_Node_Alloc(jdbh, data)
     ZEBT_T             *jdbh;
     void               *data;
{
  ZEBTREC_T          *rec;

  rec = (ZEBTREC_T *) malloc(sizeof (ZEBTREC_T));
  if (rec == NULL) {
    ZE_LogSysError("malloc jbtrec");
    return NULL;
  }

  memset(rec, 0, sizeof (ZEBTREC_T));

  rec->data = malloc(jdbh->size);
  if (rec->data == NULL) {
    ZE_LogSysError("malloc data");
    free(rec);
    return NULL;
  }
  rec->left = NULL;
  rec->right = NULL;
  rec->balance = _DB_EH;

  memcpy(rec->data, data, jdbh->size);
  return rec;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
zeBTree_Add_node(jdbh, data, root, taller)
     ZEBT_T             *jdbh;
     void               *data;
     ZEBTREC_T         **root;
     bool               *taller;
{
  if (root == NULL)
    return FALSE;

  if (*root == NULL) {
    ZEBTREC_T          *node = NULL;

    ZE_MessageInfo(19, "Empty tree ...");
    node = zeBTree_Node_Alloc(jdbh, data);
    if (node != NULL) {
      if (taller != NULL)
        *taller = TRUE;
      node->balance = _DB_EH;
      jdbh->count++;
      *root = node;
    }
    return (node != NULL);;
  }

  {
    int                 r;
    ZEBTREC_T          *troot = NULL;

    bool                ok = TRUE;

    troot = *root;

    r = (*jdbh->reccmp) (data, troot->data);

    if (r == 0) {
      ZE_LogMsgNotice(0, "Node already on the tree...");
      return FALSE;
    }

    if (r < 0) {
      ok = zeBTree_Add_node(jdbh, data, &(troot->left), taller);

      if (ok && (taller != NULL) && *taller) {
        switch (troot->balance) {
          case _DB_LH:
            {
              ZEBTREC_T          *node = NULL;

              node = zeBTree_Left_Balance(troot);
              *taller = FALSE;
              if (node == NULL) {
                ZE_LogMsgWarning(0, "zeBTree_Left_Balance returned NULL...");
                return FALSE;
              }
              troot = node;
            }
            break;
          case _DB_EH:
            troot->balance = _DB_LH;
            *taller = TRUE;
            break;
          case _DB_RH:
            troot->balance = _DB_EH;
            *taller = FALSE;
            break;
          default:
            /*
             * error 
             */
            break;
        }
      }
      *root = troot;
      return ok;
    }

    if (r > 0) {
      ok = zeBTree_Add_node(jdbh, data, &(troot->right), taller);

      if (ok && (taller != NULL) && *taller) {
        switch (troot->balance) {
          case _DB_LH:
            troot->balance = _DB_EH;
            *taller = FALSE;
            break;
          case _DB_EH:
            troot->balance = _DB_RH;
            *taller = TRUE;
            break;
          case _DB_RH:
            {
              ZEBTREC_T          *node = NULL;

              node = zeBTree_Right_Balance(troot);
              *taller = FALSE;
              if (node == NULL) {
                ZE_LogMsgWarning(0, "zeBTree_Right_Balance returned NULL...");
                return FALSE;
              }
              troot = node;
            }
            break;
          default:
            /*
             * error 
             */
            break;
        }

      }
      *root = troot;
      return ok;
    }
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
jlog2(x)
     int                 x;
{
  int                 n = 0;

  for (x /= 2; x != 0; x /= 2)
    n++;

  return n;
}

int
zeBTree_Max_Height(jdbh)
     ZEBT_T             *jdbh;
{

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  return 2 * jlog2(jdbh->count);
}
