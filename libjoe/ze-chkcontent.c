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


#define JDEBUG 0

#define LOG_LEVEL       10
#define H0_LOG_LEVEL    12

#ifndef _FFR_MSG_ENTROPY
#define _FFR_MSG_ENTROPY  0
#endif


#ifndef ENTROPY_BUF_SIZE
#define ENTROPY_BUF_SIZE 16000
#endif


int                 check_rfc2822_headers_count(char *, header_T *);
int                 check_rfc2822_headers_syntax(char *, header_T *);

static uint32_t     check_header_date(char *id, header_T * headers);

static int          check_unwanted_html_tags(char *, char *, bestof_T *);
static bool         check_unwanted_boundary(char *, char *, bestof_T *);
static bool         check_unwanted_mailer(char *, char *, bestof_T *);
static bool         check_unwanted_charset(char *, char *, bestof_T *);
static int          check_unwanted_expressions(char *, char *, bestof_T *);

static int          count_html_tags(char *);
static int          count_html_comments(char *);


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#if 1
#define URLBL_SCORE()       (MAX(urlbl_score, data->scores.urlbl))
#define REGEX_SCORE()       (MAX(MAX(clean_score, raw_score), data->scores.body))
#else
#define URLBL_SCORE()       (urlbl_score + data->scores.urlbl)
#define REGEX_SCORE()       (MAX(clean_score, raw_score) + data->scores.body)
#endif
#define CURRENT_SCORE()     (URLBL_SCORE() + REGEX_SCORE())

