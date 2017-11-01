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

#include <ze-btree.h>

#include "ze-libjc.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define   _DB_LH   -1
#define   _DB_EH    0
#define   _DB_RH    1

struct JBTREC_T
{
  JBTREC_T           *left;
  JBTREC_T           *right;
  int                 balance;
  void               *data;
};

static bool         jbt_destroy_tree(JBTREC_T *);

static bool         jbt_add_node(JBT_T *, void *, JBTREC_T **, bool *);

static JBTREC_T    *jbt_del_node(JBT_T *, void *, JBTREC_T *, bool *);

static JBTREC_T    *jbt_get_node(JBT_T *, void *, JBTREC_T *);

static int          jbt_browse_tree(JBT_T *, JBT_BROWSE_F, JBTREC_T *, void *);
static bool         jbt_cpy_tree(JBT_T *, JBTREC_T *, JBT_SEL_F, void *);

static JBTREC_T    *jbt_node_alloc(JBT_T *, void *);

static JBTREC_T    *jbt_rotate_left(JBTREC_T *);
static JBTREC_T    *jbt_rotate_right(JBTREC_T *);

static JBTREC_T    *jbt_left_balance(JBTREC_T *);
static JBTREC_T    *jbt_right_balance(JBTREC_T *);

static int          jlog2(int);

