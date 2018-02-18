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


#ifndef __ZE_RESOURCE_H

/** @addtogroup Systools
*
* @{
*/

#define FD_LEVEL_OK     0
#define FD_LEVEL_SHORT  1
#define FD_LEVEL_HI     2

int                 count_file_descriptors(void);

int                 check_file_descriptors(void);

int                 setup_file_descriptors();

bool                enable_coredump(bool);

bool                check_rusage();

/** @} */

#define __ZE_RESOURCE_H
#endif