static              bool
check_mime_part(buf, size, id, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *id;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mime_part;
{
  spamchk_T          *data = (spamchk_T *) arg;

  int                 clean_score = 0, raw_score = 0, urlbl_score = 0;

  bool                simple_text = TRUE;

  int                 x = CURRENT_SCORE();

  id = STRNULL(id, "00000000.000");
  if (mime_part == NULL)
  {
    ZE_LogMsgWarning(0, "mime_part NULL");
    return FALSE;
  }

  if (data == NULL)
  {
    ZE_LogMsgWarning(0, "data NULL");
    return FALSE;
  }

  if (arg == NULL)
  {
    ZE_LogMsgWarning(0, "arg NULL");
    return FALSE;
  }

  data->size += mime_part->size;
  data->nb_part++;

  switch (type)
  {
    case MIME_TYPE_TEXT:
      data->nb_text++;
      break;
    case MIME_TYPE_IMAGE:
      data->nb_image++;
      break;
    case MIME_TYPE_AUDIO:
      data->nb_audio++;
      break;
    case MIME_TYPE_VIDEO:
      data->nb_video++;
      break;
    case MIME_TYPE_APPLICATION:
      data->nb_application++;
      break;
    case MIME_TYPE_EXTENSION_TOKEN:
      data->nb_token++;
      break;
    case MIME_TYPE_MESSAGE:
      data->nb_message++;
      break;
    case MIME_TYPE_MULTIPART:
      data->nb_multipart++;
      break;
  }

  if (mime_part->hdrs != NULL)
  {
    rfc2822_hdr_T      *h;

    if (mime_part->type == MIME_TYPE_IMAGE)
      ;

    if ((h = rfc2822_lookup_header(mime_part->hdrs, "content-id")) != NULL)
    {
      char               *name = NULL;
      rfc2822_hdr_T      *hx = NULL;

      hx = rfc2822_lookup_header(mime_part->hdrs, "content-type");
      if (hx != NULL)
        name = rfc2822_get_main_attr(hx);

      if (name != NULL && strncasecmp(name, "image", strlen("image")) == 0)
      {
        SET_BIT(data->flags.msg, SPAM_MSG_CONTENT_ID);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - M%02d unwanted Content-ID for %s", id,
                       SPAM_MSG_CONTENT_ID, STRNULL(name, "???"));
      }
      FREE(name);
    }
  }
#if 1
  /*
   ** let's check empty attachments... usually appearing inside HTML
   ** messages pointing to an inline inline attached empty image
   */
  if (mime_part->hdrs != NULL)
  {
    rfc2822_hdr_T      *hdr = NULL;
    char               *name = NULL;
    char               *mime = NULL;

    hdr = mime_part->hdrs;
    while ((hdr = rfc2822_lookup_header(hdr, "content-type")) != NULL)
    {
      mime = rfc2822_get_main_attr(hdr);
      if (mime != NULL)
      {
        name = rfc2822_get_attr(hdr, "name=");
        if (name != NULL)
        {
          char               *p = NULL;

          p = strrchr(name, '.');

          ZE_MessageInfo(11, "    CTYPE : %-8s %4d %6d %s", STRNULL(p, ".---"),
                       type, size, name);
          if (size == 0)
          {
            switch (type)
            {
              case MIME_TYPE_IMAGE:
              case MIME_TYPE_AUDIO:
              case MIME_TYPE_VIDEO:
              case MIME_TYPE_APPLICATION:
              case MIME_TYPE_EXTENSION_TOKEN:
              case MIME_TYPE_MESSAGE:
                SET_BIT(data->flags.msg, SPAM_MSG_EMPTY_ATTACHMENT);
                if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
                  ZE_MessageInfo(LOG_LEVEL,
                               "%s SPAM CHECK - M%02d empty attachment %s", id,
                               SPAM_MSG_EMPTY_ATTACHMENT, STRNULL(name, "???"));
                break;
              default:
                break;
            }
          }

        }
      }
      FREE(name);
      FREE(mime);

      hdr = hdr->next;
    }

    hdr = mime_part->hdrs;
    while ((hdr = rfc2822_lookup_header(hdr, "content-disposition")) != NULL)
    {
      mime = rfc2822_get_main_attr(hdr);
      if (mime != NULL)
      {
        name = rfc2822_get_attr(hdr, "filename=");
        if (name != NULL)
        {
          char               *p = NULL;

          p = strrchr(name, '.');

          ZE_MessageInfo(11, "    CDISP : %-8s %4d %6d %s", STRNULL(p, ".---"),
                       type, size, name);
          if (size == 0)
          {
            switch (type)
            {
              case MIME_TYPE_IMAGE:
              case MIME_TYPE_AUDIO:
              case MIME_TYPE_VIDEO:
              case MIME_TYPE_APPLICATION:
              case MIME_TYPE_EXTENSION_TOKEN:
              case MIME_TYPE_MESSAGE:
                SET_BIT(data->flags.msg, SPAM_MSG_EMPTY_ATTACHMENT);
                if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
                  ZE_MessageInfo(LOG_LEVEL,
                               "%s SPAM CHECK - M%02d empty attachment %s", id,
                               SPAM_MSG_EMPTY_ATTACHMENT, STRNULL(name, "???"));
                break;
              default:
                break;
            }
          }
        }
      }
      FREE(name);
      FREE(mime);

      hdr = hdr->next;
    }
  }
#endif

  /* spam_oracle */
  if (type != MIME_TYPE_TEXT)
  {
    kstats_update(&data->mksize.other, (double) size);
    return TRUE;
  }
#if 1
  convert_8to7(buf, FALSE);
#else
  /* remove unprintable or 8 bits caracteres before checking text */
  {
    char               *p, *q;
    size_t              sz;

    p = q = buf;
    sz = size;

    for (sz = size; *p != '\0' && sz > 0; p++, sz--)
    {
      int                 c = *((unsigned char *) p);

      if (c == 0x0A || c == 0x0D)
      {
        *q++ = *p;
        continue;
      }

      if (*p == 0x1B)
      {
        size--;
        continue;
      }

      if (c < 0x20)
      {
        if (1)
          *q++ = ' ';
        else
          size--;
        continue;
      }

      if (c > 0x7F)
      {
        char                s[] = "                                "
          "                                "
          "AAAAAAACEEEEIIIIGNOOOOOxOUUUUYPB" "aaaaaaaceeeeiiiionooooo-ouuuuyby";

        c -= 0x80;
        if (c > 0 && c < sizeof (s))
          *q++ = s[0x80];
        continue;
      }

      *q++ = *p;
    }
    *q = '\0';
  }
#endif

  ZE_MessageInfo(19, "TYPE (%d) (%ld) (%s)", type, (long) size, mime_part->mime);

  /*
   **
   ** text/html 
   **
   */
  if (strcasecmp("text/html", mime_part->mime) == 0)
  {
    size_t              real_size = 0;
    int                 nb_tags;

    kstats_update(&data->mksize.html, (double) size);

    data->nb_text_html++;

    simple_text = FALSE;

    if (mime_part->encode == MIME_ENCODE_BASE64)
      data->nb_text_html_base64++;

    data->sz_text_html += mime_part->size;

    data->html.len_clean = mime_part->size;
    data->html.len_raw = mime_part->size;

    /* URL BL */
    if (data->scores.do_urlbl)
      urlbl_score = check_rurlbl(id, data->ip, buf);

    /* clean HTML code and handle it */
    {
      char               *html_clean = NULL;

      html_clean = cleanup_html_buffer(buf, size + 1);
      convert_8to7(html_clean, FALSE);
      if (html_clean != NULL)
      {
        real_size = strlen(html_clean);

        ZE_MessageInfo(19, "Checking HTML clean");
        if (data->scores.do_regex
            && (CURRENT_SCORE() < data->content_max_score))
        {
          clean_score = check_regex(id, data->ip, html_clean, MAIL_BODY);

          if (CURRENT_SCORE() < data->content_max_score)
          {
            char               *x = NULL;

            x = realcleanup_text_buf(html_clean, strlen(html_clean));
            if (x != NULL)
            {
              int                 sc = 0;

              sc = check_regex(id, data->ip, x, MAIL_BODY);
              if (sc > clean_score)
                clean_score = sc;
            }
            FREE(x);
          }
        }

        if (data->scores.do_oracle)
        {
          kstats_T            st = KSTATS_INITIALIZER;

          if (text_word_length(html_clean, &st, strlen(html_clean)))
          {
            data->html.st_wlen = st;
            data->html.len_clean = strlen(html_clean);
          }
        }
      }
#if _FFR_MSG_ENTROPY == 1
      if (data->spam_oracle)
      {
        if (html_clean != NULL)
        {
          if (strlen(html_clean) < ENTROPY_BUF_SIZE)
            text_buffer_entropy(html_clean, strlen(html_clean) + 1,
                                &data->html.h0, &data->html.h1, &data->html.h2);
        } else
        {
          if (size < ENTROPY_BUF_SIZE)
            text_buffer_entropy(buf, size,
                                &data->html.h0, &data->html.h1, &data->html.h2);
        }
      }
#endif
      FREE(html_clean);
    }


    if (data->scores.do_oracle)
    {
      int                 n;

      /* XXX 100000 ??? JOE */
      if (size <= 100000)
        data->html_invalid_tags += check_valid_html_tags(id, buf);

      if ((n = check_unwanted_html_tags(id, buf, &data->best)) > 0)
        data->html_unwanted_tags = n;

      nb_tags = count_html_tags(buf);

      if (nb_tags * 5 > real_size)
        data->html_high_tag_ratio++;

      {
        /*
         **  CLEAN buf from HTML codings
         */
      }
    }
  }

  /* 
   ** 
   ** text/plain
   **
   */
  if (strcasecmp("text/plain", mime_part->mime) == 0)
  {
    kstats_update(&data->mksize.plain, (double) size);

    data->nb_text_plain++;

    simple_text = FALSE;

    if (data->scores.do_oracle)
    {
      {
        char               *pc = buf;
        int                 sz = 0;

        for (pc = buf; pc != NULL && *pc != '\0'; pc++)
          if (isalnum(*pc))
            sz++;

        /* check cleaned up html part size */
        ZE_MessageInfo(19, "PLAIN TEXT size %d", size);
        if (sz >= 0 && sz < 80)
        {
          SET_BIT(data->flags.plain, SPAM_PLAIN_TOO_SHORT);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            ZE_MessageInfo(LOG_LEVEL,
                         "%s SPAM CHECK - P%02d text/plain part too short : %d",
                         id, SPAM_PLAIN_TOO_SHORT, sz);
        }
      }

      if ((mime_part != NULL) && (strlen(mime_part->charset) == 0))
      {
        SET_BIT(data->flags.plain, SPAM_PLAIN_NO_CHARSET);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - P%02d text/plain w/o charset", id,
                       SPAM_PLAIN_NO_CHARSET);
      }

      if (size > data->plain.len_raw)
      {
        kstats_T            st = KSTATS_INITIALIZER;

        data->plain.len_raw = size;
        data->plain.len_clean = size;

        if (text_word_length(buf, &st, strlen(buf)))
        {
          data->plain.st_wlen = st;
          data->plain.len_clean = strlen(buf);
        }
#if _FFR_MSG_ENTROPY == 1
        if (size < ENTROPY_BUF_SIZE)
          text_buffer_entropy(buf, size,
                              &data->plain.h0, &data->plain.h1,
                              &data->plain.h2);
#endif
      }

      if (mime_part->encode == MIME_ENCODE_BASE64)
        data->nb_text_plain_base64++;
      data->sz_text_plain += mime_part->size;

      if (strspn(buf, " \n\r\t") == size)
        data->nb_text_plain_empty++;
    }

    /* URL BL */
    if (data->scores.do_urlbl)
      urlbl_score = check_rurlbl(id, data->ip, buf);

    if (data->scores.do_regex && CURRENT_SCORE() < data->content_max_score)
    {
      char               *x = NULL;

      x = realcleanup_text_buf(buf, strlen(buf));

      if (x != NULL)
      {
        ZE_MessageInfo(30, "BUF ...%s", x);
        clean_score = check_regex(id, data->ip, x, MAIL_BODY);
        FREE(x);
      }
    }
  }

  /*
   **
   ** Text message without mime definition
   **
   */
  if (simple_text)
  {
    kstats_update(&data->mksize.simple, (double) size);

    if (data->scores.do_oracle)
    {
      if (size > data->plain.len_raw)
      {
        kstats_T            st = KSTATS_INITIALIZER;

        data->plain.len_raw = size;
        data->plain.len_clean = size;

        if (text_word_length(buf, &st, strlen(buf)))
        {
          data->plain.st_wlen = st;
          data->plain.len_clean = strlen(buf);
        }
#if _FFR_MSG_ENTROPY == 1
        if ((size < ENTROPY_BUF_SIZE) && (size > 16))
          text_buffer_entropy(buf, size,
                              &data->plain.h0, &data->plain.h1,
                              &data->plain.h2);
#endif
      }

      if (strspn(buf, " \n\r\t") == size)
        data->nb_text_simple_empty++;
    }

    /* URL BL */
    if (data->scores.do_urlbl)
      urlbl_score = check_rurlbl(id, data->ip, buf);

    if (data->scores.do_regex && CURRENT_SCORE() < data->content_max_score)
    {
      char               *x = NULL;

      x = realcleanup_text_buf(buf, strlen(buf));

      if (x != NULL)
      {
        ZE_MessageInfo(30, "BUF ...%s", x);
        clean_score = check_regex(id, data->ip, x, MAIL_BODY);
        FREE(x);
      }
    }
  }

  /* Checking special usual spam expressions */
  data->msg_bad_expressions += check_unwanted_expressions(id, buf, &data->best);

  ZE_MessageInfo(19, "Checking HTML raw and TEXT");
  if (data->scores.do_regex && (CURRENT_SCORE() < data->content_max_score))
  {
    raw_score = check_regex(id, data->ip, buf, MAIL_BODY);
  }

  data->scores.body = REGEX_SCORE();
  data->scores.urlbl = URLBL_SCORE();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
