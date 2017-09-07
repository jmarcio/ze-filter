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
#include <j-chkmail.h>

static name2id_T    names[] = {
  {"OK", RCPT_OK},
  {"REJECT", RCPT_REJECT},
  {"TEMPFAIL", RCPT_TEMPFAIL},
  {"Access Denied", RCPT_ACCESS_DENIED},
  {"Bad Network", RCPT_BAD_NETWORK},
  {"User Unknown", RCPT_USER_UNKNOWN},
  {NULL, -1}
};

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          log_level;

  configure("j-rcpt-test", conf_file, FALSE);

  set_log_output(FALSE, TRUE);
  log_level = 10;

  if (!policy_init())
  {
    MESSAGE_INFO(0, "Error opening policy database !");
    exit(1);
  }

  if (!rcpt_init())
  {
    MESSAGE_INFO(0, "Error opening rcpt database !");
    exit(1);
  }

  {
    char               *ip, *name, *to;
    int                 netclass;

    int                 flag;
    char               *result = NULL;
    int                 c;

    const char         *args = "i:h:t:l:v";

    ip = "0.0.0.0";
    name = "localhost";
    to = "nobody@localdomain";

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
        case 'i':
          ip = optarg;
          break;
        case 'h':
          name = optarg;
          break;
        case 't':
          to = optarg;
          break;
        case 'v':
          log_level++;
          break;
        case 'l':
          log_level = atoi(optarg);
          break;
        default:
          (void) fprintf(stderr, "-> Unknown command line option : %c\n", c);
          exit(1);
      }
    }

    netclass = GetClientNetClass(ip, name, NULL, NULL, 0);

    MESSAGE_INFO(0, "Client IP address : %s", ip);
    MESSAGE_INFO(0, "Client hostname   : %s", name);
    MESSAGE_INFO(0, "Recipient         : %s", to);
    MESSAGE_INFO(0, "Client NET class  : %02X %s", netclass,
                 NET_CLASS_LABEL(netclass));

    flag = check_rcpt(to, ip, name, netclass);

    result = get_name_by_id(names, flag);
    result = STRNULL(result, "???");

    MESSAGE_INFO(0, "RESULT            = %2d %s", flag, result);
  }

  policy_close();
  rcpt_close();

  return 0;
}
