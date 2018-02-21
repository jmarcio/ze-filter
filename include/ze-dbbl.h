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

#ifndef __ZE_DBBL_H

/** @addtogroup RBL
*
* @{
*/

#define  BL_ORACLE              1
#define  BL_PATMATCH            2
#define  BL_BADRCPT             3
#define  BL_THROTTLE            4


bool                db_open_blacklist();
bool                db_check_blacklist(char *ip);
bool                db_close_blacklist();

/* dynamic or static blacklist */

typedef struct db_map_T
{
  char                why[32];
  char                key[32];

  int                 weight;
  time_t              date;
  char                ipres[24];
  char                msg[64];
} db_map_T;


bool                db_blackliste_check(char *why, char *key, db_map_T *r);

bool                db_map_check(char *, char *, char *, char *, size_t);
bool                db_map_add(char *, char *, char *, char *);

bool                db_map_open(char *bl);
bool                db_map_close(char *bl);
bool                db_map_close_all(void);

/** @} */

#define __ZE_DBBL_H
#endif
