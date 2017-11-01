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

#ifndef _J_FILEEXP_H_

int                 free_fext ();
int                 add_fext (char *);

void                init_default_file_extensions ();
void                init_file_extension_regex ();

void                list_filename_extensions (int fd);

bool                check_filename_xfile (char *);

void                dump_xfiles_table ();
bool                load_xfiles_table (char *, char *);
bool                check_xfiles (char *, char *, size_t, char *, size_t );

#define _J_FILEEXP_H_
#endif
