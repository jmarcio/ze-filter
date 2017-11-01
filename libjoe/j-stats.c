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

#include <ze-sys.h>

#include "ze-chkmail.h"

/* ****************************************************************************
 *                                                                            *
 *   SCORE STATS                                                              *
 **************************************************************************** */
#define MAX_SCORE  32
#define STATE_FILE        "j-state"

static struct
{
  int                 nb;
  pthread_mutex_t     mutex;
  int                 ora[MAX_SCORE + 2];
  int                 reg[MAX_SCORE + 2];
  int                 msg[MAX_SCORE + 2];
  int                 bay[MAX_SCORE + 2];
} msg_score =
{
0, PTHREAD_MUTEX_INITIALIZER};


#define SCORE_LOCK()       MUTEX_LOCK(&msg_score.mutex)
#define SCORE_UNLOCK()     MUTEX_UNLOCK(&msg_score.mutex)


void
msg_score_stats_update(scores)
     msg_scores_T       *scores;
{
  int                 msg_sc;
  int                 reg_sc;
  int                 ora_sc;
  double              bay_sc;

  if (scores == NULL)
    return;

  reg_sc = scores->body + scores->headers + scores->urlbl;
  ora_sc = scores->oracle;
  bay_sc = scores->bayes;
  msg_sc = reg_sc + ora_sc;

  SCORE_LOCK();

  if (msg_score.nb == 0)
  {
    memset(msg_score.ora, 0, sizeof (msg_score.ora));
    memset(msg_score.reg, 0, sizeof (msg_score.reg));
    memset(msg_score.msg, 0, sizeof (msg_score.msg));
    memset(msg_score.bay, 0, sizeof (msg_score.bay));
  }

  if (msg_sc >= 0 || bay_sc >= 0.75)
  {
    int                 x;

    /* *** JOE XXX */
    x = bay_sc * MAX_SCORE;
    if (x < 0)
      x = 0;

    if (bay_sc <= MAX_SCORE)
      msg_score.bay[x]++;
    else
      msg_score.bay[MAX_SCORE + 1]++;

    if (msg_sc <= MAX_SCORE)
      msg_score.msg[msg_sc]++;
    else
      msg_score.msg[MAX_SCORE + 1]++;

    if (ora_sc <= MAX_SCORE)
      msg_score.ora[ora_sc]++;
    else
      msg_score.ora[MAX_SCORE + 1]++;

    if (reg_sc <= MAX_SCORE)
      msg_score.reg[reg_sc]++;
    else
      msg_score.reg[MAX_SCORE + 1]++;

    msg_score.nb++;
  }

  SCORE_UNLOCK();
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
msg_score_stats_print(fd, which)
     int                 fd;
     int                 which;
{
  if (fd >= 0)
  {
    int                 i = 0, j;
    int                 cnt = 0;
    int                 max = 0;
    int                 sum = 0;
    char                sout[65];
    int                 v;

    SCORE_LOCK();

    for (i = 0; i < MAX_SCORE + 2; i++)
    {
      v = 0;
      switch (which)
      {
        case 0:
          v = msg_score.msg[i];
          break;
        case 1:
          v = msg_score.reg[i];
          break;
        case 2:
          v = msg_score.ora[i];
          break;
        case 3:
          v = msg_score.bay[i];
          break;
      }
      cnt += v;
      if (v > max)
        max = v;
    }

    if (cnt > 0)
    {
      FD_PRINTF(fd, "\nMESSAGES : %7d\n", cnt);

      FD_PRINTF(fd, "\n");
      sum = 0;
      for (i = 0; i < MAX_SCORE + 2; i++)
      {
        v = 0;
        switch (which)
        {
          case 0:
            v = msg_score.msg[i];
            break;
          case 1:
            v = msg_score.reg[i];
            break;
          case 2:
            v = msg_score.ora[i];
            break;
          case 3:
            v = msg_score.bay[i];
            break;
          default:
            v = 0;
            break;
        }

        sum += v;
        memset(sout, 0, sizeof (sout));
        memset(sout, ' ', 51);

        memset(sout, '.', (50 * sum) / cnt);
        for (j = 0; j < 51; j += 10)
          sout[j] = '|';

        sout[(50 * sum) / cnt] = 'X';
        FD_PRINTF(fd, "    %4d : %7d %s\n", i, v, sout);
      }
      FD_PRINTF(fd, "\n");
    }

    SCORE_UNLOCK();
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static j_stats_T    m_stats;

static pthread_mutex_t st_mutex = PTHREAD_MUTEX_INITIALIZER;

#define STATS_LOCK()       MUTEX_LOCK(&st_mutex)
#define STATS_UNLOCK()     MUTEX_UNLOCK(&st_mutex)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
fill_counters_buffer(buf, bufsz)
     char               *buf;
     size_t              bufsz;
{
  char                str[256];
  time_t              now = time(NULL);

  if (buf == NULL)
    return FALSE;

  memset(str, 0, sizeof (str));

  snprintf(str, sizeof (str), "%12lld ", (long long) now);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "RESTART=(%lld) ",
           (long long) m_stats.glob.value[STAT_RESTART]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "CONN=(%lld) ",
           (long long) m_stats.glob.value[STAT_CONNECT]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "ABRT=(%lld) ",
           (long long) m_stats.glob.value[STAT_ABORT]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "MSGS=(%lld) ",
           (long long) m_stats.glob.value[STAT_MSGS]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "KBYTES=(%lld) ",
           ((unsigned long long) (m_stats.glob.value[STAT_BYTES]) >> 4));
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "RCPT=(%lld) ",
           (long long) m_stats.glob.value[STAT_ENVTO]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "FILES=(%lld) ",
           (long long) m_stats.glob.value[STAT_FILES]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "XFILES=(%lld) ",
           (long long) m_stats.glob.value[STAT_XFILES]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "RCPTRATE=(%lld) ",
           (long long) m_stats.glob.value[STAT_RCPT_RATE]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "THROTTLE=(%lld) ",
           (long long) m_stats.glob.value[STAT_CONN_RATE]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "OPENCONN=(%lld) ",
           (long long) m_stats.glob.value[STAT_OPEN_CONN]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "BADRCPT=(%lld) ",
           (long long) m_stats.glob.value[STAT_BAD_RCPT]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "SPAMTRAP=(%lld) ",
           (long long) m_stats.glob.value[STAT_RCPT_SPAMTRAP]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "LOCALUSER=(%lld) ",
           (long long) m_stats.glob.value[STAT_LUSERS]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "RESFAIL=(%lld) ",
           (long long) m_stats.glob.value[STAT_RESOLVE_FAIL]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "RESFORG=(%lld) ",
           (long long) m_stats.glob.value[STAT_RESOLVE_FORGED]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "MATCHING=(%lld) ",
           (long long) m_stats.glob.value[STAT_PATTERN_MATCHING]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "ORACLE=(%lld) ",
           (long long) m_stats.glob.value[STAT_ORACLE]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "URLBL=(%lld) ",
           (long long) m_stats.glob.value[STAT_URLBL]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "BAYESSPAM=(%lld) ",
           (long long) m_stats.glob.value[STAT_BAYES_SPAM]);
  strlcat(buf, str, bufsz);
  snprintf(str, sizeof (str), "BAYESHAM=(%lld) ",
           (long long) m_stats.glob.value[STAT_BAYES_HAM]);
  strlcat(buf, str, bufsz);
  if (cf_get_int(CF_SCANNER_ACTION) != OPT_OK)
  {
    snprintf(str, sizeof (str), "VIRUS=(%lld) ",
             (long long) m_stats.glob.value[STAT_VIRUS]);
    strlcat(buf, str, bufsz);
    snprintf(str, sizeof (str), "POLICY=(%lld) ",
             (long long) m_stats.glob.value[STAT_POLICY]);
    strlcat(buf, str, bufsz);
  }
  if (m_stats.glob.value[STAT_BADMX] > 0)
  {
    snprintf(str, sizeof (str), "BADMX=(%lld) ",
             (long long) m_stats.glob.value[STAT_BADMX]);
    strlcat(buf, str, bufsz);
  }
  if (m_stats.glob.value[STAT_GREY_MSGS] > 0)
  {
    snprintf(str, sizeof (str), "GREYMSGS=(%lld) ",
             (long long) m_stats.glob.value[STAT_GREY_MSGS]);
    strlcat(buf, str, bufsz);
  }
  if (m_stats.glob.value[STAT_GREY_RCPT] > 0)
  {
    snprintf(str, sizeof (str), "GREYRCPT=(%lld) ",
             (long long) m_stats.glob.value[STAT_GREY_RCPT]);
    strlcat(buf, str, bufsz);
  }
  strlcat(buf, "\n", bufsz);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static LOG_T        st_log = LOG_INITIALIZER;

bool
log_counters_reopen()
{
  return log_reopen(&st_log);
}

void
log_counters(fd, dump)
     int                 fd;
     bool                dump;
{
  time_t              now = time(NULL);

  char               *fname = cf_get_str(CF_STATS_FILE);
  char                path[1024];
  char               *wkdir = NULL;

  if (fd >= 0)
  {
    char                buf[2048];

    memset(buf, 0, sizeof (buf));
    snprintf(buf, sizeof(buf), "DATA ");
    if (fill_counters_buffer(buf, sizeof(buf)))
      FD_PRINTF(fd, "%s", buf);

    return;
  }

  if (fd < 0)
  {
    wkdir = cf_get_str(CF_WORKDIR);
    if (wkdir == NULL || strlen(wkdir) == 0)
      wkdir = J_WORKDIR;

    fname = cf_get_str(CF_STATS_FILE);

    ADJUST_LOG_NAME(path, fname, wkdir, "none:");

    if (strlen(path) == 0)
    {
      LOG_MSG_ERROR("undefined state file");
      return;
    }

    if (log_check_and_open(&st_log, path))
    {
      char                buf[2048];

      memset(buf, 0, sizeof (buf));
      if (fill_counters_buffer(buf, sizeof(buf)))
        log_write(&st_log, buf);
    }
    return;
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
stats_reset()
{
  time_t              now = time(NULL);

  STATS_LOCK();

  memset(&m_stats, 0, sizeof (m_stats));
  strlcpy(m_stats.version, VERSION, sizeof (m_stats.version));

  m_stats.glob.start = now;
  m_stats.proc.start = now;

  STATS_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
stats_inc(which, n)
     int                 which;
     long                n;
{

  if (which < 0 || which > DIM_STATS)
    return;

  STATS_LOCK();

  switch (which)
  {
    case STAT_BYTES:
      m_stats.glob.value[STAT_BYTES] += (n >> 6);
      m_stats.proc.value[STAT_BYTES] += (n >> 6);
      break;
    default:
      m_stats.glob.value[which] += n;
      m_stats.proc.value[which] += n;
      break;
  }

  STATS_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
save_state()
{
  int                 fd;
  time_t              now = time(NULL);
  char               *fname = STATE_FILE;

  char                path[1024];
  char               *wkdir = NULL;

  wkdir = cf_get_str(CF_WORKDIR);
  if (wkdir == NULL || strlen(wkdir) == 0)
    wkdir = J_WORKDIR;

  ADJUST_FILENAME(path, fname, wkdir, STATE_FILE);

  if (strlen(path) == 0)
  {
    LOG_MSG_ERROR("undefined state file");
    return;
  }

  if ((fd = open(path, O_WRONLY | O_CREAT, 00644)) >= 0)
  {
    STATS_LOCK();
    m_stats.last_save = now;
    STATS_UNLOCK();

    if (write(fd, &m_stats, sizeof (m_stats)) != sizeof (m_stats))
      LOG_SYS_ERROR("error writing %s file", STRNULL(path, "NULL"));
    close(fd);
  } else
    LOG_SYS_ERROR("error opening %s file", STRNULL(path, "NULL"));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
read_state()
{
  int                 fd;

  char               *fname = STATE_FILE;

  char                path[1024];
  char               *wkdir = NULL;

  wkdir = cf_get_str(CF_WORKDIR);
  if (wkdir == NULL || strlen(wkdir) == 0)
    wkdir = J_WORKDIR;

  ADJUST_FILENAME(path, fname, wkdir, STATE_FILE);

  if (strlen(path) == 0)
  {
    LOG_MSG_ERROR("undefined state file");
    return;
  }

  if ((fd = open(path, O_RDONLY)) >= 0)
  {
    STATS_LOCK();
    if (read(fd, &m_stats, sizeof (m_stats)) != sizeof (m_stats))
      LOG_SYS_ERROR("error reading %s file", STRNULL(path, "NULL"));
    STATS_UNLOCK();
    close(fd);
  } else
    LOG_SYS_ERROR("error opening %s file", STRNULL(path, "NULL"));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
init_proc_state()
{
  time_t              now = time(NULL);

  read_state();
  /* save_state (); */

  STATS_LOCK();

  memset(&m_stats.proc, 0, sizeof (m_stats.proc));
  m_stats.proc.start = now;
  if (m_stats.glob.start == 0)
    m_stats.glob.start = now;

  STATS_UNLOCK();

  save_state();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
reset_state()
{
  time_t              now = time(NULL);

  STATS_LOCK();

  memset(&m_stats, 0, sizeof (m_stats));

  strlcpy(m_stats.version, VERSION, sizeof (m_stats.version));
  m_stats.glob.start = now;
  m_stats.proc.start = now;

  STATS_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
print_state(ofd)
     int                 ofd;
{
  int                 fd;
  j_stats_T           st;

  char               *fname = STATE_FILE;

  char                path[1024];
  char               *wkdir = NULL;

  wkdir = cf_get_str(CF_WORKDIR);
  if (wkdir == NULL || strlen(wkdir) == 0)
    wkdir = J_WORKDIR;

  ADJUST_FILENAME(path, fname, wkdir, STATE_FILE);

  if (strlen(path) == 0)
  {
    LOG_MSG_ERROR("undefined state file");
    return 1;
  }

  if (ofd < 0)
    ofd = STDOUT_FILENO;

  if ((fd = open(path, O_RDONLY)) >= 0)
  {
    if (read(fd, &st, sizeof (st)) == sizeof (st))
    {
      char                out[256];

      close(fd);

      FD_PRINTF(ofd, "%-30s : %s\n", "Version", PACKAGE);

#if 0
      cftime(out, NULL, &st.glob.start);
#endif
      FD_PRINTF(ofd, "%-30s : %s\n", "Start", out);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Start-up",
                st.glob.value[STAT_RESTART]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Messages", st.glob.value[STAT_MSGS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Connect", st.glob.value[STAT_CONNECT]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Abort", st.glob.value[STAT_ABORT]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Close", st.glob.value[STAT_CLOSE]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# ENV RCPT", st.glob.value[STAT_ENVTO]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Attached files",
                st.glob.value[STAT_FILES]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# X-Files", st.glob.value[STAT_XFILES]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Virus", st.glob.value[STAT_VIRUS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject Local Users",
                st.glob.value[STAT_LUSERS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject No RCPT Headers",
                st.glob.value[STAT_NO_TO_HEADERS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject No Senders ",
                st.glob.value[STAT_NO_FROM_HEADERS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject Exceed Max RCPT",
                st.glob.value[STAT_MAX_RCPT]);

      FD_PRINTF(ofd, "\n");
#if 0
      cftime(out, NULL, &st.proc.start);
#endif
      FD_PRINTF(ofd, "%-30s : %s\n", "Start", out);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Start-up",
                st.proc.value[STAT_RESTART]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Messages", st.proc.value[STAT_MSGS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Connect", st.proc.value[STAT_CONNECT]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Abort", st.proc.value[STAT_ABORT]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Close", st.proc.value[STAT_CLOSE]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# ENV RCPT", st.proc.value[STAT_ENVTO]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Attached files",
                st.proc.value[STAT_FILES]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# X-Files", st.proc.value[STAT_XFILES]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Virus", st.proc.value[STAT_VIRUS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject Local Users",
                st.proc.value[STAT_LUSERS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject No RCPT Headers",
                st.proc.value[STAT_NO_TO_HEADERS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject No Senders ",
                st.proc.value[STAT_NO_FROM_HEADERS]);
      FD_PRINTF(ofd, "%-30s : %ld\n", "# Reject Exceed Max RCPT",
                st.proc.value[STAT_MAX_RCPT]);
    } else
    {
      close(fd);
      FD_PRINTF(ofd, "Error reading %s file : %s\n", path, strerror(errno));
      return 1;
    }
  } else
  {
    FD_PRINTF(ofd, "Error opening %s file : %s\n", path, strerror(errno));
    return 1;
  }
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

typedef struct _code
{
  int                 c_val;
  int                 order;
  char               *c_name;
}
CODE;

static CODE         stat_names[] = {
  {
   STAT_RESTART,
   0,
   "Filter launches"},
  {
   STAT_BYTES,
   0,
   "KBytes transfered"},
  {
   STAT_MSGS,
   0,
   "Messages"},
  {
   STAT_CONNECT,
   0,
   "Connect"},
  {
   STAT_ABORT,
   0,
   "Abort"},
  {
   STAT_CLOSE,
   0,
   "Close"},
  {
   STAT_ENVTO,
   0,
   "Cumulative # RCPT"},
  {
   STAT_FILES,
   0,
   "Attached files"},
  {
   STAT_XFILES,
   0,
   "Attached unsafe files"},
  {
   STAT_VIRUS,
   0,
   "Attached virus"},
  {
   STAT_LUSERS,
   0,
   "Msgs sent to intranet users"},
  {
   STAT_NO_TO_HEADERS,
   0,
   "No To or CC headers"},
  {
   STAT_NO_FROM_HEADERS,
   0,
   "No From headers"},
  {
   STAT_NO_HEADERS,
   0,
   "No headers at all"},
  {
   STAT_RESOLVE_FAIL,
   0,
   "Gateways without DNS resolution"},
  {
   STAT_RESOLVE_FORGED,
   0,
   "Gateways with forged DNS names"},
  {
   STAT_MAX_RCPT,
   0,
   "Msgs exceeding allowed # of recipients"},
  {
   STAT_RCPT_RATE,
   0,
   "Hosts exceeding allowed recipient rate"},
  {
   STAT_CONN_RATE,
   0,
   "Hosts exceeding allowed connection rate"},
  {
   STAT_OPEN_CONN,
   0,
   "Hosts exceeding allowed open connections"},
  {
   STAT_EMPTY_CONN,
   0,
   "Hosts doing too much empty connections"},
  {
   STAT_BAD_RCPT,
   0,
   "Hosts harvesting"},
  {
   STAT_SUBJECT_CONTENTS,
   0,
   "Forbidden subject contents (regex)"},
  {
   STAT_PATTERN_MATCHING,
   0,
   "Forbiden body contents (regex)"},
  {
   STAT_BINARY,
   0,
   "Binary encoded message body"},
  {
   STAT_BASE64,
   0,
   "BASE 64 encoded message body"},
  {
   STAT_QUOTED_PRINTABLE,
   0,
   "Quoted-printable message body"},
  {
   STAT_POLICY,
   0,
   "Message violating site policy"},
  {
   STAT_BADMX,
   0,
   "Sender address has BAD MX"},
  {
   STAT_GREY_RCPT,
   0,
   "Recipient delayed by greylisting"},
  {
   STAT_GREY_MSGS,
   0,
   "Message delayed by greylisting"},
  {-1, 0, NULL}
};


char               *
stats_title(code)
     int                 code;
{
  CODE               *p = stat_names;

  if (code < 0)
    return NULL;

  while (p->c_name)
  {
    if (code == p->c_val)
      return p->c_name;
    p++;
  }
  return NULL;
}





/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
print_p_stats(ofd, st, title, c, all)
     int                 ofd;
     p_stats_T          *st;
     char               *title;
     int                 c;
     int                 all;
{
  char                out[256];
  char               *p;

  FD_PRINTF(ofd, "\n%s\n", title);
  strlcpy(out, ctime(&st->start), sizeof (out));

  if ((p = strchr(out, '\n')) != NULL)
    *p = '\0';
  FD_PRINTF(ofd, "%c %-30s : %s\n", c, "Start", out);
  /* FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Start-up", st->value[STAT_RESTART]); */
  FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Messages", st->value[STAT_MSGS]);
  FD_PRINTF(ofd, "%c %-30s : %10lu\n", c, "# KBytes",
            ((unsigned long) (st->value[STAT_BYTES]) >> 4));
  FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Connect", st->value[STAT_CONNECT]);
  FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Abort", st->value[STAT_ABORT]);
  FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Close", st->value[STAT_CLOSE]);
  if (all || st->value[STAT_ENVTO] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# ENV RCPT",
              st->value[STAT_ENVTO]);
  if (all || st->value[STAT_FILES] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Attached files",
              st->value[STAT_FILES]);
  if (all || st->value[STAT_XFILES] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# X-Files",
              st->value[STAT_XFILES]);
  if (all || st->value[STAT_VIRUS] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Virus", st->value[STAT_VIRUS]);
  if (all || st->value[STAT_LUSERS] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Reject Local Users",
              st->value[STAT_LUSERS]);
  if (all || st->value[STAT_NO_TO_HEADERS] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Reject No RCPT Headers",
              st->value[STAT_NO_TO_HEADERS]);
  if (all || st->value[STAT_NO_FROM_HEADERS] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Reject No Senders ",
              st->value[STAT_NO_FROM_HEADERS]);
  if (all || st->value[STAT_MAX_RCPT] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Reject Exceed Max RCPT",
              st->value[STAT_MAX_RCPT]);
  if (all || st->value[STAT_RCPT_RATE] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Reject Exceed rcpt rate",
              st->value[STAT_RCPT_RATE]);
  if (all || st->value[STAT_CONN_RATE] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Reject Exceed conn rate",
              st->value[STAT_CONN_RATE]);
  if (all || st->value[STAT_RESOLVE_FAIL] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Resolve FAIL",
              st->value[STAT_RESOLVE_FAIL]);
  if (all || st->value[STAT_RESOLVE_FORGED] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Resolve FORGED",
              st->value[STAT_RESOLVE_FORGED]);
  if (all || st->value[STAT_BADMX] > 0)
    FD_PRINTF(ofd, "%c %-30s : %10ld\n", c, "# Sender with bad MX",
              st->value[STAT_BADMX]);

}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
print_p_stats_all(ofd, sta, titlea, stb, titleb, all, nf)
     int                 ofd;
     p_stats_T          *sta;
     char               *titlea;
     p_stats_T          *stb;
     char               *titleb;
     int                 all;
     bool                nf;
{
  char                out[256];
  char               *p;
  int                 i;

  strlcpy(out, ctime(&sta->start), sizeof out);
  if ((p = strchr(out, '\n')) != NULL)
    *p = '\0';
  FD_PRINTF(ofd, "\n%-30s %s\n", titlea, out);

  strlcpy(out, ctime(&stb->start), sizeof out);
  if ((p = strchr(out, '\n')) != NULL)
    *p = '\0';
  FD_PRINTF(ofd, "%-30s %s\n", titleb, out);

  FD_PRINTF(ofd, "%-40s   : %10ld  %10ld\n",
            "# Start-up", sta->value[STAT_RESTART], stb->value[STAT_RESTART]);
  FD_PRINTF(ofd, "%-40s   : %10ld  %10ld\n",
            "# Messages", sta->value[STAT_MSGS], stb->value[STAT_MSGS]);
  FD_PRINTF(ofd, "%-40s   : %10lu  %10lu\n",
            "# KBytes",
            (unsigned long) sta->value[STAT_BYTES] >> 4,
            (unsigned long) stb->value[STAT_BYTES] >> 4);
  FD_PRINTF(ofd, "%-40s   : %10ld  %10ld\n",
            "# Connect", sta->value[STAT_CONNECT], stb->value[STAT_CONNECT]);

  if (nf)
  {
    for (i = STAT_ABORT; i < DIM_STATS; i++)
    {
      if (sta->value[i] > 0 || stb->value[i] > 0)
      {
        char               *p = stats_title(i);

        if (p != NULL)
          FD_PRINTF(ofd, "* %-40s : %10ld  %10ld\n", p, sta->value[i],
                    stb->value[i]);
      }
    }
  } else
  {
    FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
              "Abort", sta->value[STAT_ABORT], stb->value[STAT_ABORT]);
    FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
              "Close", sta->value[STAT_CLOSE], stb->value[STAT_CLOSE]);

    if (sta->value[STAT_ENVTO] > 0 || stb->value[STAT_ENVTO] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "ENV RCPT", sta->value[STAT_ENVTO], stb->value[STAT_ENVTO]);

    if (all || sta->value[STAT_FILES] > 0 || stb->value[STAT_FILES] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Attached files", sta->value[STAT_FILES],
                stb->value[STAT_FILES]);
    if (all || sta->value[STAT_XFILES] > 0 || stb->value[STAT_XFILES] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "X-Files", sta->value[STAT_XFILES], stb->value[STAT_XFILES]);
    if (all || sta->value[STAT_VIRUS] > 0 || stb->value[STAT_VIRUS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Virus", sta->value[STAT_VIRUS], stb->value[STAT_VIRUS]);
    if (all || sta->value[STAT_LUSERS] > 0 || stb->value[STAT_LUSERS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject Local Users", sta->value[STAT_LUSERS],
                stb->value[STAT_LUSERS]);
    if (all || sta->value[STAT_MAX_RCPT] > 0 || stb->value[STAT_MAX_RCPT] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject Exceed Max RCPT",
                sta->value[STAT_MAX_RCPT], stb->value[STAT_MAX_RCPT]);
    if (all || sta->value[STAT_RCPT_RATE] > 0 || stb->value[STAT_RCPT_RATE] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject Exceed Throttle",
                sta->value[STAT_RCPT_RATE], stb->value[STAT_RCPT_RATE]);
    if (all || sta->value[STAT_CONN_RATE] > 0 || stb->value[STAT_CONN_RATE] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject Exceed Conn Rate",
                sta->value[STAT_CONN_RATE], stb->value[STAT_CONN_RATE]);
    if (all || sta->value[STAT_BADMX] > 0 || stb->value[STAT_BADMX] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Sender has bad MX", sta->value[STAT_BADMX],
                stb->value[STAT_BADMX]);
    if (all || sta->value[STAT_SINGLE_MESSAGE] > 0
        || stb->value[STAT_SINGLE_MESSAGE] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Message per connection limit",
                sta->value[STAT_SINGLE_MESSAGE],
                stb->value[STAT_SINGLE_MESSAGE]);

    if (all || sta->value[STAT_RCPT_REJECT] > 0
        || stb->value[STAT_RCPT_REJECT] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Recipient - - REJECT",
                sta->value[STAT_RCPT_REJECT], stb->value[STAT_RCPT_REJECT]);

    if (all || sta->value[STAT_RCPT_ACCESS] > 0
        || stb->value[STAT_RCPT_ACCESS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Recipient - ACCESS DENIED",
                sta->value[STAT_RCPT_ACCESS], stb->value[STAT_RCPT_ACCESS]);

    if (all || sta->value[STAT_RCPT_BAD_NETWORK] > 0
        || stb->value[STAT_RCPT_BAD_NETWORK] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Recipient - BAD_NETWORK",
                sta->value[STAT_RCPT_BAD_NETWORK],
                stb->value[STAT_RCPT_BAD_NETWORK]);

    if (all || sta->value[STAT_RCPT_UNKNOWN] > 0
        || stb->value[STAT_RCPT_UNKNOWN] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Recipient - USER UNKNOWN",
                sta->value[STAT_RCPT_UNKNOWN], stb->value[STAT_RCPT_UNKNOWN]);

    if (all || sta->value[STAT_RCPT_SPAMTRAP] > 0
        || stb->value[STAT_RCPT_SPAMTRAP] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Recipient - SPAMTRAP",
                sta->value[STAT_RCPT_SPAMTRAP], stb->value[STAT_RCPT_SPAMTRAP]);

    if (all || sta->value[STAT_BAD_RCPT] > 0 || stb->value[STAT_BAD_RCPT] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Connections from harvesters",
                sta->value[STAT_BAD_RCPT], stb->value[STAT_BAD_RCPT]);

    if (all || sta->value[STAT_GREY_RCPT] > 0 || stb->value[STAT_GREY_RCPT] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Recipients delayed by greylisting",
                sta->value[STAT_GREY_RCPT], stb->value[STAT_GREY_RCPT]);
    if (all || sta->value[STAT_GREY_MSGS] > 0 || stb->value[STAT_GREY_MSGS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Messages delayed by greylisting",
                sta->value[STAT_GREY_MSGS], stb->value[STAT_GREY_MSGS]);
    if (all || sta->value[STAT_RESOLVE_FAIL] > 0
        || stb->value[STAT_RESOLVE_FAIL] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Resolve FAIL",
                sta->value[STAT_RESOLVE_FAIL], stb->value[STAT_RESOLVE_FAIL]);
    if (all || sta->value[STAT_RESOLVE_FORGED] > 0
        || stb->value[STAT_RESOLVE_FORGED] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Resolve FORGED",
                sta->value[STAT_RESOLVE_FORGED],
                stb->value[STAT_RESOLVE_FORGED]);
    if (all || sta->value[STAT_NO_TO_HEADERS] > 0
        || stb->value[STAT_NO_TO_HEADERS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Reject No RCPT Headers",
                sta->value[STAT_NO_TO_HEADERS], stb->value[STAT_NO_TO_HEADERS]);
    if (all || sta->value[STAT_NO_FROM_HEADERS] > 0
        || stb->value[STAT_NO_FROM_HEADERS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Reject No Senders ",
                sta->value[STAT_NO_FROM_HEADERS],
                stb->value[STAT_NO_FROM_HEADERS]);
    if (all || sta->value[STAT_PATTERN_MATCHING] > 0
        || stb->value[STAT_PATTERN_MATCHING] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Reject pattern matching",
                sta->value[STAT_PATTERN_MATCHING],
                stb->value[STAT_PATTERN_MATCHING]);
    if (all || sta->value[STAT_URLBL] > 0 || stb->value[STAT_URLBL] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Reject URL BL",
                sta->value[STAT_URLBL], stb->value[STAT_URLBL]);
    if (all || sta->value[STAT_ORACLE] > 0 || stb->value[STAT_ORACLE] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Detected by heuristic filter",
                sta->value[STAT_ORACLE], stb->value[STAT_ORACLE]);
    if (all || sta->value[STAT_BAYES_SPAM] > 0
        || stb->value[STAT_BAYES_SPAM] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Detected by bayes filter",
                sta->value[STAT_BAYES_SPAM], stb->value[STAT_BAYES_SPAM]);
    if (all || sta->value[STAT_SUBJECT_CONTENTS] > 0
        || stb->value[STAT_SUBJECT_CONTENTS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Reject subject contents",
                sta->value[STAT_SUBJECT_CONTENTS],
                stb->value[STAT_SUBJECT_CONTENTS]);
    if (all || sta->value[STAT_HEADERS_CONTENTS] > 0
        || stb->value[STAT_HEADERS_CONTENTS] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n", "Reject headers contents",
                sta->value[STAT_HEADERS_CONTENTS],
                stb->value[STAT_HEADERS_CONTENTS]);
    if (all || sta->value[STAT_BINARY] > 0 || stb->value[STAT_BINARY] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject binary encoded body",
                sta->value[STAT_BINARY], stb->value[STAT_BINARY]);
    if (all || sta->value[STAT_BASE64] > 0 || stb->value[STAT_BASE64] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject base 64 encoded body",
                sta->value[STAT_BASE64], stb->value[STAT_BASE64]);
    if (all || sta->value[STAT_QUOTED_PRINTABLE] > 0
        || stb->value[STAT_QUOTED_PRINTABLE] > 0)
      FD_PRINTF(ofd, "# %-40s : %10ld  %10ld\n",
                "Reject quoted printable encoded body",
                sta->value[STAT_QUOTED_PRINTABLE],
                stb->value[STAT_QUOTED_PRINTABLE]);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
dump_state(ofd, jp, jg, all, nf)
     int                 ofd;
     int                 jp;
     int                 jg;
     int                 all;
     bool                nf;
{
  int                 fd;
  j_stats_T           st;
  char               *p;

  char               *fname = STATE_FILE;

  char                path[1024];
  char               *wkdir = NULL;

  wkdir = cf_get_str(CF_WORKDIR);
  if (wkdir == NULL || strlen(wkdir) == 0)
    wkdir = J_WORKDIR;

  ADJUST_FILENAME(path, fname, wkdir, STATE_FILE);

  if (strlen(path) == 0)
  {
    LOG_MSG_ERROR("undefined state file");
    return 1;
  }

  if (ofd < 0)
    ofd = STDOUT_FILENO;

  if ((fd = open(path, O_RDONLY)) >= 0)
  {
    if (read(fd, &st, sizeof (st)) == sizeof (st))
    {
      char                out[256];

      close(fd);

      FD_PRINTF(ofd, "%-30s : %s\n", "Version", PACKAGE);

      strlcpy(out, ctime(&st.last_save), sizeof (out));
      if ((p = strchr(out, '\n')) != NULL)
        *p = '\0';
      FD_PRINTF(ofd, "%-30s : %s\n", "Last counters dump", out);

      if (!jp && !jg)
        print_p_stats(ofd, &st.glob, "*** GLOBAL DATA ***\n", 'G', all);

      if (!jp && jg)
        print_p_stats(ofd, &st.glob, "*** GLOBAL DATA ***\n", 'G', all);

      if (jp && !jg)
        print_p_stats(ofd, &st.proc, "*** PROCESSUS DATA ***\n", 'P', all);

      if (jp && jg)
        print_p_stats_all(ofd, &st.proc,
                          "*** PROCESSUS DATA ***",
                          &st.glob, "*** GLOBAL DATA ***", all, nf);
    } else
    {
      close(fd);
      LOG_SYS_ERROR("Error reading %s file", path);
      return 1;
    }
  } else
  {
    LOG_SYS_ERROR("Error opening %s file", path);
    return 1;
  }
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
print_filter_stats_summary()
{

  /* XXX Joe */

  char                out[256];
  char               *p;
  char               *title = "*** PROCESSUS DATA ***";
  p_stats_T          *st;
  char                c = ' ';

  st = &m_stats.proc;

  MESSAGE_INFO(9, "%s", title);
  strlcpy(out, ctime(&st->start), sizeof (out));

  if ((p = strchr(out, '\n')) != NULL)
    *p = '\0';
  MESSAGE_INFO(9, "%c %-30s : %s", c, "Start", out);
  /* MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Start-up", st->value[STAT_RESTART]); */
  MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Messages", st->value[STAT_MSGS]);
  MESSAGE_INFO(9, "%c %-30s : %10lu", c, "# KBytes",
               ((unsigned long) (st->value[STAT_BYTES]) >> 4));
  MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Connect", st->value[STAT_CONNECT]);
  MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Abort", st->value[STAT_ABORT]);
  MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Close", st->value[STAT_CLOSE]);
  if (st->value[STAT_ENVTO] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# ENV RCPT", st->value[STAT_ENVTO]);
  if (st->value[STAT_FILES] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Attached files",
                 st->value[STAT_FILES]);
  if (st->value[STAT_XFILES] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# X-Files", st->value[STAT_XFILES]);
  if (st->value[STAT_VIRUS] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Virus", st->value[STAT_VIRUS]);
  if (st->value[STAT_LUSERS] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Reject Local Users",
                 st->value[STAT_LUSERS]);
  if (st->value[STAT_NO_TO_HEADERS] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Reject No RCPT Headers",
                 st->value[STAT_NO_TO_HEADERS]);
  if (st->value[STAT_NO_FROM_HEADERS] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Reject No Senders ",
                 st->value[STAT_NO_FROM_HEADERS]);
  if (st->value[STAT_MAX_RCPT] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Reject Exceed Max RCPT",
                 st->value[STAT_MAX_RCPT]);
  if (st->value[STAT_RCPT_RATE] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Reject Exceed rcpt rate",
                 st->value[STAT_RCPT_RATE]);
  if (st->value[STAT_CONN_RATE] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Reject Exceed conn rate",
                 st->value[STAT_CONN_RATE]);
  if (st->value[STAT_RESOLVE_FAIL] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Resolve FAIL",
                 st->value[STAT_RESOLVE_FAIL]);
  if (st->value[STAT_RESOLVE_FORGED] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Resolve FORGED",
                 st->value[STAT_RESOLVE_FORGED]);
  if (st->value[STAT_BADMX] > 0)
    MESSAGE_INFO(9, "%c %-30s : %10ld", c, "# Sender with bad MX",
                 st->value[STAT_BADMX]);

}
