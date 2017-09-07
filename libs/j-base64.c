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

#include <j-sys.h>

#include "j-libjc.h"
#include "j-base64.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char        *B64 =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define ISB64CHAR(c)      ((strchr (B64, (int) c) != NULL ) ? TRUE : FALSE)

static char
b64index(c)
     char                c;
{
  char               *p = strchr(B64, c);

  if (p != NULL)
    return (p - B64);

  return -1;
}


static int
decode_buf64(o, i)
     char               *o;
     char               *i;

{
  int                 ix[4];

  if ((i == NULL) || (o == NULL))
    return 0;

  o[0] = o[1] = o[2] = o[3] = '\0';

  if (!ISB64CHAR(i[0]) || !ISB64CHAR(i[1]))
    return 0;

  ix[0] = b64index(i[0]);
  ix[1] = b64index(i[1]);
  o[0] = (ix[0] << 2) | (ix[1] >> 4);

  if ((i[2] == '=') || !ISB64CHAR(i[2]))
    return 1;

  ix[2] = b64index(i[2]);

  o[1] = (ix[1] << 4) | (ix[2] >> 2);

  if ((i[3] == '=') || !ISB64CHAR(i[3]))
    return 2;

  ix[3] = b64index(i[3]);

  o[2] = (ix[2] << 6) | ix[3];

  return 3;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
base64_decode(out, in, nout, nin)
     char               *out;
     char               *in;
     size_t             *nout;
     size_t             *nin;

{
  char               *p = in;
  char               *q = out;
  int                 res = 0;

  char                ibuf[5];
  char                obuf[4];
  int                 di = 0;
  int                 nsep = 0;

  size_t              no = 0;

  if ((in == NULL) || (out == NULL))
    return 0;

  memset(ibuf, 0, sizeof (ibuf));
  memset(obuf, 0, sizeof (obuf));
  *q = '\0';
  no = 1;

  nsep = 0;
  while ((p != NULL) && (*p != '\0'))
  {

    if (*p == '=')
    {
      while (di < 4)
        ibuf[di++] = '=';
      break;
    }

    if (strchr(" \t\r\n", *p))
    {
      if (nsep++ == 2)
        break;
      p++;
      continue;
    }
    nsep = 0;

    if (!ISB64CHAR(*p))
    {
      res = 1;
      break;
    }

    ibuf[di++] = *p;
    ibuf[di] = '\0';

    if (di == 4)
    {
      int                 l;

      if ((nout != NULL) && (no + 3 > *nout))
        break;
      l = decode_buf64(obuf, ibuf);

      strcat(q, obuf);
      q += l;
      no += l;
      *q = '\0';
      di = 0;
      memset(ibuf, 0, sizeof (ibuf));
      memset(obuf, 0, sizeof (obuf));
    }

    p++;
    continue;

  }

  if ((di > 0) && ((nout == NULL) || (no + 3 < *nout)))
  {
    int                 l;

    while (di < 4)
      ibuf[di++] = '=';

    l = decode_buf64(obuf, ibuf);
    strcat(q, obuf);
    q += l;
    no += l;
    *q = '\0';
  }
  *q = '\0';

  if (nin != NULL)
    *nin = (size_t) (p - in);
  if (nout != NULL)
    *nout = (size_t) (q - out);

  return res;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void               *
new_base64_decode(bufin, size)
     char               *bufin;
     size_t             *size;

{
  char               *p = bufin;
  char               *q = NULL, *bufout;
  int                 res = 0;

  char                ibuf[5];
  char                obuf[4];
  int                 di = 0;
  int                 nsep = 0;

  size_t              sz_in, sz_out;
  size_t              no = 0;

  if ((bufin == NULL) || (strlen(bufin) == 0))
  {
    LOG_MSG_ERROR("input buffer NULL or empty");
    return NULL;
  }

  sz_in = strlen(bufin);
  if ((bufout = (char *) malloc(sz_in + 1)) == NULL)
  {
    LOG_SYS_ERROR("malloc output buffer");
    return NULL;
  }
  memset(bufout, 0, sz_in + 1);

  memset(ibuf, 0, sizeof (ibuf));
  memset(obuf, 0, sizeof (obuf));
  *q = '\0';
  no = 1;

  nsep = 0;
  while ((p != NULL) && (*p != '\0'))
  {

    if (*p == '=')
    {
      while (di < 4)
        ibuf[di++] = '=';
      break;
    }

    if (strchr(" \t\r\n", *p))
    {
      if (nsep++ == 2)
        break;
      p++;
      continue;
    }
    nsep = 0;

    if (!ISB64CHAR(*p))
    {
      res = 1;
      break;
    }

    ibuf[di++] = *p;

    if (di == 4)
    {
      int                 l;

      if (no + 3 > sz_in)
        break;
      l = decode_buf64(obuf, ibuf);
      strcat(q, obuf);
      q += l;
      no += l;
      *q = '\0';
      di = 0;
      memset(ibuf, 0, sizeof (ibuf));
      memset(obuf, 0, sizeof (obuf));
    }

    p++;
    continue;

  }

  if ((di > 0) && (no + 3 < sz_in))
  {
    int                 l;

    while (di < 4)
      ibuf[di++] = '=';

    l = decode_buf64(obuf, ibuf);
    strcat(q, obuf);
    q += l;
    no += l;
    *q = '\0';
  }
  *q = '\0';

  sz_out = (size_t) (q - bufout);
  if (size != NULL)
    *size = sz_out;

  return bufout;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
base64_encode(sout, szout, sin, szin)
     char               *sout;
     size_t              szout;
     unsigned char      *sin;
     size_t              szin;
{
  unsigned char      *q = sin;
  unsigned char       bin[3];
  char                cin[5];
  int                 sz;

  ASSERT(sout != NULL);
  ASSERT(szout > 0);

  memset(sout, 0, szout);

  if (3 * szout < 4 * szin)
    return FALSE;

  sz = szin;
  while (sz > 0)
  {
    int                 i;

    memset(bin, 0, sizeof (bin));
    bin[0] = (unsigned char) q[0];
    if (sz > 1)
      bin[1] = (unsigned char) q[1];
    if (sz > 2)
      bin[2] = (unsigned char) q[2];

    strlcpy(cin, "====", sizeof (cin));

    i = (bin[0] & 0xFC) >> 2;
    cin[0] = B64[i];

    i = ((bin[0] & 0x03) << 4) | ((bin[1] & 0xF0) >> 4);
    cin[1] = B64[i];

    if (sz > 1)
    {
      i = ((bin[1] & 0x0F) << 2) | ((bin[2] & 0xC0) >> 6);
      cin[2] = B64[i];
    }

    if (sz > 2)
    {
      i = (bin[2] & 0x3F);
      cin[3] = B64[i];
    }
    strlcat(sout, cin, szout);

    sz -= 3;
    q += 3;
  }

  return TRUE;
}
