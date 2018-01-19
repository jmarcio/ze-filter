package CCSysAdmCheck;
require 5.000;
require Exporter;
use Carp;

use CCTools;

use strict;
use warnings;

our $VERSION = '1.3';

our @ISA    = qw(Exporter);
our @EXPORT = qw($VERSION
  %SysAdmEnv

  $debug

  %SysAdmConf
  %SysAdmAlert
  %SysAdmRegular
  %SysAdmChecks
  %SysAdmExclude

  CheckVMachines
  CheckServices
  CheckFilesystems
  ListDisks
  CheckDiskIO
  CheckDirectories
  CheckSyslog
  CheckDHCPLogs
  CheckMailQueue
  CheckHardwareFaults

  ServiceAdmin

  AddCheck
  GetChecks
  DecodeCheck
);

our @EXPORT_OK = qw();

#
# ----------------------------------------------------------------------
#

###
 #     #    #     #     #####
 #     ##   #     #       #
 #     # #  #     #       #
 #     #  # #     #       #
 #     #   ##     #       #
###    #    #     #       #

our $debug = 0;

our %SysAdmConf    = ();
our %SysAdmAlert   = ();
our %SysAdmRegular = ();
our %SysAdmChecks  = ();
our %SysAdmExclude = ();

our %SysAdmEnv = ();

sub initSysAdmEnv {
  $SysAdmEnv{date}     = `date +"%a %e %b %Y"`;
  $SysAdmEnv{hour}     = `date +"%a %e %b %Y %H:%M:%S"`;
  $SysAdmEnv{hostname} = `hostname`;
  $SysAdmEnv{uname}    = `uname -a | awk -F# '{print \$1}'`;
  $SysAdmEnv{release}  = "";
  if (-f "/etc/release") {
    $SysAdmEnv{release} = `head -1 /etc/release`;
  }
  $SysAdmEnv{os} = `uname`;
  {
    my $id   = POSIX::geteuid();
    my $user = getpwuid($id);
    $SysAdmEnv{user} = $user;
    $SysAdmEnv{id}   = $id;
  }
  $SysAdmEnv{appli}   = `basename $0`;
  $SysAdmEnv{version} = $VERSION;

  my $p = cc_whereis("uptime");
  $SysAdmEnv{uptime} ="";
  if (defined $p && $p ne "") {
    $SysAdmEnv{uptime} = `$p`;
  }

  foreach my $k (keys %SysAdmEnv) {
    $SysAdmEnv{$k} = "unknown" unless defined $SysAdmEnv{$k};
    chomp $SysAdmEnv{$k};
  }
  $SysAdmEnv{uname} .= "\n $SysAdmEnv{release}" if $SysAdmEnv{release} ne "";

  return 1;
}

#
# ----------------------------------------------------------------------
#
#  #####
# #     #  ######  #####   #    #     #     ####   ######   ####
# #        #       #    #  #    #     #    #    #  #       #
#  #####   #####   #    #  #    #     #    #       #####    ####
#       #  #       #####   #    #     #    #       #            #
# #     #  #       #   #    #  #      #    #    #  #       #    #
#  #####   ######  #    #    ##       #     ####   ######   ####

# --------------------------------------------------------------
#
#
sub CheckServices {
  if ($SysAdmEnv{os} =~ /sunos/i) {
    return check_sunos_services(@_);
  }
  if ($SysAdmEnv{os} =~ /linux/i) {
    return check_linux_services(@_);
  }
  return (0, 0);
}

# --------------------------------------------------------------
#
#
sub check_sunos_services {
  my ($lmin, $h, undef) = @_;
  my $ok    = 0;
  my $level = 0;
  my @Data  = qw();

  goto fin unless $SysAdmEnv{os} =~ /sunos/i;

  goto fin unless -x "/bin/svcs";

  open FIN, "/bin/svcs -a |" || return 0;
  my @R = <FIN>;
  close FIN;
  chomp @R;
  my @Maint   = sort grep {/^maintenance/} @R;
  my @Offline = sort grep {/^offline/} @R;

  if ($#Offline >= 0) {
    $level = 1;
  }
  if ($#Maint >= 0) {
    $level = 2;
  }
  if ($level >= $lmin) {
    push @{$h}, ("", str2sep("Services"), "");
    if ($level > $lmin) {
      push @{$h}, @Maint, @Offline;
    } else {
      push @{$h}, "   R.A.S.";
    }
  }
  $ok = 1;

fin:
  return ($ok, $level);
}

# --------------------------------------------------------------
#
#
sub check_linux_services {
  my ($lmin, $h, undef) = @_;
  my $ok    = 0;
  my $level = 0;
  my @Data  = qw();

  push @Data, ("", str2sep("Services"), "");

  if (-x "/usr/bin/systemctl") {
    open FIN, "/usr/bin/systemctl |" || return 0;
    my @R = <FIN>;
    close FIN;
    my @Failed = sort grep {/failed/} @R;

    if ($#Failed >= 0) {
      push @Data, @Failed;
      $level = 1;
    }
    $ok = 1;
    goto fin;
  }

fin:
  if ($ok && $level == 0) {
    push @Data, "   R.A.S.";
  }
  push @{$h}, @Data;

  return ($ok, $level);
}

