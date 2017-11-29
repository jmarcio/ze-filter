
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


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

#define CHECK_SM_MAC_DEF(value, label, id)				\
  do									\
    {									\
      if (value == NULL) {						\
	static int nb = 0;						\
	if (nb++ < 32)							\
	  ZE_MessageWarning(10, "%s : %s macro value is NULL"		\
			  " - maybe undefined at MTA configuration",	\
			  STREMPTY(id, "NOID"),				\
			  label);					\
      }									\
    } while (0)

sfsistat
mlfi_connect(ctx, hostname, hostaddr)
     SMFICTX            *ctx;
     char               *hostname;
     _SOCK_ADDR         *hostaddr;
{
  CTXPRIV_T          *priv = NULL;
  char                ip[64];
  char               *ident = NULL;
  time_t              conn_id = time(NULL);
  int                 res = SMFIS_CONTINUE;

  int                 res_resolve = RESOLVE_NULL;
  int                 ip_class;
  int                 serv_rate;
  CONNID_T            id;
  int                 nb_open = 0;
  char               *mailserver = NULL;

  char               *msg = NULL;

  char               *client_name, *client_addr, *client_ptr, *client_resolve;

  INIT_CALLBACK_DELAY();

  nb_open = count_connections(1);

  memset(&id, 0, sizeof (id));
  memset(ip, 0, sizeof (ip));
  new_conn_id(&id);

  stats_inc(STAT_CONNECT, 1);

  /*
   ** Allocate contextual private data 
   */
  priv = NULL;
  priv = MLFIPRIV(ctx);
  if (priv == NULL) {
    priv = malloc(sizeof (CTXPRIV_T));
    if (priv == NULL) {
      ZE_LogSysError("malloc(priv) error");

      (void) jsmfi_setreply(ctx, "421", "4.5.1",
                            "I'm too busy now. Try again later !");
      res = SMFIS_TEMPFAIL;

      goto error;
    }
    memset(priv, 0, sizeof (CTXPRIV_T));

    if (smfi_setpriv(ctx, priv) != MI_SUCCESS) {
      FREE(priv);
      ZE_LogMsgError(0, "smfi_setpriv(priv) error");

      (void) jsmfi_setreply(ctx, "421", "4.5.1",
                            "I'm too busy now. Try again later !");
      res = SMFIS_TEMPFAIL;

      goto error;
    }
  } else {
    static int          nx = 0;

    if (nx++ < 10)
      ZE_MessageInfo(10,
                   "private storage area already allocated for this connection");
  }


  INIT_CALLBACK(priv, CALLBACK_CONNECT);

  priv->conn_id = conn_id;
  priv->id = id;

#if HAVE_GETHRTIME
  priv->t_open = gethrtime();
#else
  priv->t_open = time_ms();
#endif
  priv->fd = -1;

#if _FFR_DELAYED_REJECT
  FREE_DELAYED_RESULT(priv->delayed_result);
#endif

  /*
   ** Create and update macro list
   */
  priv->sm = sm_macro_new();
  sm_macro_update(ctx, priv->sm);
#if 0
  sm_macro_log_all(CONNID_STR(priv->id), priv->sm);
#endif

#if 1
  client_name = sm_macro_get_str(priv->sm, "{client_name}");
  client_addr = sm_macro_get_str(priv->sm, "{client_addr}");
  client_ptr = sm_macro_get_str(priv->sm, "{client_ptr}");
  client_resolve = sm_macro_get_str(priv->sm, "{client_resolve}");
  ident = sm_macro_get_str(priv->sm, "_");
#else
  client_name = smfi_getsymval(ctx, "{client_name}");
  client_addr = smfi_getsymval(ctx, "{client_addr}");
  client_ptr = smfi_getsymval(ctx, "{client_ptr}");
  client_resolve = smfi_getsymval(ctx, "{client_resolve}");
  ident = smfi_getsymval(ctx, "_");
#endif

  CHECK_SM_MAC_DEF(client_name, "{client_name}", CONNID_STR(priv->id));
  CHECK_SM_MAC_DEF(client_addr, "{client_addr}", CONNID_STR(priv->id));
  CHECK_SM_MAC_DEF(client_ptr, "{client_ptr}", CONNID_STR(priv->id));
  CHECK_SM_MAC_DEF(client_resolve, "{client_resolve}", CONNID_STR(priv->id));
  /*
   * CHECK_SM_MAC_DEF(ident, "_ (ident)", CONNID_STR(priv->id)); 
   */

  ident = STREMPTY(ident, hostname);
  ident = STREMPTY(ident, client_name);
  ident = STREMPTY(ident, client_addr);
  ident = STREMPTY(ident, client_ptr);
  ident = STREMPTY(ident, "unknown");

  mailserver = smfi_getsymval(ctx, "j");
  mailserver = STREMPTY(mailserver, my_hostname);

  /*
   ** Let's log connection...
   */
  {
    char               *name = NULL;
    char               *addr = NULL;
    char                buf[128];
    char                daemon[128];
    char               *dport, *dname, *daddr;

    name = STREMPTY(hostname, client_name);
    name = STREMPTY(name, "unknown");
    addr = STREMPTY(client_addr, "x.x.x.x");

    if (strlen(ident) == 0 || STRCASEEQUAL(ident, "unknown"))
      snprintf(buf, sizeof (buf), "%s [%s]", name, addr);
    else
      snprintf(buf, sizeof (buf), "%s", ident);

    dport = sm_macro_get_str(priv->sm, "{daemon_port}");
    dname = sm_macro_get_str(priv->sm, "{daemon_name}");
#if 1
    daddr = sm_macro_get_str(priv->sm, "{if_addr}");
    if (daddr == NULL)
#endif
      daddr = sm_macro_get_str(priv->sm, "{daemon_addr}");

#if 1
    snprintf(daemon, sizeof (daemon), "%s:%s:%s", STREMPTY(dname, "-"),
             STREMPTY(daddr, "-"), STREMPTY(dport, "-"));
#else
    snprintf(daemon, sizeof (daemon), "%s:%s", STREMPTY(dname, "-"),
             STREMPTY(dport, "-"));
#endif
    if ((priv->daemon = strdup(daemon)) == NULL) {
      ZE_LogSysError("strdup(%s) error", daemon);
    }

    if (cf_get_int(CF_CLUSTER) == OPT_YES)
      ZE_MessageNotice(9, "%s-%s : %s Connect from %s", CONNID_STR(priv->id),
                     mailserver, daemon, buf);
    else
      ZE_MessageNotice(9, "%s : %s Connect from %s", CONNID_STR(priv->id), daemon,
                     buf);
  }

  if (client_addr != NULL && client_ptr != NULL) {
    if (!STREQUAL(client_addr, "127.0.0.1")) {
      if (STRCASEEQUAL(client_ptr, "localhost") ||
          STRNCASEEQUAL(client_ptr, "localhost.", strlen("localhost."))) {
        ZE_MessageNotice(9, "%s Fake localhost : ADDR=%s PTR=%s",
                       CONNID_STR(priv->id), client_addr, client_ptr);
      }
    }
  }

  /*
   ** Set more private data values
   */
  if ((ident != NULL) && ((priv->ident = strdup(ident)) == NULL)) {
    ZE_LogSysError("strdup priv->ident error (%s)", CONNID_STR(priv->id));
    res = SMFIS_TEMPFAIL;
    log_msg_context(ctx, "Can't allocate memory for ident value");

    goto fin;
  }

  if ((mailserver != NULL) && ((priv->mailserver = strdup(mailserver)) == NULL)) {
    ZE_LogSysError("strdup priv->mailserver error (%s)", CONNID_STR(priv->id));
    res = SMFIS_TEMPFAIL;
    log_msg_context(ctx, "Can't allocate memory for mailserver value");

    goto fin;
  }

  /*
   ** Let's check callback parameter values
   */
  if (hostname == NULL || strlen(hostname) == 0) {
    char               *name = NULL;
    char               *addr = NULL;
    char               *ptr = NULL;

    name = STRNULL(client_name, "unknown");
    addr = STRNULL(client_addr, "unknown");
    ptr = STRNULL(client_ptr, "unknown");

    (void) jsmfi_setreply(ctx, "421", "4.5.1",
                          "Can't get your hostname. Try again later !");
    res = SMFIS_TEMPFAIL;

    log_msg_context(ctx, MSG_NO_PEER_HOSTNAME);
    ZE_MessageInfo(9, "%s %s : hostname parameter NULL : name=%s addr=%s ptr=%s",
                 CONNID_STR(priv->id), ident, name, addr, ptr);

    goto fin;
  }

  if (hostaddr == NULL) {
    char               *name = NULL;
    char               *addr = NULL;
    char               *ptr = NULL;

    name = STRNULL(client_name, "unknown");
    addr = STRNULL(client_addr, "unknown");
    ptr = STRNULL(client_ptr, "unknown");

    (void) jsmfi_setreply(ctx, "421", "4.5.1",
                          "Can't get your IP address. Try again later !");
    res = SMFIS_TEMPFAIL;

    log_msg_context(ctx, "Can't get client IP address");
    ZE_MessageInfo(9, "%s %s : hostname parameter NULL : name=%s addr=%s ptr=%s",
                 CONNID_STR(priv->id), ident, name, addr, ptr);

    goto fin;
  }

  /*
   ** Let's handle client_resolve result 
   */
  {
    ZE_MessageInfo(15, "%s Let's get {client_resolve} value",
                 CONNID_STR(priv->id));

    res_resolve = RESOLVE_NULL;
    if (client_resolve != NULL) {
      if (strcasecmp(client_resolve, "OK") == 0)
        res_resolve = RESOLVE_OK;
      else if (strcasecmp(client_resolve, "FAIL") == 0)
        res_resolve = RESOLVE_FAIL;
      else if (strcasecmp(client_resolve, "FORGED") == 0)
        res_resolve = RESOLVE_FORGED;
      else if (strcasecmp(client_resolve, "TEMPFAIL") == 0)
        res_resolve = RESOLVE_TEMPFAIL;
      else if (strcasecmp(client_resolve, "TEMP") == 0)
        res_resolve = RESOLVE_TEMPFAIL;
      else
        ZE_MessageWarning(9, "%s {client_resolve} returned %s",
                        CONNID_STR(priv->id), client_resolve);
    }
    ZE_MessageInfo(15, "%s Resolve result : %s %s", CONNID_STR(priv->id),
                 STRNULL(client_addr, "unknown"), STRNULL(client_resolve, ""));
    priv->resolve_res = res_resolve;
  }

  /*
   ** Let's get peer IP address
   */
  {
    sa_family_t         addr_family = AF_INET;

    /*
     * TO BE DONE - check this against others --- IPV6 
     */
    addr_family = hostaddr->sa_family;
    priv->addr_family = addr_family;
    switch (addr_family) {
      case AF_INET:
        {
          struct sockaddr_in *sin = (struct sockaddr_in *) hostaddr;

          if (!jinet_ntop(AF_INET, &sin->sin_addr, ip, sizeof (ip))) {
            ZE_MessageError(8, "%08lX mlfi_connect : inet_ntop : %s", conn_id,
                          strerror(errno));
            (void) jsmfi_setreply(ctx, "421", "4.5.1",
                                  "Unknown network error. Try again later !");
            res = SMFIS_TEMPFAIL;
            log_msg_context(ctx, "Can't convert client address (inet_ntop)");
            goto fin;
          }
        }
        break;

      case AF_INET6:
        {
          struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) hostaddr;

          if (!jinet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof (ip))) {
            ZE_MessageError(8, "%08lX mlfi_connect : inet_ntop : %s", conn_id,
                          strerror(errno));
            (void) jsmfi_setreply(ctx, "421", "4.5.1",
                                  "Unknown network error. Try again later !");
            res = SMFIS_TEMPFAIL;
            log_msg_context(ctx, "Can't convert client address (inet_ntop)");
            goto fin;
          }
        }
        break;
      default:
        ZE_MessageWarning(10, "%s : Unknown address family : %d",
                        CONNID_STR(priv->id), addr_family);
        break;
    }
    priv->addr_family = addr_family;
    ZE_MessageInfo(11, "%s Got IP : %s : family %d", CONNID_STR(priv->id), ip,
                 addr_family);
  }

  if ((strlen(ip) > 0) && ((priv->peer_addr = strdup(ip)) == NULL)) {
    ZE_LogSysError("strdup priv->ip error (%s)", CONNID_STR(priv->id));
    res = SMFIS_TEMPFAIL;

    log_msg_context(ctx, "Internal filter error");

    goto fin;
  }

  /*
   ** Update address resolve cache
   */
  if (cf_get_int(CF_RESOLVE_CACHE_ENABLE) == OPT_YES) {
    if (strlen(ip) > 0 && strlen(hostname) > 0)
      resolve_cache_add("PTR", ip, hostname);
  }

  /*
   ** Let's get peer host name
   */
  {
    char               *name = NULL;
    char                tname[64];

    FREE(priv->peer_name);

    memset(tname, 0, sizeof (tname));

    name = sm_macro_get_str(priv->sm, "{client_name}");
    if (name == NULL || strlen(name) == 0) {
      ZE_MessageInfo(11, "%-15s Can't get {client_name} macro value for : %-6s",
                   CONNID_STR(priv->id), ip);
      name = hostname;
    }
    if (name == NULL || strlen(name) == 0) {
      ZE_MessageInfo(11, "%-15s Can't get hostname parameter value for : %-6s",
                   CONNID_STR(priv->id), ip);
      name = sm_macro_get_str(priv->sm, "{client_ptr}");
    }
    if (name == NULL || strlen(name) == 0) {
      ZE_MessageInfo(11, "%-15s Can't get {client_ptr} macro value for : %-6s",
                   CONNID_STR(priv->id), ip);
    }

    if (name != NULL && STRCASEEQUAL(name, "unknown")) {
      snprintf(tname, sizeof (tname), "[%s]", priv->peer_addr);
      name = tname;
      priv->resolve_res = res_resolve = RESOLVE_FAIL;
    }

    if ((priv->peer_name = strdup(name)) == NULL) {
      ZE_LogSysError("strdup({client_ptr}) (%s)", CONNID_STR(priv->id));
      res = SMFIS_TEMPFAIL;
      log_msg_context(ctx, "Internal filter error");

      goto fin;
    }

    if (priv->peer_name == NULL) {
      ZE_LogMsgError(0, "%s Can't get peer hostname", CONNID_STR(priv->id));
      res = SMFIS_TEMPFAIL;
      log_msg_context(ctx, "Internal filter error");

      goto fin;
    }
  }

  /*
   **Check if hostname is valid
   */
