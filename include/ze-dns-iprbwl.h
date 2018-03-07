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


#ifndef __ZE_DNS_IPRBWL_H

/** @addtogroup RBL
*
* @{
*/

typedef struct
{
  char                line[256];

  char                rbwl[64];

  char                code[64];
  char                netclass[20];

  char                checks[20];
  char                onmatch[20];

  uint16_t            flags;

  double              odds;
} iprbwl_T;

bool                init_iprbwl_table();

void                dump_iprbwl_table();
bool                load_iprbwl_table(char *cfdir, char *fname);

uint32_t            check_iprbwl_table(char *id, char *ip, char *name,
                                       iprbwl_T * rbwl);

bool                check_dns_iprbwl(char *ip, char *name, char *rbwl, char *code,
                                     size_t size);

/** @} */

#define __ZE_DNS_IPRBWL_H
#endif
