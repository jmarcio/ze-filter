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


#ifndef ZE_BESTOF_N_H


typedef int (*bestcomp_F) (const void *, const void *);

#define BOFDIM          48

typedef struct
{
  int                 dim;
  int                 n;
  double              best[BOFDIM];
  bestcomp_F          bcmp;
} bestof_T;

bool                bestof_init(bestof_T *b, int dim, bestcomp_F bcmp);

bool                bestof_add(bestof_T *b, double v);

double              bestof_average(bestof_T *b);

int                 bestof_count(bestof_T *b);


# define ZE_BESTOF_N_H    1
#endif             /* J_BESTOF_N_H */
