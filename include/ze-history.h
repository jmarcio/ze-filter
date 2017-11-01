/*
 *
 * ze-filter - Mail Server Filter for sendmail
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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#ifndef __JHISTORY_H__

typedef struct History_T History_T;
typedef struct HistRaw_T HistRaw_T;
typedef struct HistRes_T HistRes_T;


#define H_SUMMARY       0
#define H_EMPTY         1
#define H_REJ_EMPTY     2
#define H_BADRCPT       3
#define H_REJ_BADRCPT   4
#define H_REJ_OPEN      5
#define H_THROTTLE      6
#define H_REJ_THROTTLE  7
#define H_RESOLVE       8
#define H_REJ_RESOLVE   9
#define H_REJ_REGEX    10
#define H_XFILES       11
#define H_SPAMTRAP     12
#define H_REJ_BADMX    13
#define H_REJ_GREY     14


bool            raw_history_open (bool);

void            raw_history_close ();

bool            raw_history_add_entry (SMFICTX *);

bool            res_history_update (History_T *, char *, time_t, time_t, bool);

void            res_history_print (History_T *, char *, char *, bool, bool);

void            res_history_summary (History_T *, char *, time_t, time_t, bool, bool, int, int);

bool            load_live_history (History_T *, time_t, time_t);

#define __JHISTORY_H__
#endif