#if 0
#define    HOSTNAME_IPV4  "[[]?[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+[]]?"
#define    HOSTNAME_IPV6  "[[]?(ipv6:)?[0-9a-z:]+"

  if (res != SMFIS_CONTINUE || IS_KNOWN(ip_class))
    goto hostname_check_ok;
  if (strexpr(priv->peer_name, HOSTNAME_IPV4, NULL, NULL, TRUE))
    goto hostname_check_ok;
  if (strexpr(priv->peer_name, HOSTNAME_IPV6, NULL, NULL, TRUE))
    goto hostname_check_ok;

  {
    bool                badname = FALSE;

    badname = strexpr(priv->peer_name, "[^a-z0-9.-]+", NULL, NULL, TRUE);
    if (badname) {
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        ZE_MessageInfo(9, "%s SPAM CHECK - Invalid priv->peer_name %s",
                     CONNID_STR(priv->id), priv->peer_name);
      if (0) {
        ZE_MessageInfo(9, "%s - %s Invalid hostname %s",
                     CONNID_STR(priv->id), priv->peer_addr, priv->peer_name);

        (void) jsmfi_setreply(ctx, "421", "4.5.1", "Invalid priv->peer_name !");
        res = SMFIS_TEMPFAIL;
        log_msg_context(ctx, "Invalid client hostname");

        goto fin;
      }
    }
  }

