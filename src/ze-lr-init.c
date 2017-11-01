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
 *  Creation     : Mon Dec 28 11:16:21 CET 2009
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


#include <ze-sys.h>
#include <ze-filter.h>
#include <ze-lr-init.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

bool   lr_filter_ok = FALSE;

static void        *lr_initialize(void *);

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

bool
lr_data_load(background)
     bool background;
{
  if (background)
  {
    pthread_t           tid;
    int                 r;

    if ((r = pthread_create(&tid, NULL, lr_initialize, (void *) TRUE)) != 0) {
      LOG_SYS_ERROR("Error launching lr_initialize_task");
      return FALSE;
    }
  } else
    lr_initialize(FALSE);

  return TRUE;
}



/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void        *
lr_initialize(arg)
     void               *arg;
{
  char                path[1024];
  char               *lrname = "j-lr.txt";
  char               *cdb_dir = NULL;

  {
    bool              is_thread = (bool) arg;

    if (is_thread)
    {
      pthread_t           tid;

      tid = pthread_self();
      (void) pthread_detach(tid);
    }
  }

  cdb_dir = cf_get_str(CF_CDBDIR);
  cdb_dir = STREMPTY(cdb_dir, J_CDBDIR);
  memset(path, 0, sizeof (path));

  ADJUST_FILENAME(path, lrname, cdb_dir, "j-lr.txt");

  lr_filter_ok = FALSE;
  MESSAGE_INFO(9, "Opening perceptron data file : %s", path);
  (void) lr_data_close();
  if (lr_data_open(path))
  {
    MESSAGE_INFO(9, "Perceptron data file : %s : OK !", path);
    lr_filter_ok = TRUE;
  } else
    MESSAGE_WARNING(9, "Couldn't open perceptron data file : %s", path);

  return NULL;
}


