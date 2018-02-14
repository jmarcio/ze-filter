
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
#if 0
#include <ze-sys.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>
#endif
#include "libmilter/mfapi.h"

#ifndef HAVE_XXFI_UNKNOWN
#define HAVE_XXFI_UNKNOWN               1
#endif
#ifndef HAVE_XXFI_DATA
#define HAVE_XXFI_DATA                  1
#endif
#ifndef HAVE_XXFI_NEGOTIATE
#define HAVE_XXFI_NEGOTIATE             1
#endif
#ifndef HAVE_XXFI_SIGNAL
#define HAVE_XXFI_SIGNAL                0
#endif

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static sfsistat     mlfi_connect(SMFICTX *, char *, _SOCK_ADDR *);
static sfsistat     mlfi_helo(SMFICTX *, char *);
static sfsistat     mlfi_envfrom(SMFICTX *, char **);
static sfsistat     mlfi_envto(SMFICTX *, char **);
static sfsistat     mlfi_header(SMFICTX *, char *, char *);
static sfsistat     mlfi_eoh(SMFICTX *);
static sfsistat     mlfi_body(SMFICTX *, unsigned char *, size_t);
static sfsistat     mlfi_eom(SMFICTX *);
static sfsistat     mlfi_close(SMFICTX *);
static sfsistat     mlfi_abort(SMFICTX *);
static sfsistat     mlfi_cleanup(SMFICTX *, bool);

#define MLFIPRIV(ctx)  (ctx != NULL ? (CTXPRIV_T *) smfi_getpriv(ctx) : NULL)

typedef struct mlfiPfiv {
  int                 dummy;

  unsigned long       pf0;
  unsigned long       pf1;
  unsigned long       pf2;
  unsigned long       pf3;
} CTXPRIV_T;


#define FREE(x)					\
  do {						\
    if (x != NULL)				\
      free(x);					\
    x = NULL;					\
  } while (0)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_connect(ctx, hostname, hostaddr)
     SMFICTX            *ctx;
     char               *hostname;
     _SOCK_ADDR         *hostaddr;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  if (hostname == NULL)
    hostname = "HOST_UNKNOWN";

  if (priv == NULL) {
    printf("mlfi_connect : priv is NULL\n");
  }
  FREE(priv);

  smfi_setpriv(ctx, NULL);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_helo(ctx, name)
     SMFICTX            *ctx;
     char               *name;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  printf("smfi_hello -> OK !\n");
  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_envfrom(SMFICTX * ctx, char **from)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_envto(SMFICTX * ctx, char **to)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_header(SMFICTX * ctx, char *headerf, char *headerv)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_eoh(SMFICTX * ctx)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_body(SMFICTX * ctx, unsigned char *buf, size_t size)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_eom(SMFICTX * ctx)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_close(SMFICTX * ctx)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_abort(SMFICTX * ctx)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_cleanup(SMFICTX * ctx, bool ok)
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              sfsistat
mlfi_unknown(ctx, cmd)
     SMFICTX            *ctx;
     const char         *cmd;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              sfsistat
mlfi_data(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  return SMFIS_CONTINUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_negotiate(SMFICTX * ctx,
               unsigned long f0, unsigned long f1,
               unsigned long f2, unsigned long f3,
               unsigned long *pf0, unsigned long *pf1,
               unsigned long *pf2, unsigned long *pf3)
{
  CTXPRIV_T          *priv = NULL;
  sfsistat            result = SMFIS_ALL_OPTS;

  *pf0 = f0;
  *pf1 = f1;
  *pf2 = f2;
  *pf3 = f3;

  printf("malloc inside mlfi_negotiate\n");
  priv = malloc(sizeof (CTXPRIV_T));
  if (priv != NULL) {
    printf("malloc priv OK\n");
    if (smfi_setpriv(ctx, priv) != MI_SUCCESS) {
      FREE(priv);
      printf("smfi_setpriv(priv) error\n");

      result = SMFIS_TEMPFAIL;

      goto fin;
    }
  }

  if (priv != NULL) {
  }

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
struct smfiDesc     smfilter = {
  "SampleFilter",               /* filter name */
  SMFI_VERSION,                 /* version code -- do not change */
  SMFIF_ADDHDRS,                /* flags */
  mlfi_connect,                 /* connection info filter */
  mlfi_helo,                    /* SMTP HELO command filter */
  mlfi_envfrom,                 /* envelope sender filter */
  mlfi_envto,                   /* envelope recipient filter */
  mlfi_header,                  /* header filter */
  mlfi_eoh,                     /* end of header */
  mlfi_body,                    /* body block filter */
  mlfi_eom,                     /* end of message */
  mlfi_abort,                   /* message aborted */
  mlfi_close                    /* connection cleanup */
#if HAVE_XXFI_UNKNOWN
    , mlfi_unknown              /* unknown command */
#endif
#if HAVE_XXFI_DATA && HAVE_XXFI_UNKNOWN
    , mlfi_data                 /* data */
#endif
#if HAVE_XXFI_NEGOTIATE && HAVE_XXFI_DATA && HAVE_XXFI_UNKNOWN
    , mlfi_negotiate            /* negotiate */
#endif
#if HAVE_XXFI_SIGNAL
    , NULL                      /* signale */
#endif
};

int
main(argc, argv)
     int                 argc;
     char               *argv[];
{
  int                 res;

#if 0
  (void) smfi_setconn("local:/tmp/null-milter.sock");
#else
  (void) smfi_setconn("inet:2040@127.0.0.1");
#endif

  if (smfi_register(smfilter) == MI_FAILURE) {
    fprintf(stderr, "smfi_register failed\n");
    exit(EX_UNAVAILABLE);
  }

  res = smfi_main();
  printf("smfi_mail returned %d\n", res);
  return res;
}

/* eof */
