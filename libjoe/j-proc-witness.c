/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Apr  2 11:40:00 CEST 2007
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
#include <j-proc-witness.h>


char               *milter_sock_file = NULL;


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char        *pid_file = NULL;

bool
create_pid_file(fname)
     char               *fname;
{
  FILE               *fpid;

  if (fname == NULL || strlen(fname) == 0)
  {
    MESSAGE_INFO(0, "pid_file : NULL pointer");
    return FALSE;
  }

  if (access(fname, F_OK) == 0)
  {
    bool                running = FALSE;

    if ((fpid = fopen(fname, "r")) != NULL)
    {
      char                buf[256];

      if (fgets(buf, sizeof (buf), fpid) != NULL)
      {
        long                pid;

        errno = 0;
        pid = strtol(buf, NULL, 10);
        if (errno == 0 && pid > 0)
        {
          if (kill(pid, 0) == 0)
            running = TRUE;
        }
      }
      fclose(fpid);
    }

    if (!running)
    {
      LOG_MSG_WARNING("PID_FILE %s exists, but j-chkmail isn't running !",
                      fname);
      (void) remove(fname);
    } else
    {
      LOG_MSG_ERROR("PID_FILE %s exists. Is there another j-chkmail running ?",
                    fname);
      exit(EX_SOFTWARE);
    }
  }

  if ((fpid = fopen(fname, "w")) != NULL)
  {
    fprintf(fpid, "%d\n", (int) getpid());
    fclose(fpid);

    if (chmod(fname, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
      LOG_SYS_ERROR("error changing pid file mode %s", fname);

    pid_file = strdup(fname);
    if (pid_file == NULL)
      LOG_SYS_ERROR("error strdup(%s)", fname);

    return TRUE;
  }

  LOG_SYS_ERROR("PID_FILE %s : can't create", fname);

  exit(EX_CANTCREAT);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
remove_pid_file()
{
  pid_t               pid = -1;
  FILE               *fpid = NULL;
  char                buf[32];

  pid = getpid();

  LOG_MSG_DEBUG(20, "remove_pid_file : %d", pid);

  if (pid_file == NULL || strlen(pid_file) == 0)
    return;

  /* check if contents of pid_file equals to pid */
  if ((fpid = fopen(pid_file, "r")) == NULL)
    return;

  memset(buf, 0, sizeof (buf));
  if (fgets(buf, sizeof (buf), fpid) == NULL)
    ;

  fclose(fpid);

  if (atoi(buf) == pid)
    remove(pid_file);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
remove_milter_sock()
{
  char               *sock_file;

  sock_file = milter_sock_file;

  if (sock_file == NULL || strlen(milter_sock_file) == 0)
    goto end;

  if (!strexpr(sock_file, "^(unix|local):", NULL, NULL, TRUE))
    goto end;

  if (strncasecmp(sock_file, "unix:", strlen("unix:")) == 0)
    sock_file += strlen("unix:");

  if (strncasecmp(sock_file, "local:", strlen("local:")) == 0)
    sock_file += strlen("local:");

  if (strlen(sock_file) > 0 && *sock_file == '/')
  {
    struct stat         buf;

    if (lstat(sock_file, &buf) == 0)
    {
      MESSAGE_WARNING(9, "Removing SOCK_FILE : %s", sock_file);
      remove(sock_file);
    }
  }

end:
  /* milter_sock_file = NULL; */
  return;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
define_milter_sock(cf, arg_p, arg_u, arg_i)
     char               *cf;
     char               *arg_p;
     char               *arg_u;
     char               *arg_i;
{
  static char         sm_sock[256];
  char               *sock = NULL;

  memset(sm_sock, 0, sizeof (sm_sock));

  sock = cf;
  if (sock != NULL && strlen(sock) > 0)
    strlcpy(sm_sock, sock, sizeof (sm_sock));

  if ((sock = getenv("JCHKMAIL_SOCKET")) != NULL)
    strlcpy(sm_sock, sock, sizeof (sm_sock));

  if (arg_i != NULL)
    snprintf(sm_sock, sizeof (sm_sock), "inet:%s@localhost", arg_i);

  if (arg_u != NULL)
    snprintf(sm_sock, sizeof (sm_sock), "local:%s", arg_u);

  if (arg_p != NULL)
    strlcpy(sm_sock, arg_p, sizeof (sm_sock));

  MESSAGE_INFO(12, "SM_SOCK = %s", sm_sock);

  milter_sock_file = sm_sock;

  return milter_sock_file;
}
