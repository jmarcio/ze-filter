
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
#include <libze.h>
#include "ze-filter.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define OS_OTHER 1

#if defined(OS_SOLARIS)
#undef OS_OTHER
#endif
#if defined(OS_TRU64)
#undef OS_OTHER
#endif
#if defined(OS_LINUX)
#undef OS_OTHER
#endif
#if defined(OS_FREEBSD)
#undef OS_OTHER
#endif
#if defined(OS_HPUX)
#undef OS_OTHER
#endif


void               *load_measure_thread(void *);

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#if 1
typedef int64_t     loadtype_T;
#else
typedef uint32_t    loadtype_T;
#endif

typedef struct cpustats_t cpustats_t;

#define JCPU_STATES    5

#define JCPU_SLOPE   100

#if 0
static char        *cpustatenames[] =
  { "idle", "user", "nice", "kernel", "wait", NULL };
#endif

struct cpustats_t {
  bool                ok;
  time_t              date;
  loadtype_T          load[JCPU_STATES];
  int32_t             slope;
};

static bool         gather_cpu_load_info();

#define CPUSTAT_INITIALIZER  {FALSE, (time_t ) 0, {0, 0, 0, 0, 0}}

#define HISTLEN      8

static cpustats_t   cpu_new = CPUSTAT_INITIALIZER;
static cpustats_t   cpu_old = CPUSTAT_INITIALIZER;
static cpustats_t   cpu_pct = CPUSTAT_INITIALIZER;
static cpustats_t   cpu_ary[HISTLEN];
static int          cpu_ptr = 0;
static bool         init_ok = FALSE;

static pthread_mutex_t cpu_mutex = PTHREAD_MUTEX_INITIALIZER;

#define CPU_LOAD_LOCK()    MUTEX_LOCK(&cpu_mutex)

#define CPU_LOAD_UNLOCK()  MUTEX_UNLOCK(&cpu_mutex)

static time_t       dt_stat = 10;




