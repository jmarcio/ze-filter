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


#ifndef __ZE_CPULOAD_H

/** @addtogroup Systools
 *
 * System Tools
 * @{
 */

#define JCPU_IDLE          0
#define JCPU_USER          1
#define JCPU_NICE          2
#define JCPU_KERNEL        3
#define JCPU_WAIT          4


double              get_cpu_load_info(int);
void                print_cpu_load_info();
void                log_cpu_load_info();

bool                cpuload_start(void);
void                cpuload_stop(void);

void                load_measure_stop(void);

/** @} */

#define __ZE_CPULOAD_H
#endif