scan_body_contents(id, ip, fname, maxsize, data, flags, scores)
     char               *id;
     char               *ip;
     char               *fname;
     size_t              maxsize;
     spamchk_T          *data;
     msg_flags_T        *flags;
     msg_scores_T       *scores;
{
  size_t              size;
  uint32_t            mime_flags = 0;

  if (fname == NULL)
    return 0;

  size = get_file_size(fname);
  if (size < 0)
    return 0;

  data->spool_size = size;
  data->max_spool_size = maxsize;

  if (scores != NULL)
    data->scores = *scores;

  if (data->scores.do_regex)
    data->scores.do_regex = (size <= maxsize);

  data->content_max_score = cf_get_int(CF_REGEX_MAX_SCORE);
  data->ip = ip;

  bestof_init(&data->best, 4, NULL);

  kstats_reset(&data->mksize.plain);
  kstats_reset(&data->mksize.plain_clean);
  kstats_reset(&data->mksize.html);
  kstats_reset(&data->mksize.html_clean);
  kstats_reset(&data->mksize.simple);
  kstats_reset(&data->mksize.other);
  kstats_reset(&data->mksize.attach);

  /*
   **
   **  ORACLE - pre processing
   **
   */
  {
    uint32_t            flags = 0;

    if ((flags = check_header_date(id, data->hdrs)) != 0)
      data->flags.msg |= flags;
  }

  if (data->scores.do_oracle)
  {
    header_T           *h;

    if ((h = get_msgheader(data->hdrs, "Content-type")) != NULL)
    {
      char                value[256];

      ZE_MessageInfo(15, "%s : got content-type header", id);

      memset(value, 0, sizeof (value));
      if (get_msgheader_attribute(h, "charset", value, sizeof (value))
          && (strlen(value) > 0))
      {
        if (check_unwanted_charset(id, value, &data->best))
        {
          SET_BIT(data->flags.msg, SPAM_MSG_UNWANTED_CHARSET);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            ZE_MessageInfo(LOG_LEVEL,
                         "%s SPAM CHECK - M%02d unwanted charset : %s", id,
                         SPAM_MSG_UNWANTED_CHARSET, value);
        }
      }

      if ((h->value != NULL)
          && strexpr(h->value, "multipart/(alternative|mixed)", NULL, NULL,
                     TRUE))
      {
        if (get_msgheader_attribute(h, "boundary", value, sizeof (value)))
        {
          ZE_MessageInfo(19, "Boundary : %s", value);
          if (check_unwanted_boundary(id, value, &data->best))
          {
            SET_BIT(data->flags.msg, SPAM_MSG_UNWANTED_BOUNDARY);

            if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
              ZE_MessageInfo(LOG_LEVEL,
                           "%s SPAM CHECK - M%02d unwanted boundary (%s)",
                           id, SPAM_MSG_UNWANTED_BOUNDARY, value);
          }
        } else
        {
          SET_BIT(data->flags.msg, SPAM_MSG_UNWANTED_BOUNDARY);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            ZE_MessageInfo(LOG_LEVEL,
                         "%s SPAM CHECK - M%02d short boundary boundary=(%s) len=(%d)",
                         id, SPAM_MSG_UNWANTED_BOUNDARY, "(NULL)", 0);
        }
      }

    }

    if ((h = get_msgheader(data->hdrs, "X-Mailer")) != NULL)
    {
      char               *mailer = STRNULL(h->value, "");

      if (check_unwanted_mailer(id, mailer, &data->best))
      {
        SET_BIT(data->flags.msg, SPAM_MSG_UNWANTED_MAILER);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - M%02d unwanted mailer (%s)",
                       id, SPAM_MSG_UNWANTED_MAILER, mailer);
      }
    }
    if ((h = get_msgheader(data->hdrs, "User-Agent")) != NULL)
    {
      char               *mailer = STRNULL(h->value, "");

      if (check_unwanted_mailer(id, mailer, &data->best))
      {
        SET_BIT(data->flags.msg, SPAM_MSG_UNWANTED_MAILER);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - M%02d unwanted mailer (%s)",
                       id, SPAM_MSG_UNWANTED_MAILER, mailer);
      }
    }

    /* check if Subject is all HI CAPS */
    for (h = data->hdrs; h != NULL && (h = get_msgheader(h, "Subject")) != NULL;
         h = h->next)
    {
      bool                nm = FALSE;
      bool                na = FALSE;
      char               *p = h->value;

      if (strlen(h->value) == 0)
        continue;

      for (p = h->value; p != NULL && *p != '\0'; p++)
      {
        if (islower(*p))
          nm = TRUE;
        if (isalpha(*p))
          na = TRUE;
      }
      if (!nm)
      {
        SET_BIT(data->flags.msg, SPAM_MSG_SUBJECT_HI_CAPS);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - M%02d Subject doesn't contains lower case chars : %s",
                       id, SPAM_MSG_SUBJECT_HI_CAPS, h->value);
      }
      if (!na)
      {
        SET_BIT(data->flags.msg, SPAM_MSG_SUBJECT_NO_ALPHA);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - M%02d Subject doesn't contains alpha chars : %s",
                       id, SPAM_MSG_SUBJECT_NO_ALPHA, h->value);
      }
    }

    /*
     ** RFC 2822 headers compliance
     */
    data->nb_rfc2822_hdrs_errors = check_rfc2822_headers_count(id, data->hdrs);
    if (data->nb_rfc2822_hdrs_errors > 0)
    {
      SET_BIT(data->flags.msg, SPAM_MSG_RFC2822_HEADERS);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL, "%s SPAM CHECK - M%02d RFC2822 headers", id,
                     SPAM_MSG_RFC2822_HEADERS);
    }

    if (TRUE)
    {
      int                 n;

      n = check_rfc2822_headers_syntax(id, data->hdrs);
      if (n > 0)
      {
        SET_BIT(data->flags.msg, SPAM_MSG_HEADERS_SYNTAX);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL, "%s SPAM CHECK - M%02d Headers syntax :",
                       id, SPAM_MSG_HEADERS_SYNTAX);
      }
      data->headers_syntax_errors = n;
    }

    /*
     ** Base 64 encoded messge
     */
    if ((h = get_msgheader(data->hdrs, "Content-Transfer-Encoding")) != NULL)
    {
      if ((h->value != NULL) && strexpr(h->value, "base64", NULL, NULL, TRUE))
      {
        SET_BIT(data->flags.msg, SPAM_MSG_BASE64);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL, "%s SPAM CHECK - M%02d B64 encoded message",
                       id, SPAM_MSG_BASE64);
      }
    }

    /*
     ** Subject encoding subject
     */
    if ((h = get_msgheader(data->hdrs, "Subject")) != NULL)
    {
      if ((h->value != NULL) &&
          strexpr(h->value, "^=[?].*[?][bB][?].*[?]=", NULL, NULL, TRUE))
      {
        SET_BIT(data->flags.msg, SPAM_MSG_BASE64_SUBJECT);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL, "%s SPAM CHECK - M%02d B64 encoded Subject",
                       id, SPAM_MSG_BASE64_SUBJECT);
      }
    }
  }

  /*
   **
   ** Message body content checking
   **
   */

  /* N = ??? XXX */
  if (size < 8 * maxsize)
    decode_mime_file(id, fname, &mime_flags, check_mime_part, (void *) data);

  /*
   **
   ** ORACLE - post processing
   **
   */
  if (data->scores.do_oracle)
  {

    /* 
     ** message checking 
     */
    if ((data->nb_part > 1) && (data->nb_text == 0))
    {
      SET_BIT(data->flags.msg, SPAM_MSG_NO_TEXT_PART);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - M%02d No HTML nor TEXT parts : Total = %d",
                     id, SPAM_MSG_NO_TEXT_PART, data->nb_part);
    }

    /* compare sizes */
