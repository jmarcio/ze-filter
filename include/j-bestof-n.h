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
 *  Creation     : Sun Aug 19 22:21:21 CEST 2007
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


#ifndef J_BESTOF_N_H


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


# define J_BESTOF_N_H    1
#endif             /* J_BESTOF_N_H */
