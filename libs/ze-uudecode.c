
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

#include <ze-sys.h>

#include "ze-libjc.h"

#include "ze-uudecode.h"

#define DBG_LEVEL     30

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define   RE_BEG "^begin(-base64){0,1}[ \t]{1,}[0]{0,1}[0-7]{3,3}[ \t]{1,}[^ \t\r\n]{1,}"
#define   RE_END "^end"

#define   REGCOMP_FLAGS  (REG_ICASE | REG_NEWLINE | REG_EXTENDED)

#if 0
#define   REGEXEC_FLAGS  (REG_NOTBOL | REG_NOTEOL)
#else
#define   REGEXEC_FLAGS  (0)
#endif

typedef struct {
  uint8_t             sz;
  char                buf[64];
} UUBUF_T;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
is_uu_line(b)
     char               *b;
{
  char               *p;
  int                 ld, lc;

  if (b == NULL)
    return FALSE;

  if ((*b < 0x20) || (*b > 0x5F))
    return FALSE;

  ld = *b - 0x20;
  while (ld % 3 != 0)
    ld++;
  lc = (4 * ld) / 3;

  for (p = b; *p != 0; p++)
    if ((*p < 0x20) || (*p > 0x60)) {
      printf("  CHAR = %c %d\n", *p, *p);
      return FALSE;
    }

  if (strlen(b) != lc + 1)
    return FALSE;

#if CHECK_UU_LINE_LENGTH
  if (ld > 45)
    return FALSE;
#endif

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
uu_line_decode(b, line)
     UUBUF_T            *b;
     char               *line;
{
  int                 ld, lc;
  char               *pi, *po;

  if ((line == NULL) || !is_uu_line(line))
    return FALSE;

  ld = *line - 0x20;
  b->sz = ld;

  while (ld % 3 != 0)
    ld++;
  lc = (4 * ld) / 3;

  pi = line + 1;
  po = b->buf;

  ZE_MessageInfo(DBG_LEVEL, "LINE : %s", pi);

  for (; lc > 0; lc -= 4) {
    pi[0] -= 0x20;
    pi[1] -= 0x20;
    pi[2] -= 0x20;
    pi[3] -= 0x20;

    po[0] = ((pi[0] << 2) & 0xFC) | ((pi[1] >> 4) & 0x03);
    po[1] = ((pi[1] << 4) & 0xF0) | ((pi[2] >> 2) & 0x0F);
    po[2] = ((pi[2] << 6) & 0xC0) | ((pi[3] >> 0) & 0x3F);

    pi += 4;
    po += 3;
    *po = '\0';
  }
  b->buf[ld] = '\0';

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
uudecode_buffer(bufin, uublk)
     char               *bufin;
     UU_BLOCK_T         *uublk;
{
  regex_t             re_beg, re_end;
  bool                ok = TRUE;
  size_t              sz_out = 0;
  int                 r;
  char               *bufout = NULL;
  mode_t              mode = 0;
  char               *name = NULL;

  if ((r = regcomp(&re_beg, RE_BEG, REGCOMP_FLAGS)) != 0) {
    char                sout[256];

    regerror(r, &re_beg, sout, sizeof (sout));
    ZE_LogMsgError(0, "regcomp error : %s", sout);
    ok = FALSE;
  }
  if ((r = regcomp(&re_end, RE_END, REGCOMP_FLAGS)) != 0) {
    char                sout[256];

    regerror(r, &re_end, sout, sizeof (sout));
    ZE_LogMsgError(0, "regcomp error : %s", sout);
    ok = FALSE;
  }
  if (ok) {
    regmatch_t          rm_beg, rm_end;

    ok = TRUE;

    if (regexec(&re_beg, bufin, 1, &rm_beg, REGEXEC_FLAGS) == 0) {
      ZE_MessageInfo(DBG_LEVEL, "BEGIN FOUND : %4d %4d", rm_beg.rm_so,
                     rm_beg.rm_eo);
    } else
      ok = FALSE;

    if (regexec(&re_end, bufin, 1, &rm_end, REGEXEC_FLAGS) == 0) {
      ZE_MessageInfo(DBG_LEVEL, "END   FOUND : %4d %4d", rm_end.rm_so,
                     rm_end.rm_eo);
    } else
      ok = FALSE;

    if (ok && (rm_end.rm_so > rm_beg.rm_eo)) {
      char               *pin = NULL;
      size_t              sz = rm_end.rm_so - rm_beg.rm_eo;
      char                line[1024];

      sz = rm_beg.rm_eo - rm_beg.rm_so;
      if ((sz > 0) && (sz < sizeof (line))) {
        char               *p;
        size_t              n;

        strncpy(line, bufin + rm_beg.rm_so, sz);
        line[sz] = '\0';

        p = line + strcspn(line, " \t\r\n");
        p += strspn(p, " \t\r\n");
        n = strcspn(p, " \t\r\n");
        if ((n > 0) && (n == strspn(p, "01234567"))) {
          char                s[8];
          long                l = 0644;

          memset(s, 0, sizeof (s));
          if (n < sizeof (s))
            strncpy(s, p, n);
          errno = 0;
          l = strtol(s, (char **) NULL, 8);
          if ((errno == EINVAL) || (errno == ERANGE)) {
            ZE_LogSysError("strtol : getting file mode");
          } else
            mode = l;
        }
        p += n;
        p += strspn(p, " \t\r\n");
        n = strcspn(p, " \t\r\n");
        p[n] = 0;
        if (n > 0) {
          if ((name = strdup(p)) == NULL)
            ZE_LogSysError("strdup : getting file name");
        }
      }

      sz = rm_end.rm_so - rm_beg.rm_eo;
      pin = (char *) malloc(sz + 1);
      bufout = (char *) malloc(sz + 1);

      if ((pin != NULL) && (bufout != NULL)) {
        char               *p, *q;
        int                 nbad = 0;

        memset(pin, 0, sz + 1);
        strncpy(pin, bufin + rm_beg.rm_eo, sz);

        p = pin + strspn(pin, "\n\r");
        q = bufout;

        while (*p != '\0') {
          char                line[1024];
          UUBUF_T             uu;

          p = buf_get_next_line(line, p, sizeof (line));
          if (is_uu_line(line)) {
            memset(&uu, 0, sizeof (uu));
            if (uu_line_decode(&uu, line)) {
              memcpy(q, uu.buf, uu.sz);
              q += uu.sz;
              sz_out += uu.sz;
            }
          } else {
            nbad++;
            if (nbad > 1) {
              ZE_LogMsgError(0, "strange : more than one bad uu line");
              break;
            }
          }
        }
        *q = '\0';
      } else {
        ZE_LogSysError("malloc buffer uuencoded");
        if (bufout != NULL)
          free(bufout);
        bufout = NULL;
      }
      if (pin != NULL)
        free(pin);
    }
  }
  regfree(&re_beg);
  regfree(&re_end);

  free_uu_block(uublk);
  if (bufout != NULL) {
    uublk->signature = SIGNATURE;
    uublk->buf = bufout;
    uublk->size = sz_out;
    uublk->name = name;
    uublk->mode = mode;
  }

  return (bufout != NULL);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
uudecode_file(fname, uublk)
     char               *fname;
     UU_BLOCK_T         *uublk;
{
  void               *bufin = NULL;
  size_t              sz_in = 0;
  bool                ok = FALSE;

  if ((fname == NULL) || (strlen(fname) == 0)) {
    ZE_LogMsgError(0, "fname NULL or empty string");
    return FALSE;
  }

  bufin = read_text_file(fname, &sz_in);

  if (bufin != NULL)
    ok = uudecode_buffer(bufin, uublk);

  if (bufin != NULL)
    free(bufin);

  return ok;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
free_uu_block(uublk)
     UU_BLOCK_T         *uublk;
{
  if (uublk == NULL)
    return;

  if (uublk->buf != NULL)
    free(uublk->buf);

  if (uublk->name != NULL)
    free(uublk->name);

  memset(uublk, 0, sizeof (UU_BLOCK_T));
}
