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
 *  Creation     : Tue May 23 13:43:08 CEST 2006
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


#ifndef __ZE_READ_CONF_DATA_H

/** @addtogroup TxtConf
*
* @{
*/

typedef bool  (*read_conf_data_file_F)(char *, char *);

bool
read_conf_data_file(char *cfdir,
		    char *fname,
		    char *dfile,
		    read_conf_data_file_F func);

/** @} */

# define __ZE_READ_CONF_DATA_H    1
#endif /* __ZE_READ_CONF_DATA_H */