#if 1
    {
      if (kmean(&data->mksize.plain) < 1.)
        ;
    }
#endif

    if (data->msg_bad_expressions > 0)
    {
      SET_BIT(data->flags.msg, SPAM_MSG_BAD_EXPRESSIONS);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - M%02d BAD EXPRESSIONS : %d", id,
                     SPAM_MSG_BAD_EXPRESSIONS, data->msg_bad_expressions);
    }

    if ((data->nb_text_html > 0) && (data->nb_text_html > data->nb_text_plain))
    {
      SET_BIT(data->flags.msg, SPAM_MSG_TOO_MUCH_HTML);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - M%02d NB HTML > PLAIN : %d %d", id,
                     SPAM_MSG_TOO_MUCH_HTML, data->nb_text_html,
                     data->nb_text_plain);
    }

    /* correlate text/plain vs text/html parts */
    if (TRUE)
    {
      double              whtml[5], wplain[5];
      double              hhtml[5], hplain[5];
      double              vcoef, lcoef;

      memset(whtml, 0, sizeof (whtml));
      memset(hhtml, 0, sizeof (hhtml));
      memset(wplain, 0, sizeof (wplain));
      memset(hplain, 0, sizeof (hplain));
      if ((data->nb_text_html > 0) && (data->nb_text_plain > 0))
      {
        bool                ldiff = FALSE;
        bool                ko = FALSE;

        whtml[0] = kmin(&data->html.st_wlen);
        whtml[1] = kmax(&data->html.st_wlen);
        whtml[2] = kmean(&data->html.st_wlen);
        whtml[3] = kstddev(&data->html.st_wlen);
        whtml[4] = (double) data->html.len_clean;

        wplain[0] = kmin(&data->plain.st_wlen);
        wplain[1] = kmax(&data->plain.st_wlen);
        wplain[2] = kmean(&data->plain.st_wlen);
        wplain[3] = kstddev(&data->plain.st_wlen);
        wplain[4] = (double) data->plain.len_clean;

        vcoef = vector_compare(whtml, wplain, 5);

        if (wplain[4] >= 1.)
          lcoef = whtml[4] / wplain[4];
        else
          lcoef = 1000.;

        if (data->html.len_clean > 2500)
        {
          if (abs(data->html.len_clean - data->plain.len_clean) > 500)
            ldiff = TRUE;
        } else
        {
          if (abs(data->html.len_clean - data->plain.len_clean) > 1000)
            ldiff = TRUE;
        }
        ldiff = FALSE;

        if (!ko && (vcoef < 0.9) && (vcoef > 0.1))
        {
          ko = TRUE;

          SET_BIT(data->flags.msg, SPAM_MSG_MATCH_MIME_PARTS);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            ZE_MessageInfo(LOG_LEVEL,
                         "%s SPAM CHECK - M%02d HTML/PLAIN parts don't match vcoef=(%7.3f) lcoef=(%7.3f) (vcoef)",
                         id, SPAM_MSG_MATCH_MIME_PARTS, vcoef, lcoef);
        }

        if (!ko && ((lcoef > 5) || (lcoef < 0.85)))
        {
          ko = TRUE;

          SET_BIT(data->flags.msg, SPAM_MSG_MATCH_MIME_PARTS);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            ZE_MessageInfo(LOG_LEVEL,
                         "%s SPAM CHECK - M%02d HTML/PLAIN parts don't match vcoef=(%7.3f) lcoef=(%7.3f) (lcoef)",
                         id, SPAM_MSG_MATCH_MIME_PARTS, vcoef, lcoef);
        }

        if (!ko && ldiff)
        {
          ko = TRUE;
          /* JOE 2007 Aug 20 */
#if 1
          SET_BIT(data->flags.msg, SPAM_MSG_MATCH_MIME_PARTS);
#endif
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            ZE_MessageInfo(LOG_LEVEL,
                         "%s SPAM CHECK - M%02d HTML/PLAIN parts don't match HTML(%6d)/PLAIN(%6d) (ldiff)",
                         id, SPAM_MSG_MATCH_MIME_PARTS,
                         data->html.len_clean, data->plain.len_clean);
        }
#if _FFR_MSG_ENTROPY == 1
        hhtml[0] = data->html.h0;
        hhtml[1] = data->html.h1;
        hhtml[2] = data->html.h2;
        hhtml[3] = (double) data->html.len_clean;

        hplain[0] = data->plain.h0;
        hplain[1] = data->plain.h1;
        hplain[2] = data->plain.h2;
        hplain[3] = (double) data->plain.len_clean;

        vcoef = vector_compare(hhtml, hplain, 4);
#if 0
        if (wplain[4] >= 1.0)
          lcoef = whtml[3] / wplain[3];
        else
          lcoef = 1000.;
#endif
        /*  compare entropies... XXX JOE */
#endif
      }
    }

    /*
     ** MIME decode errors
     */
    data->mime_errors = count_uint32bits(mime_flags);
    if (data->mime_errors > 0)
    {
      int                 i;
      char                sout[64];

      SET_BIT(data->flags.msg, SPAM_MSG_MIME_ERRORS);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - M%02d MIME decode errors : %d",
                     id, SPAM_MSG_MIME_ERRORS, data->mime_errors);
      memset(sout, 0, sizeof (sout));

      for (i = 0; i < 32; i++)
        sout[i] = GET_BIT(mime_flags, i) ? ('0' + (i % 10)) : '.';

      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - M%02d MIME errors %s",
                     id, SPAM_MSG_MIME_ERRORS, sout);
    }

    /* 
     ** text/plain checking 
     */
    if (data->nb_text_plain_base64 > 0)
    {
      SET_BIT(data->flags.plain, SPAM_PLAIN_BASE64);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - P%02d text/plain encoded base64 : %d",
                     id, SPAM_PLAIN_BASE64, data->nb_text_plain_base64);
    }

    if ((data->nb_text_plain_empty > 0)
        && (data->nb_text_plain == data->nb_text_plain_empty))
    {
      SET_BIT(data->flags.plain, SPAM_PLAIN_EMPTY);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - P%02d text/plain empty : %d", id,
                     SPAM_PLAIN_EMPTY, data->nb_text_plain_empty);
    }

    /* 
     ** text/html checking 
     */
    if (data->nb_text_html_base64 > 0)
    {
      SET_BIT(data->flags.html, SPAM_HTML_BASE64);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - H%02d text/html  encoded base64 : %d",
                     id, SPAM_HTML_BASE64, data->nb_text_html_base64);
    }

    /* check cleaned up html part size */
    if (data->html.len_clean > 0)
    {
      bool                chk = FALSE;
      double              r2c = 1.;

      if (data->html.len_clean > 0)
        r2c = ((double) data->html.len_raw) / ((double) data->html.len_clean);

      if (data->html.len_clean > 0 && data->html.len_clean < 100)
      {
        if ((data->plain.len_clean == 0) || (r2c >= 2.))
        {
          chk = TRUE;
        }
      }

      if (chk)
      {
        SET_BIT(data->flags.html, SPAM_HTML_CLEAN_TOO_SHORT);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          ZE_MessageInfo(LOG_LEVEL,
                       "%s SPAM CHECK - H%02d HTML cleaned up too short : %d",
                       id, SPAM_HTML_CLEAN_TOO_SHORT, data->html.len_clean);
      }
    }

    if (data->html_high_tag_ratio > 0)
    {
      SET_BIT(data->flags.html, SPAM_HTML_TAGS_RATIO);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - H%02d HTML tag/text ratio : %d", id,
                     SPAM_HTML_TAGS_RATIO, data->html_high_tag_ratio);
    }

    if (data->html_unwanted_tags > 0)
    {
      SET_BIT(data->flags.html, SPAM_HTML_UNWANTED_TAGS);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - H%02d HTML with unwanted tags : %d", id,
                     SPAM_HTML_UNWANTED_TAGS, data->html_unwanted_tags);
    }

    if (data->html_invalid_tags > 0)
    {
      SET_BIT(data->flags.html, SPAM_HTML_INVALID_TAGS);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(LOG_LEVEL,
                     "%s SPAM CHECK - H%02d HTML with invalid tags : %d", id,
                     SPAM_HTML_INVALID_TAGS, data->html_invalid_tags);
    }

  }

  /*
   ** Evaluate Oracle Score
   */
  if (data->scores.do_oracle)
  {
    ZE_MessageInfo(12, "Avant oracle_compute_score");
    data->scores.oracle = oracle_compute_score(id, ip, data);
    ZE_MessageInfo(12, "Apres oracle_compute_score");
  }

  if (scores != NULL)
    *scores = data->scores;

  return data->scores.body + data->scores.urlbl;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
