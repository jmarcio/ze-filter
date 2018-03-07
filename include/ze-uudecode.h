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


#ifndef __ZE_UUDECODE_H

/** @addtogroup MsgTools
 *
 * @{
 */
 
typedef struct {
  uint32_t            signature;
  size_t              size;
  char               *name;
  mode_t              mode;
  void               *buf;
} UU_BLOCK_T;

bool               uudecode_buffer (char *, UU_BLOCK_T *);

bool                uudecode_file (char *, UU_BLOCK_T *);

void                free_uu_block(UU_BLOCK_T *);

/** @} */

#define __ZE_UUDECODE_H
#endif
