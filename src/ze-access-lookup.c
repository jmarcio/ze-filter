
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
#include <ze-filter.h>

void
usage(arg)
     char               *arg;
{
  char               *tprefix, *kprefix, *ip, *name, *from, *to, *key;

  tprefix = "GreyCheck";;
  kprefix = "NetClass";

  ip = "10.3.5.5";
  name = "minho.ensmp.fr";
  from = "root@domain.com";
  to = "joe@domain.com";
  key = "10.3.5.5";

  printf("Usage : %s options\n"
         "    -t          : prefix + triplet (default is prefix + key)\n"
         "    -P prefix   : e.g.  -P %s\n"
         "    -K key      : e.g.  -K %s\n"
         "    -I address  : e.g.  -I %s\n"
         "    -H hostname : e.g.  -H %s\n"
         "    -F from     : e.g.  -F %s\n"
         "    -T to       : e.g.  -T %s\n"
         "    -v          : increase log level\n"
         "    -h          : help - this message\n"
         "\n"
         "  Sample queries : \n"
         "    %s -t -P %s -I %s -H %s -F %s -T %s\n"
         "    %s -P %s -K %s\n"
         "\n"
         "  %s\n" "  Compiled on %s\n",
         arg,
         tprefix, key, ip, name, from, to,
         arg, tprefix, ip, name, from, to,
         arg, kprefix, key, PACKAGE, __DATE__ " " __TIME__);
}

static void         HandleStdin();

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 res = 0;
  extern int          ze_logLevel;

  char               *prefix, *ip, *name, *from, *to, *key;
  int                 netclass = NET_UNKNOWN;
  bool                triplet = FALSE;
  bool                fromStdin = FALSE;

  prefix = "ContentCheck";
  ip = "0.0.0.0";
  name = "localhost";
  from = "nobody@localhost";
  to = "nobody@localdomain";
  key = "0.0.0.0";

  configure("ze-policy-lookup", conf_file, FALSE);

  zeLog_SetOutput(FALSE, TRUE);
  ze_logLevel = 10;

  {
    const char         *args = "P:K:I:H:F:T:htvs";
    int                 c;

    while ((c = getopt(argc, argv, args)) != -1) {
      switch (c) {
        case 'h':
          usage(argv[0]);
          exit(0);
          break;
        case 'P':
          prefix = optarg;
          break;
        case 'K':
          key = optarg;
          break;
        case 'I':
          ip = optarg;
          break;
        case 'H':
          name = optarg;
          break;
        case 'F':
          from = optarg;
          break;
        case 'T':
          to = optarg;
          break;
        case 't':
          triplet = !triplet;
          break;
        case 's':
          fromStdin = TRUE;
          break;
        case 'v':
          ze_logLevel++;
          break;
        case 'l':
          break;
        default:
          exit(0);
          break;
      }
    }
  }

  if (!policy_init()) {
    ZE_MessageInfo(0, "Error opening policy database !");
    exit(1);
  }

  if (fromStdin) {
    HandleStdin();
    goto end;
  }

  if (triplet) {
    bool                flag;
    char                class[256];

    memset(class, 0, sizeof (class));
    netclass = GetClientNetClass(ip, name, NULL, class, sizeof (class));

    if (strlen(class) == 0)
      strlcpy(class, NET_CLASS_LABEL(netclass), sizeof (class));
    ZE_MessageInfo(0, "Client IP address : %s", ip);
    ZE_MessageInfo(0, "Client hostname   : %s", name);
    ZE_MessageInfo(0, "Sender            : %s", from);
    ZE_MessageInfo(0, "Recipient         : %s", to);
    ZE_MessageInfo(0, "Client NET class  : %02X %s", netclass, class);
    ZE_MessageInfo(0, "");
    ZE_MessageInfo(0, "Checking = %s %s %s %s %s", prefix, ip, name, from, to);
    flag = check_policy_tuple(prefix, ip, name, class, from, to, TRUE);
    ZE_MessageInfo(0, "RESULT   = %s", STRBOOL(flag, "YES", "NO"));

    goto end;
  }

  {
    bool                flag;
    char                buf[256];

    ZE_MessageInfo(0, "Checking = %s %s", prefix, key);
    memset(buf, 0, sizeof (buf));
    flag = check_policy(prefix, key, buf, sizeof (buf), TRUE);
    ZE_MessageInfo(0, "RESULT   = %s - %s", buf, STRBOOL(flag, "YES", "NO"));
  }

end:
  policy_close();

  return 0;
}

void
HandleStdin()
{
  char                buf[1024], args[1024];

  setenv("SHOWLOOKUP", "YES", TRUE);
  for (memset(buf, 0, sizeof (buf)); fgets(buf, sizeof (buf), stdin) != NULL;
       memset(buf, 0, sizeof (buf))) {
    char               *argv[32];
    int                 argc;
    bool                ok = FALSE;
    char               *pBuf;

    zeStrChomp(buf);

    pBuf = buf;
    pBuf += strspn(pBuf, " ");
    if (*pBuf == '#')
      continue;

    strlcpy(args, pBuf, sizeof (args));
    argc = zeStr2Tokens(pBuf, 32, argv, " ");
    if (argc > 0) {
      if (STRCASEEQUAL(argv[0], "Email")) {
        char                eMail[256];

        extract_email_address(eMail, argv[2], sizeof (eMail));
        if (strlen(eMail) == 0)
          strlcpy(eMail, "nullsender", sizeof (eMail));
        ZE_MessageInfo(0, "====== %s", args);
        ok = PolicyLookupEmailAddr(argv[1], eMail, buf, sizeof (buf));
        ZE_MessageInfo(0, " ->  Result : %-5s : %s", (ok ? "OK" : "--"),
                       (ok ? buf : ""));
        ZE_MessageInfo(0, "");
        continue;
      }

      if (STRCASEEQUAL(argv[0], "Client")) {
        netclass_T          nClass;

        memset(&nClass, 0, sizeof (nClass));
        if (argv[4] != NULL && strlen(argv[4]) > 0) {
          strlcpy(nClass.label, argv[4], sizeof (nClass.label));
          nClass.ok = TRUE;
        }
        ZE_MessageInfo(0, "====== %s", args);
        ok =
          PolicyLookupClient(argv[1], argv[2], argv[3], &nClass, buf,
                             sizeof (buf));
        ZE_MessageInfo(0, " ->  Result : %-5s : %s", (ok ? "OK" : "--"),
                       (ok ? buf : ""));
        ZE_MessageInfo(0, "");
        continue;
      }

      if (STRCASEEQUAL(argv[0], "NetClass")) {
        netclass_T          nClass;

        memset(&nClass, 0, sizeof (nClass));

        ZE_MessageInfo(0, "====== %s", args);
        ok = PolicyLookupNetClass(argv[1], argv[2], &nClass, buf, sizeof (buf));
        ZE_MessageInfo(0, " ->  Result : %-5s : %s %s", (ok ? "OK" : "--"),
                       (ok ? nClass.label : ""), (ok ? nClass.equiv : ""));
        ZE_MessageInfo(0, "");
        continue;
      }
    }

  }

}