hostname_check_ok:
#endif

  /*
   ** Let's check network class
   */
  {
    char                class[64];

    memset(class, 0, sizeof (class));
    ip_class = GetClientNetClass(ip, hostname, NULL, class, sizeof (class));
    priv->netclass.class = ip_class;

    if (strlen(class) == 0) {
      char               *p = NULL;

      p = NET_CLASS_LABEL(ip_class);
      if (p != NULL)
        strlcpy(priv->netclass.label, p, sizeof (priv->netclass.label));
    } else
      strlcpy(priv->netclass.label, class, sizeof (priv->netclass.label));
  }

  if (IS_UNKNOWN(ip_class) && strlen(priv->netclass.label) == 0) {
    if (check_iprbwl_table(CONNID_STR(priv->id), ip, hostname, &priv->rbwl) !=
        0) {
      ZE_MessageInfo(12,
                   "%s RBWL check list=(%s) code=(%s) class=(%s) addr=(%s) name=(%s)",
                   CONNID_STR(priv->id), priv->rbwl.rbwl, priv->rbwl.code,
                   priv->rbwl.netclass, ip, hostname);

      strlcpy(priv->netclass.label, priv->rbwl.netclass,
              sizeof (priv->netclass.label));

      if (STRCASEEQUAL(priv->netclass.label, "LOCAL")) {
        priv->netclass.class = NET_LOCAL;
        ip_class = NET_LOCAL;
      }
      if (STRCASEEQUAL(priv->netclass.label, "DOMAIN")) {
        priv->netclass.class = NET_DOMAIN;
        ip_class = NET_DOMAIN;
      }
      if (STRCASEEQUAL(priv->netclass.label, "FRIEND")) {
        priv->netclass.class = NET_FRIEND;
        ip_class = NET_FRIEND;
      }
    }
  }

  if (STRCASEEQUAL(priv->netclass.label, "UNKNOWN")) {
    char               *p = NULL;

    switch (priv->resolve_res) {
      case RESOLVE_FAIL:
        p = cf_get_str(CF_RESOLVE_FAIL_NETCLASS);
        break;
      case RESOLVE_FORGED:
        p = cf_get_str(CF_RESOLVE_FORGED_NETCLASS);
        break;
      default:
        break;
    }
    if (p != NULL && strlen(p) > 0)
      strlcpy(priv->netclass.label, p, sizeof (priv->netclass.label));
  }
  {
    static int          n = 0;

    if (n++ < 1000)
      ZE_MessageInfo(11, "%s : %15s %-17s %s", CONNID_STR(priv->id),
                   priv->netclass.label, ip, hostname);
  }

  /*
   ** Let's udpate Throttle computation data
   */
  serv_rate = smtprate_add_entry(RATE_CONN, ip, hostname, 1, conn_id);
  priv->serv_rate = serv_rate;
  ZE_MessageInfo(15, "%s Server connection rate : %d", CONNID_STR(priv->id),
               serv_rate);

  /*
   ** Check global CPU load
   */
  res = check_cpu_load(ctx, CONNID_STR(priv->id), ip, ip_class);
  if (res != SMFIS_CONTINUE) {
    (void) jsmfi_setreply(ctx, "421", "4.5.1",
                          "I'm too busy. Try again later !");

    log_msg_context(ctx, "Server CPU load too high");

    goto fin;
  }

  /*
   ** Check number of open connections
   */
  res = check_filter_open_connections(ctx, CONNID_STR(priv->id), ip, ip_class);
  if (res != SMFIS_CONTINUE) {
    (void) jsmfi_setreply(ctx, "421", "4.5.1",
                          "I'm too busy. Try again later !");

    log_msg_context(ctx, "Simultaneous open connections too high");

    goto fin;
  }

  /*
   ** Let's check the number of file descriptors in use
   */
  {
    int                 fd_check_res;

    fd_check_res = check_file_descriptors();
    if (fd_check_res != 0) {
      switch (fd_check_res) {
        case FD_LEVEL_OK:
          break;
        case FD_LEVEL_SHORT:
          if (!IS_LOCAL(ip_class) && !IS_DOMAIN(ip_class))
            res = SMFIS_TEMPFAIL;
          break;
        case FD_LEVEL_HI:
          res = SMFIS_TEMPFAIL;
          break;
        default:
          break;
      }

      /*
       * shall remark that smfi_setreply doesn't work at mlfi_connect step 
       */
      if (res != SMFIS_CONTINUE) {
        (void) jsmfi_setreply(ctx, "421", "4.5.1",
                              "I'm too busy. Try again later !");
        log_msg_context(ctx, "Too many file descriptors in use");

        goto fin;
      }
    }
  }

  /*
   ** Let's check open connections for this address
   */
  priv->nb_open = connopen_check_host(ip, hostname, 1);

#if _FFR_MODULES
  /*
   ** ze-filter modules
   **
   */
  if (do_module_callback(ctx, 0, &res))
    goto fin;
  if (res != SMFIS_CONTINUE)
    goto fin;
#endif             /* _FFR_MODULES */


  /*
   * return at connection call ???? 
   */
  if (res == SMFIS_CONTINUE && ctx != NULL)
    res = validate_connection(ctx);

  if (res != SMFIS_CONTINUE)
    goto fin;

  /*
   ** That's all folks !
   */

  goto fin;

error:
  /*
   * with cleanup after error 
   */
  if (res != SMFIS_CONTINUE)
    log_msg_context(ctx, STRNULL(msg, "UNKNOWN REASON"));

fin:
  /*
   * without cleanup after error 
   */

  CHECK_CALLBACK_DELAY();


  /*
   * continue processing 
   */
  return res;
}
