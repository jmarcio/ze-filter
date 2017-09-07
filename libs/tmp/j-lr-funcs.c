/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2009 - Ecole des Mines de Paris
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 Jose-Marcio.Martins@ensmp.fr
 *
 *  Historique   :
 *  Creation     : Thu May 28 17:51:54 CEST 2009
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
 * web site : http://j-chkmail.ensmp.fr
 */


#include <j-sys.h>
#include <j-chkmail.h>
#include <j-lr-funcs.h>


/* ****************************************************************************
 **************************************************************************** */
bool
lr_data_open(fname)
     char               *fname;
{
  return TRUE;
}

/* ****************************************************************************
 **************************************************************************** */
bool
lr_data_close()
{
  return TRUE;
}

/* ****************************************************************************
 **************************************************************************** */
bool
lr_data_dump(fname)
     char               *fname;
{
  return TRUE;
}

/* ****************************************************************************
 **************************************************************************** */
bool
lr_classify(id, fname, mscore)
     char               *id;
     char               *fname;
     test_score_T       *mscore;
{
  return TRUE;
}

/* ****************************************************************************
 **************************************************************************** */
bool
lr_learn(id, fname, mscore, spam)
     char               *id;
     char               *fname;
     bool                spam;
     test_score_T       *mscore;
{
  return TRUE;
}

/* ****************************************************************************
 **************************************************************************** */
bool
lr_learn_options(active, threshold)
     bool                active;
     double              threshold;
{
  return TRUE;
}
