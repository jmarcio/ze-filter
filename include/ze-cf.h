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


#ifndef __ZE_CF_H

/** @addtogroup TxtConf
*
* @{
*/

/* **************************************************** */

#include "cfh-defs.h"


/* **************************************************** */

#define     MK_CF_NONE            0
#define     MK_CF_NULL            1
#define     MK_CF_DEFAULT         2
#define     MK_CF_RUNNING         3

#define     OPT_NO                0
#define     OPT_YES               1

#define     OPT_OK                0
#define     OPT_REJECT            1
#define     OPT_NOTIFY            2
#define     OPT_DISCARD           3
#define     OPT_X_HEADER          4

#define     OPT_TEMPFAIL          2

#define     OPT_SHOW              0
#define     OPT_HIDE              1

#define     OPT_SYSTEM            0
#define     OPT_SENDMAIL          1

#define     OPT_OTHER           255

#define     OPT_SENDER            0

#define     OPT_SUBJECT           0

#define     OPT_TEXT              0
#define     OPT_DB                1

#define     OPT_NONE              0
#define     OPT_ACCESS            1

#define     OPT_INTERNAL          0
#define     OPT_CLAMAV            1

#define     OPT_STANDALONE        0
#define     OPT_CLIENT            1

#define     OPT_DEFAULT           0
#define     OPT_ONE_WIN           1
#define     OPT_MAJORITY_WIN      2

#define     OPT_PLAIN             HASH_PLAIN
#define     OPT_MD5               HASH_MD5
#define     OPT_SHA1              HASH_SHA1

/* **************************************************** */

int             configure (char *, char *, bool);
int             cf_read_file (char *);
void            reload_cf_tables();

int             cf_init ();

void            cf_clear_values ();
void            cf_defaults ();

void            cf_dump (int, bool);
void            mk_cf_file(int, bool, bool);

int             cf_get_id(char *);

int             cf_set_val (int id, char *val);

int             cf_set_str_val (int id, char *val);
int             cf_append_str_val (int id, char *val);

int             cf_set_int_val (int id, int val);
int             cf_set_double_val (int id, double val);
int             cf_set_enum_val (int id, int val);

char           *cf_get_str (int id);
int             cf_get_int (int id);
double          cf_get_double (int id);

#if 1
extern char    *conf_file;
#endif

extern time_t   last_reconf_date;

extern unsigned int statistics_interval;

extern char     domain[];

extern int      priority;

/** @} */

#define __ZE_CF_H
#endif
