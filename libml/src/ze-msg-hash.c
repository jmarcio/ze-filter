
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
 *  Creation     : Wed May  9 22:18:40 CEST 2007
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


#include <ze-sys.h>
#include <libml.h>
#include <libze.h>
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
  ZEMD5_T             ctx;
  unsigned char       dig[64];

  memset(dig, 0, sizeof (dig));
  zeMD5_Init(&ctx);
  zeMD5_Update(&ctx, sin, strlen((char *) sin));
  zeMD5_Final(&ctx, dig);

  base64_encode(sout, szout, dig, ZE_MD5_DIGESTLENGTH);

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
  ZESHA1_T            ctx;
  unsigned char       dig[64];

  memset(dig, 0, sizeof (dig));
  zeSHA1_Init(&ctx);
  zeSHA1_Update(&ctx, sin, strlen((char *) sin));
  zeSHA1_Final(&ctx, dig);

  base64_encode(sout, szout, dig, ZE_SHA1_DIGESTLENGTH);

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
  for (i = 0; i < szin; i++) {
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
     char               *sin;
     size_t              szout;
{
  unsigned char       dig[64];

  memset(dig, 0, sizeof (dig));
  switch (code) {
    case HASH_SHA1:
      {
        ZESHA1_T            ctx;

        zeSHA1_Init(&ctx);
        zeSHA1_Update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        zeSHA1_Final(&ctx, dig);
        hexa_encode(sout, szout, dig, ZE_SHA1_DIGESTLENGTH);
      }
      break;
    case HASH_MD5:
      {
        ZEMD5_T             ctx;

        zeMD5_Init(&ctx);
        zeMD5_Update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        zeMD5_Final(&ctx, dig);
        hexa_encode(sout, szout, dig, ZE_MD5_DIGESTLENGTH);
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
     char               *sin;
     size_t              szout;
{
  unsigned char       dig[64];

  memset(dig, 0, sizeof (dig));
  switch (code) {
    case HASH_SHA1:
      {
        ZESHA1_T            ctx;

        zeSHA1_Init(&ctx);
        zeSHA1_Update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        zeSHA1_Final(&ctx, dig);
        base64_encode(sout, szout, dig, ZE_SHA1_DIGESTLENGTH);
      }
      break;
    case HASH_MD5:
      {
        ZEMD5_T             ctx;

        zeMD5_Init(&ctx);
        zeMD5_Update(&ctx, (unsigned char *) sin, strlen((char *) sin));
        zeMD5_Final(&ctx, dig);
        base64_encode(sout, szout, dig, ZE_MD5_DIGESTLENGTH);
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

  switch (code) {
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
