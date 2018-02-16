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


#ifndef __ZE_CHKCONTENT_H__

typedef struct
{
  size_t              len_raw;
  size_t              len_clean;
  kstats_T            st_wlen;
} msgpart_T;

typedef struct
{
  uint32_t            conn;
  uint32_t            ehlo;
  uint32_t            html;
  uint32_t            plain;
  uint32_t            mime;
  uint32_t            msg;
} msg_flags_T;

typedef struct
{
  kstats_T            plain;
  kstats_T            plain_clean;
  kstats_T            html;
  kstats_T            html_clean;
  kstats_T            simple;
  kstats_T            other;
  kstats_T            attach;
} msg_ksizes_T;

struct spamchk_T
{
  char               *ip;
  CONNID_T            id;

  size_t              spool_size;
  size_t              max_spool_size;

  int                 action;

  int                 content_max_score;

  /* message flags */
  msg_flags_T         flags;
  bestof_T            best;

  /* message scores */
  msg_scores_T        scores;

  /* measured data */
  int                 nb_badrcpt;
  int                 nb_part;
  int                 nb_text;
  int                 nb_image;
  int                 nb_audio;
  int                 nb_video;
  int                 nb_application;
  int                 nb_token;
  int                 nb_message;
  int                 nb_multipart;

  int                 nb_text_plain;
  int                 nb_text_html;
  int                 nb_text_simple;

  int                 nb_text_plain_base64;
  int                 nb_text_html_base64;

  size_t              size;
  size_t              sz_text_plain;
  size_t              sz_text_html;

  /* comparison html vs plain */
  msgpart_T           html;
  msgpart_T           plain;
  int                 nb_diff_html_plain;

  msg_ksizes_T        mksize;

  int                 mime_errors;
  int                 headers_syntax_errors;

  /* results */
  int                 nb_rfc2822_hdrs_errors;
  int                 nb_text_plain_empty;
  int                 nb_text_simple_empty;
  int                 html_unwanted_tags;
  int                 html_invalid_tags;
  int                 msg_bad_expressions;
  int                 html_high_tag_ratio;

  header_T           *hdrs;
};

int                 scan_body_contents(char *, char *, char *, size_t,
                                       spamchk_T *, msg_flags_T *,
                                       msg_scores_T *);

#define __ZE_CHKCONTENT_H__
#endif
