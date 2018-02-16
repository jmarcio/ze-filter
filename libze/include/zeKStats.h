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

#ifndef __ZE_KSTATS__

typedef struct kstats_T {
  int           n;
  double        st;
  double        st2;
  double        min;
  double        max;
} kstats_T;

#define         MAX_CLASS       8

#define   KSTATS_INITIALIZER     {0, 0., 0., 0., 0.}

double  zeKMean(kstats_T *s);
double  zeKStdDev(kstats_T *s);
double  zeKMin(kstats_T *s);
double  zeKMax(kstats_T *s);
long    zeKCount(kstats_T *s);

void    zeKStatsReset(kstats_T *);
void    zeKStatsUpdate(kstats_T *, double );
void    kstats_print(kstats_T *);
void    print_inference(kstats_T *, double , char *);

#if 0
double FGauss(double x);
double erf(double x);
double erfc(double x);

double FGaussI(double x);
double erfi(double x);
double erfci(double x);

double confidence_interval(double p, double stddev, int n);
#endif

#define __ZE_KSTATS__

#endif