check_rfc2822_headers_count(id, head)
     char               *id;
     header_T           *head;
{
  int                 nerr = 0;
  int                 r;
  char               *s;
  int                 nb = 0;

  bool                mime_ct, mime_cd, mime_cte, mime_vers;

  mime_ct = mime_cd = mime_cte = mime_vers = FALSE;

  s = "Date";
  if ((r = count_msgheader_attr(head, s)) != 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (1,1)",
                   id, s, r);
    nerr++;
  }

  s = "From";
  if ((r = count_msgheader_attr(head, s)) != 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (1,1)",
                   id, s, r);
    nerr++;
  }

  s = "Sender";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  s = "Reply-To";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  s = "To";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  nb = r;

  s = "Cc";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  nb += r;

  s = "Bcc";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  nb += r;

  if (nb == 0)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : No To nor Cc nor Bcc",
                   id);
    nerr++;
  }

  s = "Message-ID";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  if (r == 0)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  s = "In-Reply-To";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  s = "References";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  s = "Subject";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  nb = r;
  if (nb == 0)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10, "%s SPAM CHECK - MSG RFC2822 HDRS count : No Subject",
                   id);
    nerr++;
  }

  s = "MIME-Version";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  mime_vers = (r > 0);

  s = "Content-Type";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  mime_ct = (r > 0);

  s = "Content-Disposition";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }
  mime_cd = (r > 0);

  s = "Content-Transfer-Encoding";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
