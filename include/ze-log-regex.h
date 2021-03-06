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
 *  Creation     : Tue Jan 17 16:24:39 CET 2006
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


#ifndef __ZE_LOG_REGEX_H

/** @addtogroup Logging
*
* @{
*/

extern bool           mailregexlog2file;

bool    log_found_regex(char *, char *, char *, int, int, char *);
bool            log_regex_reopen();

/** @} */

# define __ZE_LOG_REGEX_H    1
#endif /* __ZE_LOG_REGEX_H */

