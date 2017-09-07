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

#ifndef __JTHROTTLE_H__


bool                connrate_init (size_t, size_t);
void                connrate_free();
bool                connrate_resize(size_t , size_t);

bool                connrate_cleanup_table(time_t now);
int                 connrate_update_table (time_t);

void                connrate_log_table ();
void                connrate_print_table (int, int, int, int);
void                connrate_save_table (char *);
int                 connrate_read_table (char *);

int                 connrate_add_host_entry (char *, time_t);
int                 connrate_add_rcpt_entry (char *, int, time_t);
int                 connrate_add_bounce_entry (char *, time_t);

int                 connrate_check_host (char *);
int                 connrate_check_rcpt (char *);
int                 connrate_check_bounce (char *);

extern unsigned int connrate_interval;  /* 1 minute */
extern unsigned int connrate_window;  /* 10 minutes */

void                add_throttle_entry (time_t);

void                update_throttle(time_t);
bool                update_throttle_dos();
bool                check_throttle_dos(void);
void                log_throttle_stats(void);

#define __JTHROTTLE_H__
#endif