#if 0
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
#endif
  }
  mime_cte = (r > 0);

  /* XXX - check mime coherence */
  if ((mime_ct && !mime_vers) || (mime_cd && !mime_vers)
      || (mime_cte && !mime_vers))
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2045 HDRS : MIME=(%d) CT=(%d) CD=(%d) CTE=(%d)",
                   id, mime_vers, mime_ct, mime_cd, mime_cte, r);
    nerr++;
  }

  s = "X-Mailer";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  s = "User-Agent";
  if ((r = count_msgheader_attr(head, s)) > 1)
  {
    if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
      ZE_MessageInfo(10,
                   "%s SPAM CHECK - MSG RFC2822 HDRS count : %-12s : %d (0,1)",
                   id, s, r);
    nerr++;
  }

  if (nerr > 5)
    nerr = 5;

  return nerr;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
check_rfc2822_headers_syntax(id, head)
     char               *id;
     header_T           *head;
{
  int                 nerr = 0;
  char               *s;
  header_T           *h = NULL;

  s = "Date";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    char               *p;
    long                pi, pf;
    bool                ok = TRUE;

    if (h->value == NULL)
      continue;

    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
      continue;
    }

    p = h->value;
    s = "(Mon|Tue|Wed|Thu|Fri|Sat|Sun)";
    if (strexpr(p, s, &pi, &pf, TRUE))
      p += pf;

    s =
      "[0-9]{1,2} (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec){1,1} [0-9]{4,4}";
    if (!strexpr(p, s, &pi, &pf, TRUE))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10,
                     "%s SPAM CHECK - MSG HDRS SYNTAX : Date : dd Mmm Yyyy : %s",
                     id, h->value);
      ok = FALSE;
    } else
      p += pf;

    s = "[0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2}";
    if (!strexpr(p, s, &pi, &pf, TRUE))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10,
                     "%s SPAM CHECK - MSG HDRS SYNTAX : Date : HH:MM:SS : %s",
                     id, h->value);
      ok = FALSE;
    } else
      p += pf;

