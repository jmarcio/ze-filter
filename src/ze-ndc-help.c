/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Sun Mar  2 11:51:21 CET 2008
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
#include <ze-filter.h>
#include <ze-ndc-help.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static bool         show_help(int fd, char *hdr, char *str);

typedef struct
{
  char               *cmd;
  char               *desc;
  char               *str;
} ndc_help_T;

static ndc_help_T   helps[] = {
  {
   "help",
   NULL,
   "  HELP\r\n" "    * We can help you\r\n" "    * Commands :\r\n"},

  {
   "version",
   NULL,
   "  VERSION\r\n"
   "    * Show filter version\r\n" "    * Syntax :\r\n" "      ze-ndc VERSION\r\n"},

  {
   "setcf",
   NULL,
   "  SETCF\r\n"
   "    * Modify running configuration (overrides ze-filter values)\r\n"
   "    * Syntax :\r\n" "      ze-ndc SETCF option value\r\n"},

  {
   "dumpcf",
   NULL,
   "  DUMPCF\r\n"
   "    * Dumps running configuration\r\n"
   "    * Syntax :\r\n" "      ze-ndc DUMPCF option\r\n"
   "          option is one of :\r\n"
   "            default\r\n"
   "            running\r\n"
   "            short\r\n"
  },

  {
   "setoracle",
   NULL,
   "  SETORACLE\r\n"
   "    * Set heuristic filter (oracle) checks scores\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc SETORACLE Tnn value\r\n"
   "        Where T is one of :\r\n"
   "           R : RBL Check\r\n"
   "           C : Connection check\r\n"
   "           M : Message check\r\n"
   "           H : HTML mime part check\r\n"
   "           P : Plain text mime part check\r\n"},

  {
   "set",
   NULL,
   "  SET\r\n"
   "    * Set the value of some internal variables\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc SET variable value\r\n"
   "        Where variable can be one of :\r\n"
   "          LOGLEVEL\r\n"
   "          LOG_SM_MACROS\r\n" "          MXCHECKLEVEL\r\n"
#if 0
   "          TLONGCONN\r\n"
   "          URLBLTIME\r\n"
   "          GREYDELAYS\r\n"
   "          GREYLIFETIME\r\n"
   "          GREYPENDING\r\n"
   "          GREYTUPLE\r\n"
   "          GREYCLEANUP\r\n" "          GREY_DEWHITE_THRESHOLD\r\n"
#endif
   },

  {
   "bayes",
   NULL,
   "  BAYES\r\n"
   "    * Reopen bayes filter database\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc BAYES command parameter\r\n"
   "        Where command can be one of :\r\n" "          REOPEN\r\n"},

#if 0
  {
   "greydelete",
   NULL,
   "    *" "    * Syntax :\r\n" "      ze-ndc \r\n"},
#endif

  {
   "show",
   NULL,
   "  SHOW\r\n"
   "    * Show internals\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc SHOW select\r\n"
   "        Where select can be one of :\r\n"
   "          RUN        : Running configuration\r\n"},

  {
   "stats",
   NULL,
   "  STATS\r\n"
   "    * Show internal statistics for this filter instance\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc STATS select\r\n"
   "        Where select can be one of :\r\n"
   "          ORACLE         : heuristic filter counters for this process\r\n"
   "          THROTTLE       : show global server rates\r\n"
   "          SMTPRATE       : show rates data for each client\r\n"
   "          CONNOPEN       : show open connections\r\n"
   "          HTIMES         : callback handling times\r\n"
   "          COUNTERS       : show internal counters\r\n"
   "          SCORES         : histogram of scores (regex + oracle)\r\n"
   "          ORASCORE       : histogram of scores (oracle)\r\n"
   "          REGSCORE       : histogram of scores (regex)\r\n"
   "          LIVEHISTORY    : show short history blacklist\r\n"
   "          LIVEHISTORY_R  : same as before with address resolution (slower)\r\n"},

  {
   "reconfig",
   NULL,
   "  RECONFIG\r\n"
   "    * Reload configuration files (ze-filter.cf)\r\n"
   "    * Syntax :\r\n" "      ze-ndc RECONFIG\r\n"},

  {
   "reload",
   NULL,
   "  RELOAD\r\n"
   "    * Reload / Reopen tables databases or logfiles\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc RELOAD select\r\n"
   "        Where select can be one of :\r\n"
   "          TABLES     : \r\n"
   "                       ze-regex\r\n"
   "                       ze-oracle\r\n"
   "                       ze-tables\r\n"
   "                       ze-xfiles\r\n"
   "          DATABASES  : \r\n"
   "                       ze-rcpt.db\r\n"
   "                       ze-policy.db\r\n"
   "                       ze-urlbl.db\r\n"
   "                       ze-bayes.db\r\n"
   "          LOGFILES   : \r\n"
   "                       ze-regex\r\n"
   "                       ze-files\r\n"
   "                       ze-virus\r\n"
   "                       ze-quarantine\r\n"
   "                       ze-stats\r\n"},

  {
   "reopen",
   NULL,
   "  REOPEN\r\n"
   "    * Reload / Reopen tables databases or logfiles\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc REOPEN select\r\n"
   "        Where select can be one of :\r\n"
   "          TABLES     : \r\n"
   "                       ze-regex\r\n"
   "                       ze-oracle\r\n"
   "                       ze-tables\r\n"
   "                       ze-xfiles\r\n"
   "          DATABASES  : \r\n"
   "                       ze-rcpt.db\r\n"
   "                       ze-policy.db\r\n"
   "                       ze-urlbl.db\r\n"
   "                       ze-bayes.db\r\n"
   "          LOGFILES   : \r\n"
   "                       ze-regex_r\n"
   "                       ze-files\r\n"
   "                       ze-virus\r\n"
   "                       ze-quarantine\r\n"
   "                       ze-stats\r\n"},

  {
   "reset",
   NULL,
   "  RESET\r\n"
   "    * Reinitialize internal values\r\n"
   "    * Syntax :\r\n"
   "      ze-ndc RESET select\r\n"
   "        Where select can be one of :\r\n"
   "          STATS\r\n" "          GREYERRORS\r\n"},

  {
   "restart",
   NULL,
   "  RESTART\r\n"
   "    * Restart the filter (soft restart)\r\n"
   "    * Syntax :\r\n" "      ze-ndc RESTART\r\n"},

  {
   NULL, NULL, NULL}
};

