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


#ifndef __ZE_SPOOL_H

#define      WHY_UNKNOWN         0
#define      WHY_XFILE           1
#define      WHY_VIRUS           2
#define      WHY_POLICY          3
#define      WHY_SPAM            4
#define      WHY_ARCHIVE         5
#define      WHY_QUARANTINE      6


#define      SUFFIX_UNKNOWN      ".unknown"
#define      SUFFIX_XFILE        ".xfile"
#define      SUFFIX_VIRUS        ".virus"
#define      SUFFIX_POLICY       ".policy"
#define      SUFFIX_SPAM         ".spam"
#define      SUFFIX_ARCHIVE      ".archive"
#define      SUFFIX_QUARANTINE   ".quarantine"

bool                spool_file_create(CTXPRIV_T *);
bool                spool_file_write(CTXPRIV_T *, char *, size_t);
bool                spool_file_close(CTXPRIV_T *);
bool                spool_file_forget(CTXPRIV_T *);

bool                spool_file_is_open(CTXPRIV_T *);

/*
** Spool file 
*/
typedef struct MSG_SPOOL_T MSG_SPOOL_T;

struct MSG_SPOOL_T
{
  char               *fname;    /* spool file name */
  char               *fsuffix;
  int                 fd;
  bool                fp_open;  /* True if spool open */
  bool                save_msg; /* shall message be quarantined */
};


#define __ZE_SPOOL_H
#endif