#if 0
    if (!ok)
      nerr++;
#endif
  }

  s = "From";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "Sender";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
#if 0
      nerr++;
#endif
    }
  }

  s = "Reply-To";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
#if 0
      nerr++;
#endif
    }
  }

  s = "To";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "Cc";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
#if 0
      nerr++;
#endif
    }
  }

  s = "Bcc";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "Message-ID";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if (!strexpr(h->value, "<[^>@]+@[^>]+>", NULL, NULL, TRUE))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %-12s", id, s);
      nerr++;
    }
  }

  s = "In-Reply-To";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if (!strexpr(h->value, "<[^>@]+@[^>]+>", NULL, NULL, TRUE))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
#if 0
      nerr++;
#endif
    }
  }

  s = "References";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if (!strexpr(h->value, "<[^>@]+@[^>]+>", NULL, NULL, TRUE))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "Subject";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
#if 0
      nerr++;
#endif
    }
  }

  s = "MIME-Version";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if (strstr(h->value, "1.0") == NULL)
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %-12s : %s", id,
                     s, h->value);
      nerr++;
    }
  }

  s = "Content-Type";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "Content-Disposition";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "Content-Transfer-Encoding";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "X-Mailer";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  s = "User-Agent";
  h = head;
  while ((h = get_msgheader_next(h, s)) != NULL)
  {
    if (h->value == NULL)
      continue;
    if ((strlen(h->value) == 0)
        || (strspn(h->value, " \t") == strlen(h->value)))
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - MSG HDRS SYNTAX : %s empty", id, s);
      nerr++;
    }
  }

  if (nerr > 5)
    nerr = 5;

  return nerr;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              uint32_t