#
# ----------------------------------------------------------------------
#
 #     # #     #
 #     # ##   ##   ####
 #     # # # # #  #
 #     # #  #  #   ####
  #   #  #     #       #
   # #   #     #  #    #
    #    #     #   ####

# --------------------------------------------------------------
#
#
sub CheckVMachines {
  if ($SysAdmEnv{os} =~ /sunos/i) {
    return check_sunos_vmachines(@_);
  }
  if ($SysAdmEnv{os} =~ /linux/i) {
    return check_linux_vmachines(@_);
  }
  return (0, 0);
}

# --------------------------------------------------------------
#
#
sub check_sunos_vmachines {
  my ($lmin, $h, undef) = @_;
  my $ok    = 0;
  my $level = 0;
  my @Data  = qw();

  goto fin unless $SysAdmEnv{os} =~ /sunos/i;
  goto fin unless -x "/sbin/zonename";
  my $name = `/sbin/zonename`;
  chomp $name;
  if ($name ne "global") {
    $ok = 1;
    goto fin;
  }
  goto fin unless -x "/usr/sbin/zoneadm";

  @Data = `zoneadm list -iv`;
  chomp @Data;
  RewriteTableLines(\@Data, 2);

  if (defined $h) {
    push @{$h}, ("", str2sep("Zones"), "");
    push @{$h}, @Data;
  }
  $ok = 1;

fin:
  return ($ok, $level);
}

# --------------------------------------------------------------
#
#
sub check_linux_vmachines {
  my ($lmin, $h, undef) = @_;
  my $ok    = 0;
  my $level = 0;
  my @Data  = qw();

  goto fin unless $SysAdmEnv{os} =~ /linux/i;

  if (defined $h) {
    push @{$h}, ("", str2sep("Machines Virtuelles"), "");
  }

  if (-x "/usr/bin/virsh") {
    @Data = `/usr/bin/virsh list --all`;
    chomp @Data;

    my $na = 0;
    my $nw = 0;
    my %VM = ();
    foreach my $line (@Data) {
      next unless $line =~ /^\s*(\d+)\s+(\S+)\s+(\S+)/;
      $VM{$1}{name}  = $2;
      $VM{$1}{state} = $3;
      if ($VM{$1}{state} =~ /warning/) {
        $level = 1;
      }
      if ($VM{$1}{state} =~ /error|alert/) {
        $level = 2;
      }
    }
    push @{$h}, @Data;

    $ok = 1;
    goto fin;
  }

  $ok = 1;

fin:
  return ($ok, $level);
}

#
# ----------------------------------------------------------------------
#

#######  #####
#       #     #   ####
#       #        #
#####    #####    ####
#             #       #
#       #     #  #    #
#        #####    ####

# ------------------------------------------------
#
#
sub CheckFilesystems {
  my ($lmin, $h, undef) = @_;

  my ($ok, $level) = (0, 0);

  my $dfopt = "-h -l";
  $dfopt = "-h";
  $dfopt .= " -P" if $SysAdmEnv{os} =~ /Linux/i;

  my @S = qw();
  push @S, "", str2sep("Etat des partitions sur disque"), "";

  my %DF = ();
  {
    my $s = sprintf("%-5s | %6s | %6s | %6s | %6s%% | %6s%% | Key",
      "LEVEL", "Size", "Used", "Avail", "Usage", "Thresh", "Key");
    push @S, $s, "-" x 78;
  }

  foreach my $str (@{$SysAdmChecks{fs}}) {
    my ($ok, $fs, $warn, $crit, $wunit, $cunit) = DecodeCheck($str);
    next unless $ok;

    $fs = "all" if $fs eq "" || $fs eq "*";
    unless ($fs eq "all" || -d $fs) {
      printf STDERR "    %s n'existe pas\n", $fs;
      next;
    }
    next unless get_df(\%DF, $fs, $warn, $crit);
  }

  push @S, "";
  foreach my $k (sort keys %DF) {
    my $s = df2string(\%DF, $k);
    push @S, $s;
    $level = max($DF{$k}{level}, $level);
  }

  $ok = 1;
  push @{$h}, @S;

end:
  return ($ok, $level);
}

# ------------------------------------------------
#
#
sub df2string {
  my ($h, $fs, undef) = @_;

  my %L2S = (0 => "OK", 1 => "WARN", 2 => "CRIT");
  my $s = sprintf(
    "%-5s | %6s | %6s | %6s | %6s%% | %6s%% | %-24s",
    $L2S{$h->{$fs}{level}}, $h->{$fs}{size},  $h->{$fs}{used},
    $h->{$fs}{avail},       $h->{$fs}{usage}, $h->{$fs}{thresh},
    $h->{$fs}{mount}
  );
  return $s;
}

