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
/* ************************************************************************ */
/* Kameleon - A Discrete Event Simulator                                    */
/*                                                                          */
/* Created by : Jose Marcio Martins da Cruz                                 */
/*              Ecole Nationale Superieure des Mines de Paris               */
/*                                                                          */
/* History :                                                                */
/*   15 May 1996 : Creation                                                 */
/* ************************************************************************ */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ze-sys.h>

#include "kstats.h"

double
kmean (s)
     kstats_T              *s;
{
  if ((s == NULL) || (s->n == 0))
    return 0;

  return s->st / s->n;
}

double
kstddev (s)
     kstats_T              *s;
{
  if ((s == NULL) || (s->n < 2))
    return 0;
  return sqrt (fabs ((s->st2 - s->st * s->st / s->n) / (s->n - 1)));
}

double
kmin (s)
     kstats_T              *s;
{
  if ((s == NULL) || (s->n == 0))
    return 0;

  return s->min;
}

double
kmax (s)
     kstats_T              *s;
{
  if ((s == NULL) || (s->n == 0))
    return 0;

  return s->max;
}

long
kcount(s)
     kstats_T              *s;
{
  if ((s == NULL) || (s->n == 0))
    return 0;

  return s->n;
}

void
kstats_reset (s)
     kstats_T              *s;
{
  if (s == NULL)
    return;
  memset (s, 0, sizeof (*s));
}

void
kstats_update (s, t)
     kstats_T              *s;
     double              t;
{
  if (s == NULL)
    return;
  if (s->n == 0)
    s->min = s->max = t;
  s->n++;
  s->st += t;
  s->st2 += t * t;
  if (t < s->min)
    s->min = t;
  if (t > s->max)
    s->max = t;
}

#if 0
void
print_inference (s, p, c)
     kstats_T              *s;
     double              p;
     char               *c;
{
  if ((s != NULL) && (s->n > 1)) {
    printf ("--> ");
    if (c != NULL)
      printf ("%s <--", c);
    printf ("\n");
    printf ("    Mean Value        : %9.4f\n", kmean (s));
    printf ("    Std. Deviation    : %9.4f\n", kstddev (s));
    printf ("    Confidence (%4.2f) : %9.4f\n", p,
            confidence_interval (p, kstddev (s), s->n));
    printf ("    Notes             : %9d\n", s->n);
  }
}

#endif

/* --------------------------------------------- */

#ifndef OK
#define OK      0
#define ERROR   -1
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define SQRT2PI 2.50662827479

#define sqr(x)  ((x)*(x))

typedef struct {
  double              x;
  double              y;
} point;

static point       *p = NULL;

/* --------------------------------------------- */
static double
fn (double x)
{
  return (exp (-sqr (x) / 2) / SQRT2PI);
}

static int
init_table (int nint, double nmax)
{
  int                 i;
  double              x, y = 0.;
  double              dx = nmax / nint;

  if (p == NULL) {
    if ((p = (point *) malloc ((nint + 1) * sizeof (point))) == NULL)
      return ERROR;
  }
  memset (p, 0, (nint + 1) * sizeof (point));

  p[0].x = 0.;
  p[0].y = 0.;
  for (i = 1; i < nint + 1; i++) {
    x = i * dx;
    y += dx * (fn (x) + fn (x - dx)) / 2;
    p[i].x = x;
    p[i].y = y;
  }
  return OK;
}

/* --------------------------------------------- */
static int          NINT = 1000;
static double       NMAX = 10.;

#define sign(x) (((x) > 0.) ? 1 : (((x) < 0.) ? -1 : 0))

static double
f (double x)
{
  double              dx = NMAX / NINT;
  int                 q;
  double              r;

  int                 i, j;
  int                 s;

  if ((p == NULL) && ((init_table (NINT, NMAX)) != OK)) {
    printf ("Erreur pendant creation de matrice normale\n");
    exit (1);
  }

  s = sign (x);
  x = fabs (x);

  if (x > NMAX)
    return (s * p[NINT].y);

  q = (int) (x / dx);
  r = x - q * dx;

  i = j = q;
  switch (sign (r)) {
    case -1:
      i--;
      break;
    case 0:
      break;
    case 1:
      j++;
      break;
  }

  if (i != j)
    return (s *
            (p[i].y + (x - p[i].x) * (p[j].y - p[i].y) / (p[j].x - p[i].x)));
  else
    return (s * p[i].y);
}

double
FGauss (double x)
{
  return (0.5 + f (x));
}

#if !defined(HAVE_ERF)
double
erf (double x)
{
  return (2 * f (x));
}
#endif

#if !defined(HAVE_ERFC)
double
erfc (double x)
{
  return (1 - 2 * f (x));
}
#endif

/* --------------------------------------------- */
static int
luby (double y)
{
  int                 i, j, k;

  i = 0;
  j = NINT;
  while (j - i > 1) {
    k = (j + i) / 2;
    if (p[k].y > y) {
      j = k;
      continue;
    }
    if (p[k].y < y) {
      i = k;
      continue;
    }
    i = j = k;
    break;
  }
  return j;
}

static int
glby (double y)
{
  int                 i, j, k;

  i = 0;
  j = NINT;
  while (j - i > 1) {
    k = (j + i) / 2;
    if (p[k].y > y) {
      j = k;
      continue;
    }
    if (p[k].y < y) {
      i = k;
      continue;
    }
    i = j = k;

    break;
  }
  return i;
}

static double
finv (double y)
{
  int                 i, j;

  if ((p == NULL) && ((init_table (NINT, NMAX)) != OK)) {
    printf ("Erreur pendant creation de matrice normale\n");
    exit (1);
  }

  i = glby (y);
  j = luby (y);

  if (j - i > 1)
    return 0.;

  if (i == j)
    return p[i].x;

  return p[i].x + (y - p[i].y) * (p[j].x - p[i].x) / (p[j].y - p[i].y);
}

double
FGaussI (double y)
{
  if ((y > 1.) || (y < 0.))
    return 0;

  if (y >= 0.5)
    return (finv (y - 0.5));
  return (-finv (0.5 - y));
}

double
erfi (double y)
{
  if ((y > 1.) || (y < -1.))
    return 0;

  return (sign (y) * finv (fabs (y / 2)));
}

double
erfci (double y)
{
  if ((y > 2.) || (y < 0.))
    return 0;

  return (erfi (1. - y));
}

/* --------------------------------------------- */
double
confidence_interval (double p, double stddev, int n)
{
  if ((p < 0.) || (p > 1.))
    return 0.;

  return (stddev * erfi (p) / sqrt ((double) n));
}