check_header_date(id, headers)
     char               *id;
     header_T           *headers;
{
  int                 nerr = 0;
  header_T           *h = headers;
  time_t              now = time(NULL);
  uint32_t            flags = 0;

  ZE_MessageInfo(11, "Checking date : %s", id);

  if (headers == NULL)
    return 0;

  while ((h = get_msgheader_next(h, "Date")) != NULL)
  {
    time_t              date_secs;

    if (h->value == NULL)
      continue;

    date_secs = header_date2secs(h->value);

    ZE_MessageInfo(11, "%s : Checking date : %ld %s", id, date_secs, h->value);
    if (date_secs == 0)
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - INVALID DATE : %s", id, h->value);
      SET_BIT(flags, SPAM_MSG_BAD_DATE);
      nerr++;
      continue;
    }

    if (date_secs > now && date_secs - now > 48 HOURS)
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - DATE IN THE FUTUR : %s", id,
                     h->value);
      nerr++;
      SET_BIT(flags, SPAM_MSG_FUTURE_DATE);
      continue;
    }
#if 0
    if (date_secs + 1 YEARS < now)
    {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(10, "%s SPAM CHECK - DATE IN THE PASR : %s", id, h->value);
      nerr++;
      SET_BIT(flags, SPAM_MSG_TOO_OLD_DATE);
      continue;
    }
#endif
  }

  return flags;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
count_html_tags(buf)
     char               *buf;
{
  char               *p;
  int                 res = 0;
  long                pos = 0;

  if (buf == NULL)
    return 0;

  p = buf;
  while ((strlen(p) > 0) && strexpr(p, "<[^>]{1,40}>", NULL, &pos, TRUE))
  {
    p += pos;
    res++;
  }
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
count_html_comments(buf)
     char               *buf;
{
  char               *p;
  int                 res = 0;
  long                pos = 0;

  if (buf == NULL)
    return 0;

  p = buf;
  while ((strlen(p) > 0) && strexpr(p, "<!--[^>]{0,40}-->", NULL, &pos, TRUE))
  {
    p += pos;
    res++;
  }
  return res;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
check_unwanted_html_tags(id, buf, best)
     char               *id;
     char               *buf;
     bestof_T           *best;
{
  int                 n = 0;
  double              odds = 0.;

  n = count_oradata(id, "HTML-TAG", buf, FALSE, &odds);
  if (n > 0)
    bestof_add(best, odds);

  ZE_MessageInfo(12, " BAD %s %7.3f", "HTML-TAG", odds);
  return MIN(n, 3);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
check_unwanted_boundary(id, boundary, best)
     char               *id;
     char               *boundary;
     bestof_T           *best;
{
  int                 n = 0;
  double              odds = 0.;

  n = count_oradata(id, "BOUNDARY", boundary, TRUE, &odds);
  if (n > 0)
    bestof_add(best, odds);

  ZE_MessageInfo(12, " BAD %s %7.3f", "BOUNDARY", odds);
  return (n > 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
check_unwanted_mailer(id, mailer, best)
     char               *id;
     char               *mailer;
     bestof_T           *best;
{
  int                 n = 0;
  double              odds = 0.;

  if (mailer == NULL)
    return FALSE;

  if (strlen(mailer) == 0)
    return TRUE;

  n = count_oradata(id, "MAILER", mailer, TRUE, &odds);
  if (n > 0)
    bestof_add(best, odds);

  ZE_MessageInfo(12, " BAD %s %7.3f", "MAILER  ", odds);
  return (n > 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define DCSET   256

static              bool
check_unwanted_charset(id, charset, best)
     char               *id;
     char               *charset;
     bestof_T           *best;
{
  int                 n = 0;
  double              odds = 0.;

  n = count_oradata(id, "CHARSET", charset, TRUE, &odds);
  if (n > 0)
    bestof_add(best, odds);

  ZE_MessageInfo(12, " BAD %s %7.3f", "CHARSET ", odds);
  return (n > 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
check_unwanted_expressions(id, buf, best)
     char               *id;
     char               *buf;
     bestof_T           *best;
{
  int                 n = 0;
  double              odds = 0.;

  n = count_oradata(id, "BAD-EXPR", buf, FALSE, &odds);
  if (n > 0)
    bestof_add(best, odds);

  ZE_MessageInfo(12, " BAD %s %7.3f", "BAD-EXPR", odds);

  return n;
}