int                 jbt_max_height(JBT_T *);

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
jbt_init(jdbh, size, reccmp)
     JBT_T              *jdbh;
     size_t              size;
     JBT_CMP_F           reccmp;
{
  if (jdbh == NULL)
    return FALSE;

  if (size <= 0)
    return FALSE;

  if (reccmp == NULL)
    return FALSE;

  if (jdbh->signature == SIGNATURE)
    jbt_destroy(jdbh);
  else
    memset(jdbh, 0, sizeof (JBT_T));

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
jbt_lock(jdbh)
     JBT_T              *jdbh;
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
jbt_unlock(jdbh)
     JBT_T              *jdbh;
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
jbt_set_btree_size(jdbh, chkCount, maxCount)
     JBT_T              *jdbh;
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
jbt_destroy(jdbh)
     JBT_T              *jdbh;
{

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  (void) jbt_destroy_tree(jdbh->root);

  jdbh->size = 0;
  jdbh->count = 0;
  jdbh->reccmp = NULL;
  jdbh->root = NULL;

  memset(jdbh, 0, sizeof (JBT_T));

  jdbh->chkCount = FALSE;
  jdbh->maxCount = MAX_BTNODES;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
jbt_clear(jdbh)
     JBT_T              *jdbh;
{

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  (void) jbt_destroy_tree(jdbh->root);

  jdbh->count = 0;
  jdbh->root = NULL;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
jbt_count(jdbh)
     JBT_T              *jdbh;
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
jbt_browse(jdbh, func, data)
     JBT_T              *jdbh;
     JBT_BROWSE_F        func;
     void               *data;
{
  if (jdbh == NULL)
    return 0;

  if (jdbh->signature != SIGNATURE)
    return 0;

  return jbt_browse_tree(jdbh, func, jdbh->root, data);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
jbt_get(jdbh, data)
     JBT_T              *jdbh;
     void               *data;
{
  JBTREC_T           *node;

  if (jdbh == NULL)
    return NULL;
  if (data == NULL)
    return NULL;

  if (jdbh->signature != SIGNATURE)
    return NULL;

  node = jbt_get_node(jdbh, data, jdbh->root);
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
jbt_add(jdbh, data)
     JBT_T              *jdbh;
     void               *data;
{
  bool                ok = FALSE;

  if (jdbh == NULL)
    return FALSE;
  if (data == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

#if BUGGY_AVLTREE
  return TRUE;
#endif

  if (jdbh->chkCount && (jdbh->count >= MAX_BTNODES))
  {
    if (jdbh->nbErr < MAX_BTERR)
      LOG_MSG_ERROR("Too much nodes in btree : %ld", (long) jdbh->count);
    jdbh->nbErr++;
    return FALSE;
  }
  jdbh->nbErr = 0;

  MESSAGE_INFO(19, " Adding...");

  {
    bool                taller = FALSE;

    ok = jbt_add_node(jdbh, data, &(jdbh->root), &taller);
    if (!ok)
      LOG_MSG_NOTICE("jbt_add_node returned NULL...");
  }

  MESSAGE_INFO(19, " OK ... %d", jdbh->count);

  return ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
jbt_del(jdbh, data)
     JBT_T              *jdbh;
     void               *data;
{
  JBTREC_T           *root = NULL;
  bool                shorter = FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  root = jbt_del_node(jdbh, data, jdbh->root, &shorter);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
jbt_destroy_tree(root)
     JBTREC_T           *root;
{
  if (root == NULL)
    return TRUE;

  jbt_destroy_tree(root->left);
  root->left = NULL;

  FREE(root->data);

  jbt_destroy_tree(root->right);
  root->right = NULL;
  FREE(root);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 * TO DO ***                                                                  *
 **************************************************************************** */
static JBTREC_T    *
jbt_del_node(jdbh, data, root, shorter)
     JBT_T              *jdbh;
     void               *data;
     JBTREC_T           *root;
     bool               *shorter;
{
  int                 cmp;
  JBTREC_T           *res = root;

  if (jdbh == NULL)
  {
    /* JOE - ERROR */
    return NULL;
  }
  if (data == NULL)
  {
    /* JOE - ERROR */
    return NULL;
  }

  if (root == NULL)
  {
    /* XXX */
    return root;
  }

  if ((cmp = (*jdbh->reccmp) (data, root->data)) == 0)
  {
    /* XXX */
    return NULL;
  }
  if (cmp < 0)
  {
  }
  if (cmp > 0)
  {
  }
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
JBTREC_T           *
jbt_get_node(jdbh, data, root)
     JBT_T              *jdbh;
     void               *data;
     JBTREC_T           *root;
{
  int res = 0;

  if (jdbh == NULL)
    return NULL;
  if (data == NULL)
    return NULL;
  if (root == NULL)
    return NULL;

  if (root->data == NULL)
  {
    LOG_MSG_WARNING("root not NULL but root->data is NULL...");
    return NULL;
  }

  res = jdbh->reccmp(data, root->data);

  if (res == 0)
    return root;
  if (res < 0)
    return jbt_get_node(jdbh, data, root->left);
  if (res > 0)
    return jbt_get_node(jdbh, data, root->right);

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
jbt_browse_tree(jdbh, func, root, data)
     JBT_T              *jdbh;
     JBT_BROWSE_F        func;
     JBTREC_T           *root;
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
    n += jbt_browse_tree(jdbh, func, root->left, data);

  if (func != NULL)
    n += func(root->data, data);

  if (root->right != NULL)
    n += jbt_browse_tree(jdbh, func, root->right, data);

  return n;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
jbt_cpy(dst, org, getit, arg)
     JBT_T              *dst;
     JBT_T              *org;
     JBT_SEL_F           getit;
     void               *arg;
{
  if (dst == NULL || org == NULL)
    return FALSE;

  if (dst->signature != SIGNATURE || org->signature != SIGNATURE)
    return FALSE;

  return jbt_cpy_tree(dst, org->root, getit, arg);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
jbt_cpy_tree(jdbh, root, getit, arg)
     JBT_T              *jdbh;
     JBTREC_T           *root;
     JBT_SEL_F           getit;
     void               *arg;
{
  if (root == NULL)
    return FALSE;

  (void) jbt_cpy_tree(jdbh, root->left, getit, arg);
  if (root->data != NULL)
  {
    if (getit(root->data, arg))
    {
      if (!jbt_add(jdbh, root->data))
	;
    }
  } else;

  (void) jbt_cpy_tree(jdbh, root->right, getit, arg);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
jbt_cleanup(jdbh, getit, arg)
     JBT_T              *jdbh;
     JBT_SEL_F           getit;
     void               *arg;
{
  JBTREC_T           *oroot = NULL;

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  oroot = jdbh->root;
  jdbh->root = NULL;
  jdbh->count = 0;

  if (jbt_cpy_tree(jdbh, oroot, getit, arg))
  {
    jbt_destroy_tree(oroot);
  }

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static JBTREC_T    *
jbt_rotate_left(root)
     JBTREC_T           *root;
{
  JBTREC_T           *tmp;

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
static JBTREC_T    *
jbt_rotate_right(root)
     JBTREC_T           *root;
{
  JBTREC_T           *tmp;

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
static JBTREC_T    *
jbt_right_balance(root)
     JBTREC_T           *root;
{
  JBTREC_T           *x, *w;

  x = root->right;

  switch (x->balance)
  {
    case _DB_RH:
      root->balance = _DB_EH;
      x->balance = _DB_EH;
      root = jbt_rotate_left(root);
      break;
    case _DB_EH:
      MESSAGE_WARNING(10, "jbt_right_balance x->balance == _DB_EH ???");
      break;
    case _DB_LH:
      w = x->left;
      switch (w->balance)
      {
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
      x = jbt_rotate_right(x);
      root->right = x;
      root = jbt_rotate_left(root);
      break;
  }

  return root;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static JBTREC_T    *
jbt_left_balance(root)
     JBTREC_T           *root;
{
  JBTREC_T           *x, *w;

  x = root->left;

  switch (x->balance)
  {
    case _DB_LH:
      root->balance = _DB_EH;
      x->balance = _DB_EH;
      root = jbt_rotate_right(root);
      break;
    case _DB_EH:
      MESSAGE_WARNING(10, "jbt_left_balance x->balance == _DB_EH ???");
      break;
    case _DB_RH:
      w = x->right;
      switch (w->balance)
      {
        case _DB_EH:
          root->balance = _DB_EH;
          x->balance = _DB_EH;
          break;
        case _DB_RH:
          root->balance = _DB_EH;
          /* R -> L */
          x->balance = _DB_LH;
          break;
        case _DB_LH:
          root->balance = _DB_RH;
          x->balance = _DB_EH;
          break;
      }
      w->balance = _DB_EH;
      x = jbt_rotate_left(x);
      root->left = x;
      root = jbt_rotate_right(root);
      break;
  }

  return root;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static JBTREC_T    *
jbt_node_alloc(jdbh, data)
     JBT_T              *jdbh;
     void               *data;
{
  JBTREC_T           *rec;

  rec = (JBTREC_T *) malloc(sizeof (JBTREC_T));
  if (rec == NULL)
  {
    LOG_SYS_ERROR("malloc jbtrec");
    return NULL;
  }

  memset(rec, 0, sizeof (JBTREC_T));

  rec->data = malloc(jdbh->size);
  if (rec->data == NULL)
  {
    LOG_SYS_ERROR("malloc data");
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
jbt_add_node(jdbh, data, root, taller)
     JBT_T              *jdbh;
     void               *data;
     JBTREC_T          **root;
     bool               *taller;
{
  if (root == NULL)
    return FALSE;

  if (*root == NULL)
  {
    JBTREC_T           *node = NULL;

    MESSAGE_INFO(19, "Empty tree ...");
    node = jbt_node_alloc(jdbh, data);
    if (node != NULL)
    {
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
    JBTREC_T           *troot = NULL;

    bool                ok = TRUE;

    troot = *root;

    r = (*jdbh->reccmp) (data, troot->data);

    if (r == 0)
    {
      LOG_MSG_NOTICE("Node already on the tree...");
      return FALSE;
    }

    if (r < 0)
    {
      ok = jbt_add_node(jdbh, data, &(troot->left), taller);

      if (ok && (taller != NULL) && *taller)
      {
        switch (troot->balance)
        {
          case _DB_LH:
            {
              JBTREC_T           *node = NULL;

              node = jbt_left_balance(troot);
              *taller = FALSE;
              if (node == NULL)
              {
                LOG_MSG_WARNING("jbt_left_balance returned NULL...");
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
            /* error */
            break;
        }
      }
      *root = troot;
      return ok;
    }

    if (r > 0)
    {
      ok = jbt_add_node(jdbh, data, &(troot->right), taller);

      if (ok && (taller != NULL) && *taller)
      {
        switch (troot->balance)
        {
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
              JBTREC_T           *node = NULL;

              node = jbt_right_balance(troot);
              *taller = FALSE;
              if (node == NULL)
              {
                LOG_MSG_WARNING("jbt_right_balance returned NULL...");
                return FALSE;
              }
              troot = node;
            }
            break;
          default:
            /* error */
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
jbt_max_height(jdbh)
     JBT_T              *jdbh;
{

  if (jdbh == NULL)
    return FALSE;

  if (jdbh->signature != SIGNATURE)
    return FALSE;

  return 2 * jlog2(jdbh->count);
}
