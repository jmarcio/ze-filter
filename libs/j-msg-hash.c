/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Wed May  9 22:18:40 CEST 2007
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


#include <ze-sys.h>
#include <ze-libjc.h>
#include <ze-msg-hash.h>


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
jmc_str2md5(sout, sin, szout)
     char               *sout;
     unsigned char      *sin;
     size_t              szout;
{
  jmc_md5_t           ctx;
  unsigned char                dig[64];

  memset(dig, 0, sizeof (dig));
  jmc_md5_init(&ctx);
  jmc_md5_update(&ctx, sin, strlen((char *) sin));
  jmc_md5_final(&ctx, dig);

  base64_encode(sout, szout, dig, JMC_MD5_DIGESTLENGTH);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
jmc_str2sha1(sout, sin, szout)
     char               *sout;
     unsigned char      *sin;
     size_t              szout;
{
  jmc_sha1_t          ctx;
  unsigned char                dig[64];

  memset(dig, 0, sizeof (dig));
  jmc_sha1_init(&ctx);
  jmc_sha1_update(&ctx, sin, strlen((char *) sin));
  jmc_sha1_final(&ctx, dig);

  base64_encode(sout, szout, dig, JMC_SHA1_DIGESTLENGTH);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
hexa_encode(sout, szout, sin, szin)
     char               *sout;
     size_t              szout;
     unsigned char      *sin;
     size_t              szin;
{
  int                 i;

  memset(sout, 0, szout);
  for (i = 0; i < szin; i++)
  {
    char                s[4];

    snprintf(s, sizeof (s), "%02X", sin[i]);

    strlcat(sout, s, szout);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
str2hash2hex(code, sout, sin, szout)
     int                 code;
     char               *sout;
     char      *sin;
     size_t              szout;
{
  unsigned char       dig[64];

  memset(dig, 0, sizeof (dig));
  switch (code)
  {
    case HASH_SHA1:
      {
        jmc_sha1_t          ctx;

        jmc_sha1_init(&ctx);
        jmc_sha1_update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        jmc_sha1_final(&ctx, dig);
        hexa_encode(sout, szout, dig, JMC_SHA1_DIGESTLENGTH);
      }
      break;
    case HASH_MD5:
      {
        jmc_md5_t           ctx;

        jmc_md5_init(&ctx);
        jmc_md5_update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        jmc_md5_final(&ctx, dig);
        hexa_encode(sout, szout, dig, JMC_MD5_DIGESTLENGTH);
      }
      break;
  }
  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
str2hash2b64(code, sout, sin, szout)
     int                 code;
     char               *sout;
     char      *sin;
     size_t              szout;
{
  unsigned char                dig[64];

  memset(dig, 0, sizeof (dig));
  switch (code)
  {
    case HASH_SHA1:
      {
        jmc_sha1_t          ctx;

        jmc_sha1_init(&ctx);
        jmc_sha1_update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        jmc_sha1_final(&ctx, dig);
        base64_encode(sout, szout, dig, JMC_SHA1_DIGESTLENGTH);
      }
      break;
    case HASH_MD5:
      {
        jmc_md5_t           ctx;

        jmc_md5_init(&ctx);
        jmc_md5_update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        jmc_md5_final(&ctx, dig);
        base64_encode(sout, szout, dig, JMC_MD5_DIGESTLENGTH);
      }
      break;
  }
  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
hash_label2code(label)
     char               *label;
{
  if (label == NULL)
    return HASH_PLAIN;

  if (STRCASEEQUAL(label, "MD5"))
    return HASH_MD5;
  if (STRCASEEQUAL(label, "SHA1"))
    return HASH_SHA1;
  if (STRCASEEQUAL(label, "PLAIN"))
    return HASH_PLAIN;
  return HASH_PLAIN;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
hash_code2label(code)
     int                 code;
{
  char               *s = "PLAIN";

  switch (code)
  {
    case HASH_PLAIN:
      s = "PLAIN";
      break;
    case HASH_MD5:
      s = "MD5";
      break;
    case HASH_SHA1:
      s = "SHA1";
      break;
  }
  return s;
}
