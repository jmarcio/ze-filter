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
 *  Creation     : janvier 2006
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
#include <ze-libjc.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define OPTSTR       "hp:i:"

static void         usage(char *name);

typedef struct
{
  union
  {
    struct sockaddr_in6 sock6;
    struct sockaddr_in  sock4;
  } sock;
} sock_in_T;

int
main(int argc, char **argv)
{
  int                 sockfd = -1;
  sock_in_T           cliaddr;

  char               *ip = "0.0.0.0";
  char               *port = "10001";

  server_T            server;

  char                spec[1024];

  memset(&server, 0, sizeof (server));
  {
    int                 c;

    while ((c = getopt(argc, argv, OPTSTR)) >= 0)
    {
      switch (c)
      {
        case 'h':
          usage(argv[0]);
          exit(0);
          break;
        case 'i':
          ip = optarg;
          break;
        case 'p':
          port = optarg;
          break;
        default:
          printf("Error ... \n");
          exit(0);
      }
    }
  }

  snprintf(spec, sizeof (spec), "udp:%s@%s", port, ip);
  sockfd = server_listen(spec, &server);
  if (sockfd < 0)
  {
    exit(EX_SOFTWARE);
  }

  for (;;)
  {
    char                mesg[1024];
    socklen_t           sz;
    int                 n;

    memset(mesg, 0, sizeof (mesg));
    memset(&cliaddr, 0, sizeof (cliaddr));
    sz = sizeof (cliaddr);
    n =
      recvfrom(sockfd, mesg, sizeof (mesg), 0, (struct sockaddr *) &cliaddr, &sz);
    if (n < 0)
      break;

    printf("%s", mesg);
  }

  exit (0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
usage(name)
     char               *name;
{
  char               *p = basename(name);

  printf("Usage : %s options\n"
         "    -h       : This screen\n"
         "    -i       : IP address to bind (default is all addresses)\n"
         "    -p       : port to listen\n"
         "\n"
         "  " COPYRIGHT "\n"
         "  Compiled on %s %s\n\n", p, __DATE__, __TIME__);
}
