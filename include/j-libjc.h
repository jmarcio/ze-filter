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


#ifndef __J_LIBJC_H__

#include "version.h"
#include "defs.h"

#include "j-sys.h"

#include "macros.h"

#include "j-time.h"
#include "kstats.h"
#include "j-logit.h"
#include "j-linkedlist.h"
#include "j-name2id.h"
#include "j-regex.h"
#include "j-decode.h"
#include "j-base64.h"
#include "j-qp.h"
#include "j-rfc2822.h"
#include "j-demime.h"
#include "j-convert-8to7.h"
#include "j-html.h"
#include "j-divers.h"
#include "j-smtp-divers.h"
#include "j-strings.h"
#include "j-strconvert.h"
#include "j-buffer.h"
#include "j-morpho.h"
#include "j-ipv4.h"
#include "j-ipv6.h"
#include "j-ip.h"
#include "j-inet.h"
#include "j-syslog.h"
#include "j-txtlog.h"
#include "j-server.h"
#include "j-client.h"
#include "j-avclient.h"
#include "j-dns.h"
#include "j-dns-parse.h"
#include "j-ipc.h"
#include "j-db.h"
#include "j-map.h"
#include "md5.h"
#include "sha1.h"
#include "j-msg-hash.h"

#include <zmLibs.h>

#include "j-databases.h"

#include "j-resolve-cache.h"
#include "j-btree.h"
#include "j-rdfile.h"
#include "j-shmem.h"
#include "j-table.h"
#include "j-uudecode.h"
#include "j-unattach.h"
#include "j-load.h"

#include "j-cyclic.h"
#include "j-mbox.h"
#include "j-msg-score.h"
#include "j-bfilter.h"
#include "j-bcheck.h"
#include "j-lr-funcs.h"
#include "j-bestof-n.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define SIGNATURE        0x70071234

#define SZ_IP            32
#define SZ_NAME          64

#define HOSTNAMECPY(dst,src,sz)			    \
  {                                                 \
    size_t n;                                       \
    n = strlen(src);                                \
                                                    \
    strlcpy(dst, "...", sz);                        \
    if (n >= sz - 3)				    \
      strlcpy(dst + 3, n - sz + 4, sz - 3);	    \
    else                                            \
      strlcpy(dst, src, sz);			    \
  } while (0)




#define __J_LIBJC_H__
#endif