# ------------------------------------------------
#
#
sub get_df {
  my ($h, $optfs, $warn, $crit, undef) = @_;
  return 0 unless defined $h;
  my $mfs = $optfs;
  if (!defined $optfs || $optfs eq "*" || $optfs eq "all") {
    $mfs = "";
  }
  return 0 unless $mfs eq "" || -d $mfs;

  my $dfopt = "-h -l";
  $dfopt = "-h";
  $dfopt .= " -P" if $SysAdmEnv{os} =~ /Linux/i;
  open CMD, "df $dfopt $mfs |" || die "df error";
  while (<CMD>) {
    next if /Filesystem/;
    chomp;
    my @DF = split;
    my $n  = $#DF + 1;
    next unless $n == 6;
    next if $DF[4] eq "0%";
    next if $DF[0] =~ /^(swap|tmpfs|ctfs|objfs|mnttab|proc|sharefs)$/;
    next if $DF[0] =~ /^(\/platform\/)/;
    next if $DF[5] =~ /^(\/lib\/libc.so)/;
    my $key = $mfs ne "" ? $mfs : "all-$DF[5]";
    $h->{$key}{mount} = $DF[5];
    $h->{$key}{size}  = $DF[1];
    $h->{$key}{used}  = $DF[2];
    $h->{$key}{avail} = $DF[3];
    $h->{$key}{nfs}   = ($DF[0] =~ /:\//) ? 1 : 0;

    if ($DF[4] =~ /(\d+)/) {
      $DF[4] = $1;
    }
    $h->{$key}{usage} = $DF[4];

    $h->{$key}{level}  = 0;
    $h->{$key}{thresh} = 0;

    if (defined $warn) {
      if ($DF[4] > $crit) {
        $h->{$key}{level}  = 2;
        $h->{$key}{thresh} = $crit;
      }
    }
    if (defined $crit) {
      if ($DF[4] > $crit) {
        $h->{$key}{level}  = 2;
        $h->{$key}{thresh} = $crit;
      }
    }
  }
  close CMD;
  return 1;
}

#
# ----------------------------------------------------------------------
#

######
#     #     #     ####   #    #   ####
#     #     #    #       #   #   #
#     #     #     ####   ####     ####
#     #     #         #  #  #         #
#     #     #    #    #  #   #   #    #
######      #     ####   #    #   ####

# ------------------------------------------------
#
#
sub ListDisks {
  my ($lmin, $h, undef) = @_;
  my ($ok, $level) = (0, 0);
  my $dfopt = "-h -l";
  $dfopt = "-h";
  $dfopt .= " -P" if $SysAdmEnv{os} =~ /Linux/i;

  my @S = qw();

  push @S, "", str2sep("Etat de remplissage des disques"), "";

  open CMD, "df $dfopt | sort |" || die "df error";
  my @M = qw();
  while (<CMD>) {
    next unless /^\/dev\//;
    chomp;
    my @X = split;
    next unless $#X >= 0;
    my $s = sprintf "%-36s ", shift @X;
    for (my $i = 0 ; $i <= $#X ; $i++) {
      if ($i < $#X) {
        $s .= sprintf "%6s ", $X[$i];
      } else {
        $s .= sprintf " %s ", $X[$i];
      }
    }
    push @M, $s;
  }
  close CMD;
  RewriteTableLines(\@M, 2);
  push @S, @M;
  @M = qw();

  if (-x "/sbin/zfs") {
    push @S, "", "    ZFS filesystems", "";

    open CMD, "/sbin/zfs list |" || die "zfs error";
    push @M, <CMD>;
    close CMD;
  }
  chomp @M;
  RewriteTableLines(\@M, 2);
  push @S, @M;

  push @{$h}, @S;
  $ok = 1;
end:
  return ($ok, $level);
}


#
# ----------------------------------------------------------------------
#

######                                    ###   #######
#     #     #     ####   #    #            #    #     #
#     #     #    #       #   #             #    #     #
#     #     #     ####   ####    #####     #    #     #
#     #     #         #  #  #              #    #     #
#     #     #    #    #  #   #             #    #     #
######      #     ####   #    #           ###   #######

sub CheckDiskIO {
  my ($lmin, $h, undef) = @_;
  my ($ok, $level) = (0, 0);

  my @M = qw();
  if ($SysAdmEnv{os} =~ /Linux/i) {
    open CMD, "iostat -xd |" || die "iostat error";
    @M = <CMD>;
    close CMD;
    shift @M;
    @M = RewriteTableLines(\@M, 1);
  }

  if ($SysAdmEnv{os} =~ /SunOS/i) {
    open CMD, "iostat -xnmce | grep -v :vold |" || die "iostat error";
    @M = <CMD>;
    close CMD;
    chomp @M;
    @M = RewriteTableLines(\@M, 1, "wsvc_t");
  }

  my @S = qw();

  push @S, "", str2sep("IOSTATS disques"), "";
  push @S, @M;
  chomp @S;

  push @{$h}, @S;
  $ok = 1;
end:
  return ($ok, $level);
}

#
# ----------------------------------------------------------------------
#

######
#     #     #    #####    ####
#     #     #    #    #  #
#     #     #    #    #   ####
#     #     #    #####        #
#     #     #    #   #   #    #
######      #    #    #   ####

# ------------------------------------------------
#
#
sub CheckDirectories {
  my ($lmin, $h, undef) = @_;
  my ($ok, $level) = (0, 0);

  my $dfopt = "-h -l";
  $dfopt .= " -P" if $SysAdmEnv{os} =~ /Linux/i;

  my @S = qw();
  push @S, "", str2sep("Etat des partitions sur disque"), "";

  my %DF = ();
  {
    my $s = sprintf("%-5s | %6s | %6s | %6s | %6s%% | %6s%% | Key",
      "LEVEL", "Size", "Used", "Avail", "Usage", "Thresh", "Key");
    push @S, $s, "-" x 78;
  }

  foreach my $str (@{$SysAdmChecks{dir}}) {
    my ($ok, $fs, $warn, $crit, $wunit, $cunit) = DecodeCheck($str);
    next unless $ok;

    unless (-d $fs) {
      printf STDERR "    %s n'existe pas\n", $fs;
      next;
    }
    next unless get_dir(\%DF, $fs, $warn, $crit);
  }

  push @S, "";
  foreach my $k (sort keys %DF) {
    my $s = df2string(\%DF, $k);
    push @S, $s;
    $level = max($DF{$k}{level}, $level);
  }
  push @S, "";

  push @{$h}, @S;

end:
  return ($ok, $level);
}

# ------------------------------------------------
#
#
sub dir2string {
  my ($h, $fs, undef) = @_;

  my %L2S = (0 => "OK", 1 => "WARN", 2 => "CRIT");
  my $s = sprintf(
    "%-5s | %6s | %6s | %6s | %6s%% | %6s%% | %-24s",
    $L2S{$h->{$fs}{level}}, $h->{$fs}{size},  $h->{$fs}{used},
    $h->{$fs}{avail},       $h->{$fs}{usage}, $h->{$fs}{thresh},
    $h->{$fs}{mount}
  );
  return $s;
}

# ------------------------------------------------
#
#
sub get_dir {
  my ($h, $optfs, $warn, $crit, undef) = @_;
  return 0 unless defined $h;
  my $mfs = $optfs;
  if (!defined $optfs || $optfs eq "*" || $optfs eq "all") {
    $mfs = "";
  }
  return 0 unless $mfs eq "" || -d $mfs;

  my $dfopt = "-h -l";
  $dfopt .= " -P" if $SysAdmEnv{os} =~ /Linux/i;
  open CMD, "df $dfopt $mfs |" || die "df error";
  while (<CMD>) {
    next if /Filesystem/;
    chomp;
    my @DF = split;
    my $n  = $#DF + 1;
    next unless $n == 6;
    next if $DF[4] eq "0%";
    next if $DF[0] =~ /^(swap|tmpfs|ctfs|objfs|mnttab|proc|sharefs)$/;
    next if $DF[0] =~ /^(\/platform\/)/;
    my $key = $mfs ne "" ? $mfs : "all-$DF[5]";
    $h->{$key}{mount} = $DF[5];
    $h->{$key}{size}  = $DF[1];
    $h->{$key}{used}  = $DF[2];
    $h->{$key}{avail} = $DF[3];

    if ($DF[4] =~ /(\d+)/) {
      $DF[4] = $1;
    }
    $h->{$key}{usage} = $DF[4];

    $h->{$key}{level}  = 0;
    $h->{$key}{thresh} = 0;

    if (defined $warn) {
      if ($DF[4] > $crit) {
        $h->{$key}{level}  = 2;
        $h->{$key}{thresh} = $crit;
      }
    }
    if (defined $crit) {
      if ($DF[4] > $crit) {
        $h->{$key}{level}  = 2;
        $h->{$key}{thresh} = $crit;
      }
    }
  }
  close CMD;
  return 1;
}

#
# ----------------------------------------------------------------------
#
 ####    #   #   ####   #        ####    ####
#         # #   #       #       #    #  #    #
 ####      #     ####   #       #    #  #
     #     #         #  #       #    #  #  ###
#    #     #    #    #  #       #    #  #    #
 ####      #     ####   ######   ####    ####

# ------------------------------------------------
#
#
sub CheckSyslog {
  my ($lmin, $h, $logFile, undef) = @_;
  my ($ok, $level) = (0, 0);

  my @S = qw();
  push @S, "", str2sep("syslogd tourne ?"), "";

  open CMD, "/bin/ps -edf | grep syslogd | grep -v grep |" || die "ps";
  push @S, <CMD>;
  close CMD;

  system("logger -p local0.err Test syslog : $SysAdmEnv{date}");
  sleep 1;

  push @S, "", str2sep("Les messages de la veille"), "";

  my $size = 0;
  $size = file_size($logFile);
  if ($size < 10) {
    push @S, "",
      "    Log file $logFile est vide (ou presque)",
      "    Verifiez si syslog est correctement configure pour",
      "    enregistrer tous les evenements de priorite warning ou plus",
      "";
  }

  if (-f $logFile) {
    open CMD, "$logFile" || die "$logFile";
    while (<CMD>) {
      chomp;
      next if exclude_line($_);
      push @S, $_;
    }
    close CMD;
  } else {
    push @S, "", "  Fichier de log ($logFile) introuvable !", "";
  }
  $ok = 1;
  push @{$h}, @S;
end:
  return ($ok, $level);
}

# ------------------------------------------------
#
#
sub exclude_line {
  my ($line, undef) = @_;

  my ($m, $d, $h, $host, $tag, $reste) = split " ", $line, 6;

  return 0 unless defined $tag;

  if ($tag =~ /([^[:]+)/) {
    $tag = $1;
  }
  $tag =~ tr/A-Z/a-z/;

  if (exists $SysAdmExclude{$tag}) {
    my @EXPR = @{$SysAdmExclude{$tag}};
    for my $expr (@EXPR) {
      if ($line =~ /$expr/) {
        return 1;
      }
    }
  }
  if (exists $SysAdmExclude{all}) {
    my @EXPR = @{$SysAdmExclude{all}};
    for my $expr (@EXPR) {
      if ($line =~ /$expr/) {
        return 1;
      }
    }
  }
  return 0;
}

#
# ----------------------------------------------------------------------
#

#####   #    #   ####   #####
#    #  #    #  #    #  #    #
#    #  ######  #       #    #
#    #  #    #  #       #####
#    #  #    #  #    #  #
#####   #    #   ####   #

sub CheckDHCPLogs {
  my ($lmin, $h, $lfile, undef) = @_;
  my ($ok, $level) = (0, 0);

  goto end unless -f $lfile;
  unless (open DHCP, "< $lfile") {
    goto end;
  }
  my %NOFREE   = ();
  my %DUPLEASE = ();
  my ($nnf, $ndl) = (0, 0);
  while (<DHCP>) {
    chomp;

    if (/DHCPDISCOVER from (\S+) .* network (\S+): no free leases/) {
      my $s = "";
      $s = sprintf "%-20s %-20s", $2, $1;
      $NOFREE{$s}++;
      $nnf++;
      next;
    }

    if (/uid lease (\S+) for client (\S+) is duplicate on (\S+)/) {
      my $s = sprintf "%-20s %-20s %-20s", $3, $2, $1;
      $DUPLEASE{$s}++;
      $ndl++;
      next;
    }
  }
  close DHCP;
  my @S = qw();

  push @S, "", str2sep("Les messages du serveur DHCP"), "";

  if ($nnf > 0) {
    push @S, "", "  * No free leases", "";
    foreach my $k (sort keys %NOFREE) {
      push @S, (sprintf "%s %5d\n", $k, $NOFREE{$k});
    }
  }
  if ($ndl > 0) {
    push @S, "", "  * Duplicate lease", "";
    foreach my $k (sort keys %DUPLEASE) {
      push @S, (sprintf "%s %5d\n", $k, $DUPLEASE{$k});
    }
  }
  if ($nnf + $ndl == 0) {
    push @S, " R.A.S.";
  }
  push @S, "";
  close DHCP;
  $ok = 1;

  push @{$h}, @S;

end:
  return ($ok, $level);
}

#
# ----------------------------------------------------------------------
#

#    #   ####   #    #  ######  #    #  ######
##  ##  #    #  #    #  #       #    #  #
# ## #  #    #  #    #  #####   #    #  #####
#    #  #  # #  #    #  #       #    #  #
#    #  #   #   #    #  #       #    #  #
#    #   ### #   ####   ######   ####   ######

# --------------------------------------------------------------
#
#
sub CheckMailQueue {
  my ($lmin, $h, undef) = @_;

  my $qthresh = 200;

  my ($ok, $level) = (0, 0);

  my $CFDIR = "/etc/mail";
  my %Queue = ();

  my @CF = qw();

  @CF = qw(sendmail.cf);
  foreach (@CF) {
    $_ = "$CFDIR/$_" unless $_ =~ /^\//;
  }

  my $SMBin = undef;
  unless (defined $SMBin) {
    foreach my $d (qw(/usr/sbin /usr/lib)) {
      if (-x "$d/sendmail") {
        $SMBin = "$d/sendmail";
        last;
      }
    }
  }
  unless (defined $SMBin && $SMBin ne "" && -x $SMBin) {
    printf STDERR "* sendmail not found\n";
    goto end;
  }

  foreach my $cf (@CF) {
    get_queue("$SMBin -bp -C $cf | grep spool |", \%Queue);
  }
  get_queue("$SMBin -bp -Ac | grep spool |", \%Queue);

  my $QueueSize = 0;
  foreach (sort keys %Queue) {
    $QueueSize += $Queue{$_};
  }

  my @S = qw();
  push @S, "", str2sep("Remplissage de la file de messages (sendmail)"), "";
  push @S, (sprintf " . %-30s | %-6s |", "Queue", "count");
  push @S, (sprintf "%s", "-" x 45);
  foreach (sort keys %Queue) {
    push @S, (sprintf " . %-30s | %6d |", $_, $Queue{$_});
  }
  push @S, (sprintf "%s\n",             "-" x 45);
  push @S, (sprintf " . %-30s | %6d |", "TOTAL", $QueueSize);
  push @S, (sprintf " . %-30s | %6d |", "Seuil d'alerte", $qthresh);
  push @S, "";

  $ok = 1;

  push @{$h}, @S;
end:
  return ($ok, $level);
}

sub get_queue {
  my ($cmd, $q, undef) = @_;

  unless (open IN, "$cmd") {
    printf STDERR "ERROR : Can't execute $cmd\n";
    return 0;
  }
  while (<IN>) {
    chomp;

    if (/\s*([^\s]+)\s+\((\d*)\s+request/) {
      my $d = `basename $1`;
      chomp $d;
      $q->{$d} = $2;
      next;
    }

    if (/\s*([^\s]+)\s+is empty/) {
      my $d = `basename $1`;
      chomp $d;
      $q->{$d} = 0;
      next;
    }
  }
  close IN;

  return 1;
}

#
# ----------------------------------------------------------------------
#

#    #    ##    #####   #####   #    #    ##    #####   ######
#    #   #  #   #    #  #    #  #    #   #  #   #    #  #
######  #    #  #    #  #    #  #    #  #    #  #    #  #####
#    #  ######  #####   #    #  # ## #  ######  #####   #
#    #  #    #  #   #   #    #  ##  ##  #    #  #   #   #
#    #  #    #  #    #  #####   #    #  #    #  #    #  ######

# ------------------------------------------------
#
#
sub CheckHardwareFaults {
  if ($SysAdmEnv{os} =~ /sunos/i) {
    return check_sunos_hardware(@_);
  }
  if ($SysAdmEnv{os} =~ /linux/i) {
    return check_linux_hardware(@_);
  }
  return (0, 0);
}

# ------------------------------------------------
#
#
sub check_sunos_hardware {
  my ($lmin, $h, undef) = @_;
  my ($ok, $level) = (0, 0);

  goto end unless $SysAdmEnv{os} =~ /SunOS/i;
  goto end unless (-x "/usr/sbin/fmdump");
  my $zone = `zonename`;
  chomp $zone;
  unless ($zone eq "global") {
    $ok = 1;
    goto end;
  }

  my @S = qw();
  push @S, "", str2sep("Solaris fault management"), "";
  unless (open CMD, "/usr/sbin/fmadm faulty -a | ") {
    printf STDERR "Error running fmadm\n";
    goto end;
  }
  my @M = qw();
  @M = <CMD>;
  close CMD;

  my @X = grep {/^\s*FRU/} @M;

  $ok = 1;
  if ($#X < 0) {
    push @S, "   R.A.S.", "";
    chomp @S;
    push @{$h}, @S;
    goto end;
  }

  $level = 1;
  push @S, @M, "";
  unless (open CMD, "/usr/sbin/fmdump | ") {
    printf STDERR "Error running fmdump\n";
    $ok = 0;
    goto end;
  }
  @M = <CMD>;
  close CMD;
  push @S, @M;

  chomp @S;
  push @{$h}, @S;

end:
  return ($ok, $level);
}

# ------------------------------------------------
#
#
sub check_linux_hardware {
  my ($lmin, $h, undef) = @_;
  my ($ok, $level) = (0, 0);

  goto end unless $SysAdmEnv{os} =~ /Linux/i;

  my @S = qw();
  push @S, "", str2sep("Linux hardware check"), "";

  my @M = qw();
  my $n = $#M + 1;

  $ok = 1;
  if ($n == 0) {
    push @S, "   R.A.S.", "";
    chomp @S;
    push @{$h}, @S;
    goto end;
  }

  $level = 1;

end:
  return ($ok, $level);
}

#
# ----------------------------------------------------------------------
#

 #####
#     #  #    #  #####    ####
#        ##  ##  #    #  #
#        # ## #  #    #   ####
#        #    #  #    #       #
#     #  #    #  #    #  #    #
 #####   #    #  #####    ####

# --------------------------------------------------------------
#
#

sub ServiceAdmin {
  my ($svc, $cmd) = @_;

  return 0 unless $SysAdmEnv{user} eq "root";
  return 0 unless defined $svc && $svc ne "";
  $cmd = "restart" unless defined $cmd;

  if ($svc eq "cron") {
    return 0 unless $SysAdmEnv{user} eq "root";
    if (-x "/usr/sbin/svcadm") {
      system("/usr/sbin/svcadm $cmd cron > /dev/null 2>&1");
    } elsif (-x "/etc/init.d/cron") {
      system("/etc/init.d/cron $cmd > /dev/null 2>&1");
    } elsif (-x "/etc/init.d/crond") {
      system("/etc/init.d/crond $cmd > /dev/null 2>&1");
    } elsif (-x "/usr/sbin/service") {
      system("/usr/sbin/service crond $cmd > /dev/null 2>&1");
    }
    return 1;
  }
  if ($svc eq "syslog") {
    my $SyslogRC = undef;

    if (-x "/usr/sbin/svcadm") {
      $SyslogRC = "/usr/sbin/svcadm $cmd system-log > /dev/null";
    } elsif (-x "/etc/init.d/syslog") {
      $SyslogRC = "/etc/init.d/syslog $cmd > /dev/null";
    } elsif (-x "/etc/init.d/syslogd") {
      $SyslogRC = "/etc/init.d/syslogd $cmd > /dev/null";
    } elsif (-x "/etc/init.d/sysklogd") {
      $SyslogRC = "/etc/init.d/sysklogd $cmd > /dev/null";
    } elsif (-x "/etc/init.d/rsyslog") {
      $SyslogRC = "/etc/init.d/rsyslog $cmd > /dev/null";
    } elsif (-x "/usr/bin/systemctl") {
      $SyslogRC = "/usr/bin/systemctl $cmd rsyslog.service > /dev/null";
    } elsif (-x "/usr/sbin/service") {
      $SyslogRC = "/usr/sbin/service rsyslog $cmd > /dev/null";
    } elsif (-x "/sbin/service") {
      $SyslogRC = "/sbin/service rsyslog $cmd > /dev/null";
    }
    system($SyslogRC) if defined $SyslogRC && $SyslogRC ne "";
    return 1;
  }

  return 0;
}

#
# ----------------------------------------------------------------------
#

#
#           #    #####    ####
#           #    #    #  #
#           #    #####    ####
#           #    #    #       #
#           #    #    #  #    #
#######     #    #####    ####

# --------------------------------------------------------------
#
#
sub str2sep {
  my ($s, undef) = @_;
  if (defined $s && length $s > 0) {
    $s = " $s ";
  } else {
    $s = "";
  }
  return sprintf "%s%s%s", "-" x 3, $s, "-" x (80 - 3 - length $s);
}

# --------------------------------------------------------------
#
#

sub max {
  my ($a, $b, undef) = @_;
  return $a > $b ? $a : $b;
}

sub str2limits {
  my ($s, undef) = @_;

  return qw() unless defined $s && $s ne "";

  $s =~ tr/A-Z/a-z/;
  my ($warn, $crit, undef) = split ",", $s;
  my ($wunit, $cunit) = ('', '');
  if ($warn =~ /(\d+)('%'|g|m|k)?/i) {
    $warn = $1;
    $wunit = $2 if defined $2 && $2 ne "";
  } else {
    $warn = undef;
  }
  if ($crit =~ /(\d+)('%'|g|m|k)?/i) {
    $crit = $1;
    $cunit = $2 if defined $2 && $2 ne "";
  } else {
    $crit = undef;
  }
  return ($warn, $crit, $wunit, $cunit);
}

# --------------------------------------------------------------
#
#
sub file_size {
  my ($fname, undef) = @_;

  #  return 0 unless defined $fname && $fname ne "" && -f $fname;
  #  return stat($fname)->size;
  return 1000;
}

# --------------------------------------------------------------
#
#
sub AddCheck {
  my ($chk, $par, $warn, $crit, undef) = @_;

  return 0 unless defined $chk && defined $par;

  $warn = "" unless defined $warn;
  $crit = "" unless defined $crit;
  my $s = sprintf "%s %s,%s", $par, $warn, $crit;
  push @{$SysAdmChecks{$chk}}, $s;
  return 1;
}

# --------------------------------------------------------------
#
#
sub GetChecks {
  my ($chk, undef) = @_;

  return qw() unless defined $chk && $chk ne "";

  return @{$SysAdmChecks{$chk}};
}

# --------------------------------------------------------------
#
#
sub DecodeCheck {
  my ($chk, undef) = @_;
  my ($ok, $par, $str, $warn, $crit) = (0, undef, undef, undef, undef);

  goto end unless defined $chk && $chk ne "";

  ($par, $str, undef) = split " ", $chk;
  goto end unless defined $par && $par ne "";

  $str =~ tr/A-Z/a-z/;
  ($warn, $crit, undef) = split ",", $str;
  my ($wunit, $cunit) = ('', '');
  if ($warn =~ /(\d+)('%'|g|m|k)?/i) {
    $warn = $1;
    $wunit = $2 if defined $2 && $2 ne "";
  } else {
    $warn = undef;
  }
  if ($crit =~ /(\d+)('%'|g|m|k)?/i) {
    $crit = $1;
    $cunit = $2 if defined $2 && $2 ne "";
  } else {
    $crit = undef;
  }
  $ok = 1;

end:
  return ($ok, $par, $warn, $crit);
}

# --------------------------------------------------------------
#
#
sub RewriteTableLines {
  my ($Lines, $pad, $start, undef) = @_;

  $pad = 1 unless defined $pad;
  my @Len = qw();
  my @Type = qw();
  my $ok = 0;
  foreach my $l (@{$Lines}) {
    if (!$ok && defined $start) {
      $ok = 1 if $start eq "" || $l =~ /$start/;
      next unless $ok;
    }
    my @X = split " ", $l;
    for (my $i = 0; $i <= $#X; $i++) {
      $Len[$i] = 0 unless defined $Len[$i];
      $Len[$i] = max (length $X[$i], $Len[$i]);
      $Type[$i] = 0 unless defined $Type[$i];
      $Type[$i] = 1 if $X[$i] =~ /^[-+]?[0-9.,]+[ ]?[KMGTD%]?$/;
    }
  }
  $ok = 0;
  foreach my $l (@{$Lines}) {
    if (!$ok && defined $start) {
      $ok = 1 if $start eq "" || $l =~ /$start/;
      next unless $ok;
    }
    my $s = "  ";
    my @X = split " ", $l;
    for (my $i = 0; $i <= $#X; $i++) {
      #if ($X[$i] =~ /^[-+]?[0-9.,]+[ ]?[KMGTD%]?$/i) {
      if ($Type[$i]) {
        $s .= " " x ($Len[$i] - length($X[$i])) . $X[$i];
      } else {
        $s .= $X[$i] . " " x ($Len[$i] - length($X[$i]));
      }
      $s .= " " x $pad;
    }
    $l = $s;
  }
  return @{$Lines};
}

# --------------------------------------------------------------
#
#
return initSysAdmEnv();

__END__

#! /usr/bin/perl -w

use strict;
use Getopt::Long;

my $usage = 0;
my $alert = 0;
my $print = 1;
my $raw   = 0;
my $qthresh = 200;
my $SMBin = undef;

my $mailDest = "logmaster\@paris.ensmp.fr";

my $ok = GetOptions('a'    => \$alert,
                    'p'    => \$print,
                    'r'    => \$alert,
                    't=i'  => \$qthresh,
		    'sm=s' => \$SMBin,
                    'h'    => \$usage);

$raw = 0 if $alert || $print;

my $CFDIR = "/etc/mail";
my %Queue = ();

my @CF = qw();

if (0) {
  @CF = </etc/mail/*cf>;
} else {
  @CF = qw(sendmail.cf);
  $_ = "$CFDIR/$_" foreach (@CF)
}

unless (defined $SMBin) {
  foreach my $d (qw(/usr/sbin /usr/lib)) {
    if (-x "$d/sendmail") {
      $SMBin = "$d/sendmail";
      last;
    }
  }
}
unless (defined $SMBin && $SMBin ne "" && -x $SMBin) {
  exit 1;
}

foreach my $cf (@CF) {
  get_queue("$SMBin -bp -C $cf | grep spool |");
}
get_queue("$SMBin -bp -Ac | grep spool |");

if ($raw) {
  printf "%d ", time();
  foreach (sort keys %Queue) {
    printf " %s=%d", $_, $Queue{$_};
  }
  print "\n";
  exit 0;
}

my $hostname = `hostname`;
chomp $hostname;

my $date = `date`;
chomp $date;

my $QueueSize = 0;
if ($print || $alert) {

  $QueueSize = 0;
  foreach (sort keys %Queue) {
    $QueueSize += $Queue{$_};
  }

  open FOUT, ">/tmp/queue.size" || die "Can't open temporary file";
  my $oldf = select FOUT;
  print <<EOT;

  $date
  Messages en attente sur $hostname

EOT
  printf " . %-30s | %-6s |\n", "Queue", "count";
  printf "%s\n", "-" x 45;
  foreach (sort keys %Queue) {
    printf " . %-30s | %6d |\n", $_, $Queue{$_};
  }
  printf "%s\n", "-" x 45;
  printf " . %-30s | %6d |\n", "TOTAL", $QueueSize;
  printf " . %-30s | %6d |\n", "Seuil d'alerte", $qthresh;
  print "\n\n";

  select $oldf;

  if ($print && !$alert) {
    if (-f "/tmp/queue.size") {
      system ("cat /tmp/queue.size"); 
    }
  }
}

if ($alert  && $QueueSize > $qthresh) {
  my $Mail = "/usr/ucb/Mail";
  $Mail = "/usr/bin/Mail" unless -x $Mail;

  open FOUT, "> /tmp/queue.mail" || die "Can't open queue.mail";
  my $oldf = select FOUT;
  print FOUT <<EOT;
-
  Alerte : messages en attente sur $hostname

EOT

  close FOUT;
  select $oldf;

  system("cat /tmp/queue.size >> /tmp/queue.mail");
  system("$Mail -s '[ALERT] File de messages - $hostname' $mailDest < /tmp/queue.mail");

}

unlink "/tmp/queue.mail" if -f "/tmp/queue.mail";

exit (0);

#
#
#
sub get_queue
{
  my ($cmd, undef) = @_;

  open IN, "$cmd" || die;
  while (<IN>) {
    chomp;

    if (/\s*([^\s]+)\s+\((\d*)\s+request/) {
      my $d = `basename $1`;
      chomp $d;
      $Queue{$d} = $2;
      next;
    }

    if (/\s*([^\s]+)\s+is empty/) {
      my $d = `basename $1`;
      chomp $d;
      $Queue{$d} = 0;
      next;
    }
  }
  close IN;
}
