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

#ifndef __JLIVEHISTORY_H__

#define             LH_WORKTIME      0
#define             LH_BADRCPT       1
#define             LH_SPAMTRAP      2
#define             LH_EMPTYCONN     3
#define             LH_EMPTYMSGS     4
#define             LH_BADMX         5
#define             LH_LONGCONN      6
#define             LH_HI_SCORE      7
#define             LH_BAD_RESOLVE   8

#define             LH_MAX           9

void                livehistory_reset ();
bool                livehistory_clean_table ();

int                 livehistory_check_host (char *, time_t, int);
int                 livehistory_add_entry(char *, time_t, int, int);

void                livehistory_log_table(int, bool);


#define __JLIVEHISTORY_H__
#endif


