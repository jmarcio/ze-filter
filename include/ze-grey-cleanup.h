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
 *  Creation     : Tue Apr 12 14:14:49 CEST 2005
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


#ifndef __ZE_GREY_CLEANUP_H

/** @addtogroup Greylisting 
 *
 * @{
 */

void       set_grey_dewhitelist_threshold(double val);

bool       grey_check_bad_smtp_client(char *ip, uint32_t flags);

/** @} */

# define __ZE_GREY_CLEANUP_H    1
#endif /* __ZE_GREY_CLEANUP_H */