/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
cpu_load_init()
{
  if (init_ok)
    return TRUE;

  CPU_LOAD_LOCK();
  if (!init_ok)
    memset(cpu_ary, 0, sizeof (cpu_ary));
  cpu_ptr = 0;
  CPU_LOAD_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
evaluate_load_pct(pct, old, new)
     cpustats_t         *pct;
     cpustats_t         *old;
     cpustats_t         *new;
{
  loadtype_T          total, half;
  int                 i;
  loadtype_T          oldidle;
  int                 oldslope;

  if ((old == NULL) || (new == NULL) || (pct == NULL))
    return 0;

  total = 0;
  for (i = 0; i < JCPU_STATES; i++)
    total += new->load[i] - old->load[i];

  if (total == 0)
    total = 1;
  half = total / 2;

  oldidle = pct->load[JCPU_IDLE];
  oldslope = pct->slope;

  pct->date = time(NULL);
  for (i = 0; i < JCPU_STATES; i++)
    pct->load[i] = ((new->load[i] - old->load[i]) * 1000 + half) / total;

  pct->ok = new->ok;

  if (pct->load[JCPU_IDLE] < oldidle)
    pct->slope = ++oldslope;
  else
    pct->slope = 0;

  cpu_ary[cpu_ptr++] = *pct;
  cpu_ptr %= HISTLEN;

  return 0;
}


/* ****************************************************************************
 *                                                                            *
 *  ####    ####   #         ##    #####      #     ####                      *
 * #       #    #  #        #  #   #    #     #    #                          *
 *  ####   #    #  #       #    #  #    #     #     ####                      *
 *      #  #    #  #       ######  #####      #         #                     *
 * #    #  #    #  #       #    #  #   #      #    #    #                     *
 *  ####    ####   ######  #    #  #    #     #     ####                      *
 *                                                                            *
 **************************************************************************** */
#if OS_SOLARIS

static              bool
gather_cpu_load_info()
{
  static kstat_ctl_t *kc = NULL;
  static kstat_t     *ksp = NULL;

  static time_t       last = 0;
  time_t              now = time(NULL);

  if (kc == NULL) {
    if ((kc = kstat_open()) == NULL) {
      ZE_LogSysError("kstat_open() error");
      return FALSE;
    }
  } else {
    kid_t               kid;

    if ((kid = kstat_chain_update(kc)) < 0) {
      ZE_LogSysError("kstat_open() error");
      return FALSE;
    }
  }

  if (kc == NULL) {
    ZE_LogMsgError(0, "ks NULL");
    return FALSE;
  }

  if (last + dt_stat > now)
    return TRUE;
  last = now;

  CPU_LOAD_LOCK();

  cpu_old = cpu_new;

  memset(&cpu_new, 0, sizeof (cpu_new));
  cpu_new.date = time(NULL);

  for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) {
    if (strncasecmp(ksp->ks_module, "cpu_stat", 8) == 0) {
      static cpu_stat_t   cpu_buf;
      static cpu_sysinfo_t *cpuinfo = &cpu_buf.cpu_sysinfo;

      if (kstat_read(kc, ksp, &cpu_buf) < 0) {
        ZE_LogSysError("kstat_read() error");
        CPU_LOAD_UNLOCK();
        return FALSE;
      }
      cpu_new.load[JCPU_IDLE] += cpuinfo->cpu[CPU_IDLE];
      cpu_new.load[JCPU_USER] += cpuinfo->cpu[CPU_USER];
      cpu_new.load[JCPU_KERNEL] += cpuinfo->cpu[CPU_KERNEL];
      cpu_new.load[JCPU_WAIT] += cpuinfo->cpu[CPU_WAIT];
      cpu_new.ok = TRUE;
    }
  }

  (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

  CPU_LOAD_UNLOCK();

  return TRUE;
}

#endif             /* OS_SOLARIS */

/* ****************************************************************************
 *                                                                            *
 * #          #    #    #  #    #  #    #                                     *
 * #          #    ##   #  #    #   #  #                                      *
 * #          #    # #  #  #    #    ##                                       *
 * #          #    #  # #  #    #    ##                                       *
 * #          #    #   ##  #    #   #  #                                      *
 * ######     #    #    #   ####   #    #                                     *
 *                                                                            *
 **************************************************************************** */

#if OS_LINUX

#define PROC_STAT_FILE   "/proc/stat"

static              bool
gather_cpu_load_info()
{
  int                 fd;
  static time_t       last = 0;
  time_t              now = time(NULL);
  static char         buf[4096];
  int                 nbl = 0;

  if (last + dt_stat > now)
    return TRUE;
  last = now;

  if ((fd = open(PROC_STAT_FILE, O_RDONLY)) < 0) {
    ZE_LogSysError("Error opening %s", PROC_STAT_FILE);
    return FALSE;
  }

  CPU_LOAD_LOCK();

  cpu_old = cpu_new;
  memset(&cpu_new, 0, sizeof (cpu_new));
  cpu_new.date = time(NULL);

  *buf = '\0';
  if (read(fd, buf, sizeof (buf)) > 0) {
    char               *largv[32];
    int                 largc;
    int                 iarg;

    largc = zeStr2Tokens(buf, 32, largv, "\r\n");
    for (iarg = 0; iarg < largc; iarg++) {
      loadtype_T          u, n, s, i, x, y, z;
      char               *argv[16], **p;
      int                 argc;

      if (!zeStrRegex(largv[iarg], "^cpu ", NULL, NULL, TRUE))
        continue;

      u = n = s = i = x = y = z = 0;

      argc = zeStr2Tokens(largv[iarg], 16, argv, " ");

      p = argv;
      if (*p == NULL)
        continue;

      if (*++p != NULL) {
        u = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }
      if (*++p != NULL) {
        n = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }
      if (*++p != NULL) {
        s = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }
      if (*++p != NULL) {
        i = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }
      if (*++p != NULL) {
        x = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }
      if (*++p != NULL) {
        y = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }
      if (*++p != NULL) {
        z = zeStr2ulonglong(*p, NULL, 0);
        if (errno == ERANGE)
          continue;
      }

      cpu_new.load[JCPU_USER] += u;
      cpu_new.load[JCPU_NICE] += n;
      cpu_new.load[JCPU_KERNEL] += (s + x + y + z);
      cpu_new.load[JCPU_IDLE] += i;
      cpu_new.ok = TRUE;
      nbl++;

      break;
    }
  }
  if (nbl > 1) {
    cpu_new.load[JCPU_USER] /= nbl;
    cpu_new.load[JCPU_NICE] /= nbl;
    cpu_new.load[JCPU_KERNEL] /= nbl;
    cpu_new.load[JCPU_IDLE] /= nbl;
  }
  close(fd);

  (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

  CPU_LOAD_UNLOCK();

  return TRUE;
}

#endif

/* ****************************************************************************
 *                                                                            *
 *                         #####  #                                           *
 *  #####  #####   #    # #     # #    #                                      *
 *    #    #    #  #    # #       #    #                                      *
 *    #    #    #  #    # ######  #    #                                      *
 *    #    #####   #    # #     # #######                                     *
 *    #    #   #   #    # #     #      #                                      *
 *    #    #    #   ####   #####       #                                      *
 *                                                                            *
 **************************************************************************** */

#if OS_TRU64

static              bool
gather_cpu_load_info()
{
  static time_t       last = 0;
  time_t              now = time(NULL);

  if (last + dt_stat > now)
    return TRUE;
  last = now;

  CPU_LOAD_LOCK();

  cpu_old = cpu_new;

  memset(&cpu_new, 0, sizeof (cpu_new));
  cpu_new.date = time(NULL);

  cpu_new.load[JCPU_USER] = 0;
  cpu_new.load[JCPU_NICE] = 0;
  cpu_new.load[JCPU_KERNEL] = 0;
  cpu_new.load[JCPU_IDLE] = cpu_old.load[JCPU_IDLE] + 1;

  (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

  CPU_LOAD_UNLOCK();
  return TRUE;
}

#endif

/* ****************************************************************************
 *                                                                            *
 * ######  #####   ######  ######  #####    ####   #####                      *
 * #       #    #  #       #       #    #  #       #    #                     *
 * #####   #    #  #####   #####   #####    ####   #    #                     *
 * #       #####   #       #       #    #       #  #    #                     *
 * #       #   #   #       #       #    #  #    #  #    #                     *
 * #       #    #  ######  ######  #####    ####   #####                      *
 *                                                                            *
 **************************************************************************** */
#if OS_FREEBSD

#ifndef MAX_KVM_ERR
#define MAX_KVM_ERR    32
#endif

static              bool
gather_cpu_load_info()
{
  static time_t       last = 0;
  time_t              now = time(NULL);

  static int          nerr = 0;

  if ((MAX_KVM_ERR > 0) && (nerr > MAX_KVM_ERR))
    return TRUE;

  if (last + dt_stat > now)
    return TRUE;
  last = now;


#if HAVE_SYSCTLBYNAME
  CPU_LOAD_LOCK();

  cpu_old = cpu_new;

  memset(&cpu_new, 0, sizeof (cpu_new));
  cpu_new.date = time(NULL);

  {
    long                cp_time[CPUSTATES];
    size_t              len = sizeof (cp_time);

    if (sysctlbyname("kern.cp_time", &cp_time, &len, NULL, 0) == 0) {
      cpu_new.load[JCPU_USER] = cp_time[CP_USER];
      cpu_new.load[JCPU_NICE] = cp_time[CP_NICE];
      cpu_new.load[JCPU_KERNEL] = cp_time[CP_SYS] + cp_time[CP_INTR];
      cpu_new.load[JCPU_IDLE] = cp_time[CP_IDLE];

      cpu_new.ok = TRUE;
    } else
      ZE_LogSysError("sysctlbyname error");

    (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

  }

  CPU_LOAD_UNLOCK();
#else
  {
    static kvm_t       *kd = NULL;
    static struct nlist namelist[] = { {"_cp_time"}, {""} };
    static unsigned long nameaddr = 0;

    if (kd == NULL) {
      char                errbuf[_POSIX2_LINE_MAX];

      if ((kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf)) == NULL) {
        nerr++;
        ZE_LogMsgError(0, "kvm_openfiles : %s", errbuf);
        return FALSE;
      }
      if (kvm_nlist(kd, namelist) != 0) {
        nerr++;
        kvm_close(kd);
        kd = NULL;
        return FALSE;
      }
      nameaddr = namelist[0].n_value;
    }
    nerr = 0;

    CPU_LOAD_LOCK();

    cpu_old = cpu_new;

    memset(&cpu_new, 0, sizeof (cpu_new));
    cpu_new.date = time(NULL);

    {
      loadtype_T          cp_time[CPUSTATES];

      if (kvm_read(kd, nameaddr, cp_time, sizeof (cp_time)) == sizeof (cp_time)) {
        cpu_new.load[JCPU_USER] = cp_time[CP_USER];
        cpu_new.load[JCPU_NICE] = cp_time[CP_NICE];
        cpu_new.load[JCPU_KERNEL] = cp_time[CP_SYS] + cp_time[CP_INTR];
        cpu_new.load[JCPU_IDLE] = cp_time[CP_IDLE];

        cpu_new.ok = TRUE;
      } else
        ZE_LogMsgError(0, "kvm_read error");

      (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

    }

    CPU_LOAD_UNLOCK();

    if (kd != NULL) {
      if (kvm_close(kd) != 0) {

      }
    }
    kd = NULL;
  }
#endif

  return TRUE;
}

#endif

/* ****************************************************************************
 *                                                                            *
 * #    #  #####           #    #  #    #                                     *
 * #    #  #    #          #    #   #  #                                      *
 * ######  #    #  #####   #    #    ##                                       *
 * #    #  #####           #    #    ##                                       *
 * #    #  #               #    #   #  #                                      *
 * #    #  #                ####   #    #                                     *
 *                                                                            *
 **************************************************************************** */
#if OS_HPUX

static              bool
gather_cpu_load_info()
{
  static time_t       last = 0;
  time_t              now = time(NULL);

  if (last + dt_stat > now)
    return TRUE;
  last = now;

  CPU_LOAD_LOCK();

  cpu_old = cpu_new;

  memset(&cpu_new, 0, sizeof (cpu_new));
  cpu_new.date = time(NULL);

#if 0
  /*
   * WRITE HERE HOW TO GET LOAD UNDER THIS OS 
   */
#endif
  cpu_new.load[JCPU_USER] = 0;
  cpu_new.load[JCPU_NICE] = 0;
  cpu_new.load[JCPU_KERNEL] = 0;
  cpu_new.load[JCPU_IDLE] = cpu_old.load[JCPU_IDLE] + 1;

  (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

  CPU_LOAD_UNLOCK();
  return TRUE;
}

#endif

/* ****************************************************************************
 *                                                                            *
 *  ####    #####  #    #  ######  #####                                      *
 * #    #     #    #    #  #       #    #                                     *
 * #    #     #    ######  #####   #    #                                     *
 * #    #     #    #    #  #       #####                                      *
 * #    #     #    #    #  #       #   #                                      *
 *  ####      #    #    #  ######  #    #                                     *
 *                                                                            *
 **************************************************************************** */
#if OS_OTHER

static              bool
gather_cpu_load_info()
{
  static time_t       last = 0;
  time_t              now = time(NULL);

  if (last + dt_stat > now)
    return TRUE;
  last = now;

  CPU_LOAD_LOCK();

  cpu_old = cpu_new;

  memset(&cpu_new, 0, sizeof (cpu_new));
  cpu_new.date = time(NULL);

#if 0
  /*
   * WRITE HERE HOW TO GET LOAD UNDER THIS OS 
   */
#endif
  cpu_new.load[JCPU_USER] = 0;
  cpu_new.load[JCPU_NICE] = 0;
  cpu_new.load[JCPU_KERNEL] = 0;
  cpu_new.load[JCPU_IDLE] = cpu_old.load[JCPU_IDLE] + 1;

  (void) evaluate_load_pct(&cpu_pct, &cpu_old, &cpu_new);

  CPU_LOAD_UNLOCK();
  return TRUE;
}

#endif

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
print_cpu_load_info()
{
  loadtype_T          u, n, i, s;

  if ((cpu_new.date == (time_t) 0) || (cpu_old.date == (time_t) 0))
    return;

  u = cpu_new.load[JCPU_USER] - cpu_old.load[JCPU_USER];
  n = cpu_new.load[JCPU_NICE] - cpu_old.load[JCPU_NICE];
  s = cpu_new.load[JCPU_KERNEL] - cpu_old.load[JCPU_KERNEL];
  i = cpu_new.load[JCPU_IDLE] - cpu_old.load[JCPU_IDLE];
  if (u + n + s + i > 0) {
    printf("*** %ld", (long) cpu_new.date);
    printf(" - user   : %5.2f", ((double) u) / (u + n + s + i));
    printf(" - nice   : %5.2f", ((double) n) / (u + n + s + i));
    printf(" - kernel : %5.2f", ((double) s) / (u + n + s + i));
    printf(" - idle   : %5.2f", ((double) i) / (u + n + s + i));
    printf("\n");
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
log_cpu_load_info()
{
  char               *fmt = NULL;

  CPU_LOAD_LOCK();

#if OS_SOLARIS
  fmt =
    "SYSTEM LOAD - idle : %5.1f - user : %5.1f - kernel : %5.1f - wait %5.1f - %3d";

  ZE_MessageInfo(9, fmt,
                 get_cpu_load_info(JCPU_IDLE),
                 get_cpu_load_info(JCPU_USER),
                 get_cpu_load_info(JCPU_KERNEL),
                 get_cpu_load_info(JCPU_WAIT),
                 (int) get_cpu_load_info(JCPU_SLOPE));
#endif             /* OS_SOLARIS */

#if OS_LINUX
  fmt = "SYSTEM LOAD - idle/kernel/user/nice = %5.1f %5.1f %5.1f %5.1f %3d";

  ZE_MessageInfo(9, fmt,
                 get_cpu_load_info(JCPU_IDLE),
                 get_cpu_load_info(JCPU_KERNEL),
                 get_cpu_load_info(JCPU_USER),
                 get_cpu_load_info(JCPU_NICE),
                 (int) get_cpu_load_info(JCPU_SLOPE));
#endif             /* OS_LINUX */

#if OS_FREEBSD
  fmt = "SYSTEM LOAD - idle/kernel/user/nice = %5.1f %5.1f %5.1f %5.1f %3d";

  ZE_MessageInfo(9, fmt,
                 get_cpu_load_info(JCPU_IDLE),
                 get_cpu_load_info(JCPU_KERNEL),
                 get_cpu_load_info(JCPU_USER),
                 get_cpu_load_info(JCPU_NICE),
                 (int) get_cpu_load_info(JCPU_SLOPE));
#endif             /* OS_FREEBSD */

#if OS_HPUX

#endif             /* OS_HPUX */

#if OS_TRU64

#endif             /* OS_TRU64 */

#if OS_HPUX

#endif             /* OS_HPUX */

#if OS_OTHER

#endif             /* OS_OTHER */

  CPU_LOAD_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
get_cpu_load_info(which)
     int                 which;
{
  if (which == JCPU_SLOPE)
    return (double) cpu_pct.slope;

  if ((which >= JCPU_STATES) || (which < 0))
    return 0;

  if (cpu_new.ok)
    return cpu_pct.load[which] / 10.;

  if (which == JCPU_IDLE)
    return 100.;
  else
    return 0.;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define    DT_SYSLOAD       10

static bool         cpuloadgo = TRUE;
static bool         cpuloadok = FALSE;

void               *
cpuload_thread(data)
     void               *data;
{
  pthread_t           tid;
  time_t              old, now;

  ZE_MessageInfo(9, "*** cpuload_tread starting...");
  tid = pthread_self();
  (void) pthread_detach(tid);

  if (cpuloadok)
    return NULL;

  cpuloadgo = TRUE;

  old = now = time(NULL);

  cpu_load_init();

  while (cpuloadgo) {
    sleep(dt_stat);

    now = time(NULL);
    if (gather_cpu_load_info()) {
      if ((int) get_cpu_load_info(JCPU_SLOPE) > 2)
        ZE_MessageInfo(9,
                       "System Load increasing for more than %ld sec - Last Idle = %5.1f",
                       (int) get_cpu_load_info(JCPU_SLOPE) * dt_stat,
                       get_cpu_load_info(JCPU_IDLE));

      if (cf_get_int(CF_LOG_LOAD) == OPT_YES) {
        if (now - old >= 60) {
          log_cpu_load_info();
          old = now;
        }
      }
    } else
      ZE_MessageInfo(9, "Can't gather_cpu_load_info...");
  }

  cpuloadok = FALSE;

  return NULL;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
cpuload_stop()
{
  cpuloadgo = FALSE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
cpuload_start()
{
  pthread_t           tid;
  int                 r;

  ZE_MessageInfo(10, "*** Starting %s ...", ZE_FUNCTION);

#if OS_LINUX
  if ((r = pthread_create(&tid, NULL, cpuload_thread, (void *) NULL)) != 0) {
    ZE_LogSysError("Couldn't launch cpuload_thread");
    return FALSE;
  }
#endif

#if OS_SOLARIS
  if ((r = pthread_create(&tid, NULL, cpuload_thread, (void *) NULL)) != 0) {
    ZE_LogSysError("Couldn't launch cpuload_thread");
    return FALSE;
  }
#endif

#if OS_FREEBSD
  if ((r = pthread_create(&tid, NULL, cpuload_thread, (void *) NULL)) != 0) {
    ZE_LogSysError("Couldn't launch cpuload_thread");
    return FALSE;
  }
#endif

#if OS_TRU64
  if ((r = pthread_create(&tid, NULL, cpuload_thread, (void *) NULL)) != 0) {
    ZE_LogSysError("Couldn't launch cpuload_thread");
    return FALSE;
  }
#endif

#if OS_OTHER
  if ((r = pthread_create(&tid, NULL, cpuload_thread, (void *) NULL)) != 0) {
    ZE_LogSysError("Couldn't launch cpuload_thread");
    return FALSE;
  }
#endif

  return TRUE;
}
