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


#ifndef __ZE_DNS_URLBL_H__

typedef struct
{
  char                line[256];

  char                bl[64];

  char                code[64];

  char                onmatch[20];

  uint16_t            flags;

  double              odds;
  double              score;
} urlbl_T;


bool                init_urlbl_table();

void                dump_urlbl_table();
bool                load_urlbl_table(char *cfdir, char *fname);

uint32_t            check_urlbl_table(char *id, char *name, urlbl_T *bl);

bool                check_dns_urlbl(char *name, char *bl, char *code,
                                    size_t size, bool recurse);

#define __ZE_DNS_URLBL_H__
#endif
