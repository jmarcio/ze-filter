/*
 *
 * ze-filter - Mail Server Filter for sendmail
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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>

#include "ze-filter.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
conv2ascii(c)
     int                 c;
{
#if 1
  if (strchr("àäâ", c) != NULL)
    return 'a';
  if (strchr("éèêë", c) != NULL)
    return 'e';
  if (strchr("îï", c) != NULL)
    return 'i';
  if (strchr("'ôö", c) != NULL)
    return 'o';
  if (strchr("ùû", c) != NULL)
    return 'u';
  if (strchr("ç", c) != NULL)
    return 'c';
#else
  switch (c)
  {
    case 'à':
    case 'ä':
    case 'â':
      return 'a';
    case 'é':
    case 'è':
    case 'ê':
    case 'ë':
      return 'e';
    case 'î':
    case 'ï':
      return 'i';
    case 'ô':
    case 'ö':
      return 'o';
    case 'ù':
    case 'û':
      return 'u';
    case 'ç':
      return 'c';
  }
#endif

  return c;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
text2lowerascii(buf, size)
     char               *buf;
     size_t              size;
{
  char               *p = buf;
  int                 c;

  if (buf == NULL)
    return;

  for (p = buf; (size > 0) && (*p != '\0'); p++)
  {
    c = (conv2ascii(*p) + 256) % 256;
    *p = tolower(c);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define ONLY_LOWER        1

double
entropy_monogram(buf, sz)
     char               *buf;
     size_t              sz;
{
  int                 freq[256];
  int                 i;
  double              sum = 0.;
  int                 nc = 0;
  char               *p;

  if ((buf == NULL) || (sz == 0))
    return 0.;

  memset(freq, 0, sizeof (freq));
  sum = 0;

  for (i = 0, p = buf; i < sz; i++, p++)
  {
    int                 c;

    c = conv2ascii(*p);
    c = (c + 256) % 256;
    c = tolower(c);

    freq[c]++;
    nc++;
  }

  for (i = 0; i < 256; i++)
  {
    if (freq[i] != 0)
    {
      double              k;

      k = ((double) freq[i]) / ((double) nc);
      sum -= k * log(k);
    }
  }
  return sum / log(2.);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  int                 key;
  int                 count;
  double              pp;
  double              pc;
}
hash_rec_T;

static int
get_hash_index(k, h, sz)
     uint32_t            k;
     hash_rec_T         *h;
     size_t              sz;
{
  int                 i, hv = 0;

  {
    int                 c, d;
    uint32_t            tk = k;

    c = d = 0;
    for (i = 0; i < 4; i++)
    {
      d = tk & 0x000000FF;
      tk = tk >> 8;
      c = d;
      c ^= c << 6;
      hv += (c << 11) ^ (c >> 1);
      hv ^= (d << 14) + (d << 7) + (d << 4) + d;
    }
    hv %= sz;
  }
  for (i = 0; i < sz; i++)
  {
    int                 j = (hv + i) % sz;

    if (h[j].key == k)
      return j;

    if (h[j].key == 0)
    {
      h[j].key = k;
      return j;
    }
  }

  ZE_LogMsgWarning(0, "Hash table overflow");
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define     SZH0       256
#define     SZH1      4096
#define     SZH2     16384
#define     SZH3     16384


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
text_buffer_entropy(buf, sz, e0, e1, e2)
     char               *buf;
     size_t              sz;
     double             *e0;
     double             *e1;
     double             *e2;
{
  hash_rec_T         *h0, *h1, *h2;
  int                 i;
  double              sum = 0.;
  int                 nc = 0;
  char               *p;

  if ((buf == NULL) || (sz < 6))
    return FALSE;

  text2lowerascii(buf, sz);

  h0 = (hash_rec_T *) malloc(SZH0 * sizeof (hash_rec_T));
  h1 = (hash_rec_T *) malloc(SZH1 * sizeof (hash_rec_T));
  h2 = (hash_rec_T *) malloc(SZH2 * sizeof (hash_rec_T));
  if ((h0 == NULL) || (h1 == NULL) || (h2 == NULL))
  {
    FREE(h0);
    FREE(h1);
    FREE(h2);
    return FALSE;
  }

  memset(h0, 0, SZH0 * sizeof (hash_rec_T));
  memset(h1, 0, SZH1 * sizeof (hash_rec_T));
  memset(h2, 0, SZH2 * sizeof (hash_rec_T));

  sum = 0;
  for (i = 0, p = buf; i < sz - 2; i++, p++)
  {
    int                 c0, c1, c2;
    int                 hi;
    uint32_t            k;

    c0 = p[0];

    k = c0;
    hi = get_hash_index(k, h0, SZH0);
    h0[hi].count++;

    c1 = p[1];

    k = (c1 << 8) | c0;
    hi = get_hash_index(k, h1, SZH1);
    h1[hi].count++;

    c2 = p[2];

    k = (c2 << 16) | (c1 << 8) | c0;
    hi = get_hash_index(k, h2, SZH2);
    h2[hi].count++;

    nc++;
  }

  sum = 0.;
  for (i = 0; i < SZH0; i++)
  {
    if (h0[i].count > 0)
    {
      h0[i].pp = ((double) h0[i].count) / nc;
      sum -= h0[i].pp * log(h0[i].pp);
    }
  }
  *e0 = sum / log(2.);

  sum = 0.;
  for (i = 0; i < SZH1; i++)
  {
    if (h1[i].count > 0)
    {
      int                 hi;

      h1[i].pp = ((double) h1[i].count) / nc;
      hi = h1[i].key & 0xFF;
      hi = get_hash_index(hi, h0, SZH0);

      if (h0[hi].count > 0)
      {
        h1[i].pc = h1[i].pp / h0[hi].pp;
        sum -= h1[i].pp * log(h1[i].pc);
      }
    }
  }
  *e1 = sum / log(2.);

  sum = 0;
  for (i = 0; i < SZH2; i++)
  {
    if (h2[i].count > 0)
    {
      int                 hi;

      h2[i].pp = ((double) h2[i].count) / nc;
      hi = h2[i].key & 0xFFFF;
      hi = get_hash_index(hi, h1, SZH1);

      if (h1[hi].pp > 0)
      {
        h2[i].pc = h2[i].pp / h1[hi].pp;
        sum -= h2[i].pp * log(h2[i].pc);
      }
    }
  }
  *e2 = sum / log(2.);

  FREE(h0);
  FREE(h1);
  FREE(h2);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

double
entropy_token_class(buf, sz)
     char               *buf;
     size_t              sz;
{
  int                 freq[256];
  int                 i;
  double              sum = 0.;
  int                 nc = 0;
  char               *p;

  if ((buf == NULL) || (sz == 0))
    return 0.;

  memset(freq, 0, sizeof (freq));
  sum = 0;

  for (i = 0, p = buf; i < sz; i++, p++)
  {
    int                 c;

    c = conv2ascii(*p);
    c = (c + 256) % 256;

    nc++;
    if (isalpha(c))
    {
      freq[2]++;
      continue;
    }

    if (isdigit(c))
    {
      freq[3]++;
      continue;
    }
    if (isspace(c))
    {
      freq[4]++;
      continue;
    }
    if (ispunct(c))
    {
      freq[5]++;
      continue;
    }
    if (iscntrl(c))
    {
      freq[6]++;
      continue;
    }
    freq[0]++;
  }

  for (i = 0; i < 256; i++)
  {
    if (freq[i] != 0)
    {
      double              k;

      k = ((double) freq[i]) / ((double) nc);
      sum -= k * log(k);
    }
  }
  return sum / log(2.);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

double
entropy_punct_class(buf, sz)
     char               *buf;
     size_t              sz;
{
  int                 freq[256];
  int                 i;
  double              sum = 0.;
  int                 nc = 0;
  char               *p;

  if ((buf == NULL) || (sz == 0))
    return 0.;

  memset(freq, 0, sizeof (freq));
  sum = 0;

  for (i = 0, p = buf; i < sz; i++, p++)
  {
    int                 c;

    c = conv2ascii(*p);
    c = (c + 256) % 256;

    nc++;
    if (isalnum(c))
    {
      freq[1]++;
      continue;
    }

    if (isspace(c))
    {
      freq[2]++;
      continue;
    }
    if (ispunct(c))
    {
      freq[2]++;
      continue;
    }

    ZE_MessageInfo(5, "Nor alnum, space, punct %c %3d ??? ", c, c);

    freq[0]++;
  }

  for (i = 0; i < 256; i++)
  {
    if (freq[i] != 0)
    {
      double              k;

      k = ((double) freq[i]) / ((double) nc);
      sum -= k * log(k);
    }
  }
  return sum / log(2.);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
buf_extract_tokens(buf)
     char               *buf;
{
  char               *s, *ptr;

  for (s = strtok_r(buf, " ", &ptr); s != NULL; s = strtok_r(NULL, " ", &ptr))
  {
    printf("--> %s\n", s);
  }
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  kstats_T            html_st;
  size_t              html_sz;
  kstats_T            plain_st;
  size_t              plain_sz;
}
DATA_T;


static              bool
entropy_mime_part(buf, size, id, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *id;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mime_part;
{
  DATA_T             *data = (DATA_T *) arg;

  double              h0, h1, h2, h3, h4, ratio = 1.;
  char               *cleanbuf = NULL;
  char               *mtype = "PLAIN";
  char               *wbuf = buf;
  int                 n;

  uint32_t            dt;
  kstats_T            st = KSTATS_INITIALIZER;

  if (data == NULL)
    return FALSE;

  if (type != MIME_TYPE_TEXT)
    return TRUE;

#if 1
  if (abs(strspn(buf, " \t\r\n") - size) < 4)
    return TRUE;
#endif
#if 1
  if (size < 6)
    return TRUE;
#endif

  if (strcasecmp("text/html", mime_part->mime) == 0)
  {
    mtype = "HTML ";

    n = check_valid_html_tags(NULL, buf);

    ZE_LogMsgInfo(9, "NOT VALID TAGS = %6d", n);

    cleanbuf = cleanup_html_buffer(buf, strlen(buf));

#if 1
    ZE_MessageInfo(9, "\nBUF ...\n%s\n", buf);

    if (cleanbuf != NULL)
      ZE_MessageInfo(9, "\nBUF ...\n%s\n", cleanbuf);

    {
      char               *x = NULL;

      x = realcleanup_text_buf(cleanbuf, strlen(cleanbuf));
      if (x != NULL)
      {
        ZE_MessageInfo(9, "\nBUF ...\n%s\n", x);
        buf_extract_tokens(x);
        FREE(x);
      }
    }
#endif
    wbuf = cleanbuf;

    if (strlen(wbuf) > 0)
      ratio = ((double) strlen(buf)) / strlen(wbuf);

  }

  dt = time_ms();
  text_buffer_entropy(wbuf, strlen(wbuf) + 1, &h0, &h1, &h2);
  dt = time_ms() - dt;
  ZE_MessageInfo(9, "DT = %ld", dt);

  h3 = entropy_token_class(wbuf, strlen(wbuf) + 1);
  h4 = entropy_punct_class(wbuf, strlen(wbuf) + 1);

  (void) text_word_length(wbuf, &st, strlen(wbuf));

  ZE_MessageInfo(9,
               "%s ENTROPY = %7.3f %7.3f %7.3f %7.3f %7.3f - SIZE = %5d %5d %5d %7.2f",
               mtype, h0, h1, h2, h3, h4, size, strlen(buf), strlen(wbuf),
               ratio);

  ZE_MessageInfo(9,
               "%s WORDS   = %7.3f %7.3f %7.3f %7.3f - SIZE = %5d %5d %5d %7.2f",
               mtype,
               kmin(&st), kmean(&st), kmax(&st), kstddev(&st),
               size, strlen(buf), strlen(wbuf), ratio);

  if (0)
  {
    long                prob[256], nb;

    if ((nb = text_buf_histogram(wbuf, strlen(wbuf) + 1, prob)) > 0)
    {
      int                 i;

      for (i = 0; i < 256; i++)
      {
        if (prob[i] > 0)
          ZE_MessageInfo(9, "%s HISTO   = %3d %6d   %c",
                       mtype, i, prob[i], (isprint(i) ? i : '.'));
      }
    }
  }

  FREE(cleanbuf);

  return TRUE;
}



/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
message_entropy(id, fname)
     char               *id;
     char               *fname;
{
  DATA_T              data;

  if (fname == NULL)
    return FALSE;

  memset(&data, 0, sizeof (data));

  return decode_mime_file(id, fname, NULL, entropy_mime_part, &data);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  kstats_T            html_st;
  size_t              html_sz;
  kstats_T            plain_st;
  size_t              plain_sz;
}
HTTP_T;

#if 1
#define       URL_DOMAIN_EXPRESSION      "http[s]?://[^ /<>\\(\\)\"\'?]*"
#else
#define       URL_DOMAIN_EXPRESSION      "http://[^ /<>\"]*"
#endif

static              bool
mime_extract_http_urls(buf, size, id, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *id;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mime_part;
{
  HTTP_T             *data = (HTTP_T *) arg;

  if (data == NULL)
    return FALSE;

  if (type != MIME_TYPE_TEXT)
    return TRUE;

  if (abs(strspn(buf, " \t\r\n") - size) < 4)
    return TRUE;
  if (size < 6)
    return TRUE;

  {
    long                pi, pf;
    char               *p = buf;
    char                sout[4096];

    memset(sout, 0, sizeof (sout));
    while (strexpr(p, "(ftp|http)[s]?://[^ /<>\"]*", &pi, &pf, TRUE))
    {
      char                sout[1024];
      char                c = '.';
      long                xi, xf, xh;

      if (strexpr(p + pf, "></a>", &xi, NULL, TRUE) && (xi - pf) < 30)
      {
        bool                okh, okf;

        okh = strexpr(p + pf, ">", &xh, NULL, TRUE) && (xh < xi);
        okf = strexpr(p + pf, ">.+</a>", &xf, NULL, TRUE) && (xf < xi);

        c = (okf || okh) ? '.' : 'H';
      }
      strncpy(sout, p + pi, pf - pi);
      sout[pf - pi] = 0;
#if 1
      {
        char *p, *q;

        for (p = q = sout; *p != '\0'; p++) {
          if (strchr(" \t\r\n()", *p) == NULL)
	    *q++ = *p;
        }
        *q = '\0';
      }
#else
      if ((xh = strcspn(sout, " \t\r\n()")) >= 0)
        sout[xh] = '\0';
#endif

      ZE_MessageInfo(0, "HTTP : %c %s", c, sout);

      p += pf;
    }
  }

  return TRUE;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define SZBUF   0x20000

bool
message_extract_http_urls(id, fname)
     char               *id;
     char               *fname;
{
  HTTP_T              data;

  if (fname != NULL)
  {
    memset(&data, 0, sizeof (data));

    return decode_mime_file(id, fname, NULL, mime_extract_http_urls, &data);
  } else
  {
    char               *buf = NULL;
    size_t              sz;

    if ((buf = malloc(SZBUF)) != NULL)
    {
      sz = read(STDIN_FILENO, buf, SZBUF);
      if (sz > 0)
        return decode_mime_buffer(id, buf, sz, 0, NULL, mime_extract_http_urls,
                                  &data);
      if (sz < 0)
        return FALSE;
    }
  }
  return FALSE;
}
