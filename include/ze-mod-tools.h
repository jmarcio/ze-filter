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
 *  Creation     : Sun Jun 10 15:23:49 CEST 2007
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


#ifndef ZE_MOD_TOOLS_H

void                ctx2mod_args(mod_ctx_T * mod, SMFICTX * ctx);

int                 mod2ctx_result(int r);

bool                do_module_callback(SMFICTX *ctx, int step, int *result);

# define ZE_MOD_TOOLS_H    1
#endif             /* J_MOD_TOOLS_H */
