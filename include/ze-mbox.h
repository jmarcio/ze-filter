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
 *  Creation     : Mon Jun 19 11:38:04 CEST 2006
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


#ifndef __ZE_MBOX_H

/** @addtogroup MsgTools
*
* @{
*/

typedef             bool(*mbox_F) (char *fname, int id, void *arg);

int                 mbox_handle(char *fname, mbox_F func, void *arg);

int                 maildir_handle(char *dirname, mbox_F func, void *arg);

/** @} */

# define __ZE_MBOX_H    1
#endif             /* __ZE_MBOX_H */
