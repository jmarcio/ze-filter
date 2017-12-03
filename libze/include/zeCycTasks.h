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
 *  Creation     : Fri Apr 28 11:02:56 CEST 2006
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


#ifndef ZE_CYCLIC_H

typedef int    (*CYCLIC_F) (void *);

bool           CycTasks_Init(time_t interval);

bool           CycTasks_Register(CYCLIC_F task, void *arg, time_t period);

void           CycTasks_Stats();

# define ZE_CYCLIC_H    1
#endif             /* J_CYCLIC_H */
