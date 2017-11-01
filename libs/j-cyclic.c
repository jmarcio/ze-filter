/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Fri Apr 28 11:02:56 CEST 2006
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


#include <ze-sys.h>
#include <ze-chkmail.h>
#include <ze-cyclic.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define DIMTASKS     256

typedef struct
{
  uint32_t       signature;
  time_t         dt;

  CYCLIC_F       function;
  void          *arg;

  time_t         last;
  int            count;
  time_t         work;

  kstats_T       st;
} CTASK_T;

static CTASK_T tasks[DIMTASKS];
static bool    tsk_ok = FALSE;

static time_t  dt_loop = 10;
static time_t  t_start = 0;

static void   *cyclic_tasks(void *arg);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
cyclic_tasks_init(dt)
     time_t         dt;
{
  pthread_t      tid = (pthread_t) 0;

  if (!tsk_ok)
  {
    int            r;

    t_start = time(NULL);
    dt_loop = dt;
    memset(tasks, 0, sizeof (tasks));

    if ((r = pthread_create(&tid, NULL, cyclic_tasks, (void *) NULL)) != 0)
      LOG_SYS_ERROR("Couldn't launch cyclic_tasks");

    tsk_ok = TRUE;
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool
cyclic_tasks_register(task, arg, dt)
     CYCLIC_F       task;
     void          *arg;
     time_t         dt;
{
  int            i;

  ASSERT(task != NULL);
  ASSERT(dt > 0);

  for (i = 0; i < DIMTASKS; i++)
  {
    if (tasks[i].signature == SIGNATURE)
      continue;

    memset(&tasks[i], 0, sizeof(tasks[i]));

    if (tasks[i].function != NULL)
      continue;

    tasks[i].signature = SIGNATURE;
    tasks[i].function = task;
    tasks[i].arg = arg;
    tasks[i].dt = dt;
    tasks[i].last = time(NULL);

    return TRUE;
  }

  MESSAGE_WARNING(5, "No more room available for new cyclic tasks 8-( !");
  return FALSE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
cyclic_tasks_time_stats()
{
  int i;

  for (i = 0; i < DIMTASKS; i++)
  {
    int n, m;

    if (tasks[i].signature != SIGNATURE)
      continue;

    n = m = 0;
    n = tasks[i].count;
    if (tasks[i].count > 0)
      m = tasks[i].work / tasks[i].count;
    MESSAGE_INFO(10, "Cyclic task %3d : n=%5d m=%6d", i, n, m);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void   *
cyclic_tasks(arg)
     void          *arg;
{
  pthread_t      tid;

  time_t         now, last;

  now = last = time(NULL);

  tid = pthread_self();
  (void) pthread_detach(tid);

  for (;;)
  {
    int            i;

    MESSAGE_INFO(12, "*** cyclic_tasks running...");

    sleep(dt_loop);
    now = time(NULL);

    if (last + dt_loop > now)
      continue;

    last = now;

    for (i = 0; i < DIMTASKS; i++)
    {
      uint64_t   tms;

      if (tasks[i].signature != SIGNATURE)
        continue;

      if (tasks[i].last + tasks[i].dt > now)
        continue;

      if (tasks[i].function == NULL)
        continue;

      tasks[i].last = now;

      /* XXX do something with result ??? */
      tms = time_ms();
      (void) tasks[i].function(tasks[i].arg);
      tasks[i].work += (time_ms() - tms);
      tasks[i].count++;
    }
  }
  return NULL;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if 0
int
test_task(void *arg)
{
  MESSAGE_INFO(10, "Hi ! %s : %ld", STRNULL(arg, "(null)"), time(NULL) - t_start);

  return 0;
}
#endif