static ndc_help_T  *
get_help_rec(cmd, buf, sz)
     char               *cmd;
     char               *buf;
     size_t              sz;
{
  ndc_help_T         *p = NULL;
  char                s[32];

  for (p = helps; p->cmd != NULL; p++)
  {
    if (p->cmd != NULL && STRCASEEQUAL(p->cmd, cmd))
    {
      snprintf(buf, sz, "%s", p->str);
      if (!STRCASEEQUAL(p->cmd, "help"))
        return p;
    }
  }

  for (p = helps; p->cmd != NULL; p++)
  {
    snprintf(s, sizeof (s), "      %s\r\n", p->cmd);
    strtoupper(s);
    strlcat(buf, s, sz);
  }

  return NULL;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
ndc_help(fd, hdr, help, argc, argv)
     int                 fd;
     char               *hdr;
     bool                help;
     int                 argc;
     char              **argv;
{
  char               *str = NULL;
  ndc_help_T         *p = NULL;
  char               *cmd = "help";

  char                buf[1024];

  if (argc > 0 && argv[0] != NULL)
    cmd = argv[0];

  memset(buf, 0, sizeof (buf));
  p = get_help_rec(cmd, buf, sizeof (buf));

  return show_help(fd, hdr, buf);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
show_help(fd, hdr, str)
     int                 fd;
     char               *hdr;
     char               *str;
{
  if (!sd_printf(fd, "200 %s\r\n", PACKAGE))
    goto ioerror;
  if (hdr != NULL)
    if (!sd_printf(fd, "%s\r\n", hdr))
      goto ioerror;
  if (!sd_printf(fd, "%s\r\n", str))
    goto ioerror;
  if (!sd_printf(fd, "200 OK !!\r\n"))
    goto ioerror;

  return TRUE;

ioerror:
  return FALSE;
}
