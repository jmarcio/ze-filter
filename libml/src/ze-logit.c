/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Wed Aug 15 15:27:48 CEST 2007
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
#include <ze-logit.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
logit(p)
     double              p;
{
  if (p <= 0.)
    return 0.;

  if (p >= 1.)
    return 1.;
  p += (p >= 0.5 ? -1e-10 : 1e-10);

  return log(p / (1. - p));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
logitodd(p)
     double              p;
{
  if (p <= 0.)
    return 0;

  return log(p);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
logit2(a, b)
     int                 a;
     int                 b;
{
  return log((a + 0.5) / (b + 0.5));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
logitinv(x)
     double              x;
{
  return 1. / (1. + exp(-x));
}
