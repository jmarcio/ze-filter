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

#ifndef _KSTATS_

typedef struct kstats_T {
  int           n;
  double        st;
  double        st2;
  double        min;
  double        max;
} kstats_T;

#define         MAX_CLASS       8

#define   KSTATS_INITIALIZER     {0, 0., 0., 0., 0.}

double  kmean(kstats_T *s);
double  kstddev(kstats_T *s);
double  kmin(kstats_T *s);
double  kmax(kstats_T *s);
long    kcount(kstats_T *s);

void    kstats_reset(kstats_T *);
void    kstats_update(kstats_T *, double );
void    kstats_print(kstats_T *);
void    print_inference(kstats_T *, double , char *);


double FGauss(double x);
double erf(double x);
double erfc(double x);

double FGaussI(double x);
double erfi(double x);
double erfci(double x);

double confidence_interval(double p, double stddev, int n);

#define _KSTATS_

#endif


