/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Sun Mar  2 11:51:21 CET 2008
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


#ifndef J_NDC_HELP_H

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool                ndc_help(int fd,
                             char *helpstr, bool help, int argc, char **argv);

# define J_NDC_HELP_H    1
#endif             /* J_NDC_HELP_H */
