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
 *  Creation     : Sun Aug 19 22:21:21 CEST 2007
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
#include <ze-filter.h>
#include <ze-bestof-n.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
dblcmp(a, b)
     const void         *a;
     const void         *b;
{
  double             *ra, *rb;

  ra = (double *) a;
  rb = (double *) b;

  return (int) (fabs(*ra) - fabs(*rb));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
bestof_init(b, dim, bcmp)
     bestof_T           *b;
     int                 dim;
     bestcomp_F          bcmp;
{
  ASSERT(b != NULL);

  memset(b, 0, sizeof (*b));

  b->dim = dim;
  b->n = 0;
  if (bcmp != NULL)
    b->bcmp = bcmp;
  else
    b->bcmp = dblcmp;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
bestof_add(b, v)
     bestof_T           *b;
     double              v;
{
  if (b->n < b->dim)
  {
    b->best[b->n++] = v;
    return TRUE;
  }

  qsort(b->best, b->n, sizeof (double), b->bcmp);
  if (b->bcmp(&v, &b->best[0]) > 0)
    b->best[0] = v;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
bestof_average(b)
     bestof_T           *b;
{
  int                 i;
  double              v = 0.;

  if (b->n <= 0)
    return 0.;

  for (i = 0; i < b->n; i++)
    v += b->best[i];

  return v / b->n;
}
