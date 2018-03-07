
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
 *  Creation     : Fri Jan 21 14:26:51 CET 2005
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


#ifndef __ZE_GREY_H

/** @addtogroup Greylisting
 *
 * Greylisting Filtering
 * @{
 */


#define             GREY_OK       200
#define             GREY_WAIT     400
#define             GREY_REJECT   500
#define             GREY_ERROR    700
#define             GREY_DUNNO    100

#define             GREY_EMAIL_NONE    0
#define             GREY_EMAIL_FULL    1
#define             GREY_EMAIL_USER    2
#define             GREY_EMAIL_HOST    3

#define             GREY_ADDR_NONE     0
#define             GREY_ADDR_FULL     1
#define             GREY_ADDR_NET      2

#define             GREY_STANDALONE    0
#define             GREY_CLIENT        1
#define             GREY_SERVER        2

#define             GREY_DW_NONE              0x00
#define             GREY_DW_NULLSENDER        0x01
#define             GREY_DW_BAD_RESOLVE       0x02
#define             GREY_DW_DOMAIN_MISMATCH   0x04
#define             GREY_DW_BAD_CLIENT        0x08
#define             GREY_DW_SPAMTRAP          0x10
#define             GREY_DW_BAD_RCPT          0x20
#define             GREY_DW_BAD_MX            0x40

#define             GREY_DW_ALL             0xFFFF

#define             GDB_NONE        0
#define             GDB_PENDING     1
#define             GDB_VALID       2
#define             GDB_WHITELIST   3
#define             GDB_BLACKLIST   4
#define             GDB_ALL         0xFF

#define             GREY_ARGS     16
#define             ARG_DATE_INIT  0
#define             ARG_DATE_UPDT  1
#define             ARG_IP         2
#define             ARG_HOSTNAME   3
#define             ARG_FROM       4
#define             ARG_RESOLVE    5
#define             ARG_COUNT      6
#define             ARG_FLAGS      7

int                 grey_check(char *, char *, char *, char *, bool *, bool);
int                 grey_validate(char *, char *, char *, char *);

typedef             bool(*greycleanup_F) (char *);

bool                grey_init(char *, bool, int);
bool                grey_reload();
void                grey_close();

bool                grey_set_tuples(char *ip, char *from, char *to);

bool                grey_set_delays(time_t tp_min_norm, time_t tp_max_norm,
                                    time_t tp_min_null, time_t tp_max_null);

bool                grey_set_lifetime(time_t tv, time_t tw, time_t tb);

bool                grey_set_max_pending(int normal, int bounce);

bool                grey_set_cleanup_interval(time_t tclean);

void                grey_set_compat_domain_check(bool enable);

bool                grey_list(int, int);

void                grey_set_dewhite_flags(char *, bool);

int                 grey_dump(int fd, char *which, time_t dt);

int                 grey_upload(char *fname, char *which);

int                 grey_dbcount(int);

bool                grey_remove_entry(char *where, char *key);

bool                grey_remove_entries_from_file(char *where, char *fname);

/** @} */

#define __ZE_GREY_H    1
#endif             /* __ZE_GREY_H */
