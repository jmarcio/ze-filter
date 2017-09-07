#! /usr/bin/perl -w

#
#
#  j-chkmail - Mail Server Filter for sendmail
#
#  Copyright (c) 2001, 2002, 2003, 2009 - Ecole des Mines de Paris
#
#   Author     : Jose Marcio Martins da Cruz
#                Jose-Marcio.Martins@ensmp.fr
#
#

use strict;
use Time::Local;

# ************************************************************************-
# To use this script you may need to change or adjust values inside this
# section
#
# Set this values to 1 or 0 if you want to enable/disable scanning of
# quarantine directory
my $USE_CLAMAV = 1;
my $USE_MCAFEE = 0;
my $USE_SOPHOS = 0;
#
# Adjust the path of scanners
my $CLAMAVPATH = "/opt/clamav/bin/clamdscan";
my $CLAMAVARGS = " --stdout ";

my $MCAFEEPATH = "/opt/uvscan/uvscan";
my $MCAFEEARGS = "--mime --secure -rv --summary --noboot";

if ($USE_MCAFEE) {
  $ENV{LD_LIBRARY_PATH} = "/opt/uvscan";
}


my $SOPHOSPATH = "/usr/local/bin/sweep";
my $SOPHOSARGS = "-s -f -all -archive -rec -mime ";

my $fname = "/var/jchkmail/files/j-files";
my $vname = "/var/jchkmail/files/j-virus";

# ************************************************************************-
#
#
$CLAMAVPATH = "" if !$USE_CLAMAV;
$MCAFEEPATH = "" if !$USE_MCAFEE;
$SOPHOSPATH = "" if !$USE_SOPHOS;

my $now = time();

my $hostname = `hostname`;
chomp $hostname;

my $date = `date`;
chomp $date;

my $ext;

my $ok = 0;

# ************************************************************************
#
#
my %STATS = ();
my %EXT   = ();
my %XSTAT = ();
my %DSTAT = ();

my %CLK2DAY = ();

my $tlast = 0;

if (-f $fname) {
  open FIN, $fname or die "*** Can't open input file $fname";

  my ($clk, $conn, $ip, $type, $mime, $xfile);
  while (<FIN>) {
    chomp;

    if (0) {
      ($clk, $conn, $type, $mime, $xfile) = split (" ", $_, 5);
    } else {
      my $reste = $_;
      ($clk, $reste) = split (" ", $reste, 2);
      ($conn, $reste) = split (" ", $reste, 2);
      ($ip, $reste) = split (" ", $reste, 2);
      if ($ip =~ /IP=\(.*\)/) {
	($type, $reste) = split (" ", $reste, 2);
      } else {
	$type = $ip;
      }
      ($mime, $xfile) = split (" ", $reste, 2);
    }

    next unless $type =~ /XXX/;
    my $cle = clock2day($clk);

    $cle = int ($clk / 86400);
    $CLK2DAY{$cle} = clock2day($clk);

    $tlast = $cle if $cle > $tlast;

    print $_, "\n" if 0;

    if (defined($xfile) && ($xfile =~ /\.([^.]*)$/)) {
      my $ext = $1;

      $ext =~ tr/A-Z/a-z/;

      $EXT{$ext} = 1;

      push @{$STATS{$cle}}, $ext;

      $XSTAT{$cle}{$ext}++;

      if (exists($DSTAT{$cle})) {
        $DSTAT{$cle}++;
      } else {
        $DSTAT{$cle} = 1;
      }
      $ok = 1;
    }
  }
  close FIN;

  if (!$ok) {
    print <<TXT2;

    $date

    No records found

TXT2
    exit 1;
  }

  print <<TXT1;

  j-chkmail weekly report on $date

  Unsafe files detected on server : $hostname

  Week ending on $CLK2DAY{$tlast}

TXT1

  if ($tlast > 7) {
    my $i;
    foreach $i ($tlast - 7 .. $tlast) {
      $XSTAT{$i}{"dummy"} = 0 if !exists ($XSTAT{$i}{"dummy"});
      $DSTAT{$i} = 0 if !exists($DSTAT{$i});
    }
  }

  my ($cle, $total) = ("", 0);

  printf "----------------------------------------------------------|------\n";
  printf "         ";
  foreach $cle (sort keys %XSTAT) {
    next if $cle + 7 <= $tlast;
    printf " %5s ", $CLK2DAY{$cle};
  }
  printf "|\n";

  printf "----------------------------------------------------------|------\n";
  foreach $ext (sort keys %EXT) {
    my $nbext;
    my $line = "";
    $line = sprintf  "* .%-5s ", $ext;
    foreach $cle (sort keys %XSTAT) {
      next if $cle + 7 <= $tlast;
      my $n = 0;
      $n = $XSTAT{$cle}{$ext} if exists($XSTAT{$cle}{$ext});
      $line .= sprintf " %5d ", $n;
      $nbext += $n;
    }
    printf "%s| %5d \n", $line, $nbext if $nbext > 0;
    $total += $nbext;
  }

  printf "----------------------------------------------------------|------\n";
  printf "  TOTAL  ";

  foreach $cle (sort keys %XSTAT) {
    next if $cle + 7 <= $tlast;
    printf " %5d ", $DSTAT{$cle};
  }
  printf "| %5d\n", $total if defined($total);

  printf "\n\n";
}

# ************************************************************************
#
#
%STATS = ();
%EXT   = ();
%XSTAT = ();
%DSTAT = ();

%CLK2DAY = ();

$tlast = 0;

my $virus = "";

if (-f $vname) {
  open FIN, $vname or die "*** Can't open input file $vname";

  my ($clk, $conn, $type, $mime, $xfile);
  while (<FIN>) {
    chomp;
    ($clk, $conn, undef, $virus) = split (" ", $_, 5);

    next unless defined($clk) && defined($virus) && $virus ne "";

    my $cle = clock2day($clk);

    $cle = int ($clk / 86400);
    $CLK2DAY{$cle} = clock2day($clk);

    $tlast = $cle if $cle > $tlast;

    print $_, "\n" if 0;

    if (defined($virus)) {
      $EXT{$virus} = 1;

      push @{$STATS{$cle}}, $virus;

      $XSTAT{$cle}{$virus}++;

      if (exists($DSTAT{$cle})) {
        $DSTAT{$cle}++;
      } else {
        $DSTAT{$cle} = 1;
      }
      $ok = 1;
    }
  }
  close FIN;

  if (!$ok) {
    print <<TXT2;

    $date

    No records found

TXT2
    exit 1;
  }

  print <<TXT1;

  j-chkmail weekly report on $date

  Virus detected on server : $hostname

TXT1
  #Week ending on $CLK2DAY{$tlast}

  my ($cle, $total) = ("",0);

  if ($tlast > 7) {
    my $i;
    foreach $i ($tlast - 7 .. $tlast) {
      $XSTAT{$i}{"dummy"} = 0 if !exists ($XSTAT{$i}{"dummy"});
      $DSTAT{$i} = 0 if !exists($DSTAT{$i});
    }
  }

  print ("-" x 80);
  print "|------\n";
  print " " x 31;
  foreach $cle (sort keys %XSTAT) {
    next if $cle + 7 <= $tlast;
    $CLK2DAY{$cle} = clock2day($cle * 86400) if !exists($CLK2DAY{$cle});
    printf " %5s ", $CLK2DAY{$cle};
  }
  printf "|\n";


  print ("-" x 80);
  print "|------\n";
  foreach $virus (sort keys %EXT) {
    my $nbext;
    my $line = "";
    $line = sprintf  "* %-28s ", $virus;
    foreach $cle (sort keys %XSTAT) {
      next if $cle + 7 <= $tlast;
      my $n = 0;
      $n = $XSTAT{$cle}{$virus} if exists($XSTAT{$cle}{$virus});
      $line .= sprintf " %5d ", $n;
      $nbext += $n;
    }
    printf "%s| %5d \n", $line, $nbext if $nbext > 0;
    $total += $nbext;
  }

  print ("-" x 80);
  print "|------\n";
  printf "  %-28s ", "TOTAL";

  foreach $cle (sort keys %XSTAT) {
    next if $cle + 7 <= $tlast;
    printf " %5d ", $DSTAT{$cle};
  }
  printf "| %5d\n", $total if defined($total);

  printf "\n\n";
}

# ************************************************************************
#
#
my %VIRUS = ();

if (-f $MCAFEEPATH) {
print <<T2;

    Quarantined virus : Identified by McAfee uvscan

T2
  my $ti = time();
  my$av_args = $MCAFEEARGS;
  my$cmd = "$MCAFEEPATH $av_args /var/spool/jchkmail | grep Found |";

  open VIRUS, $cmd || die "Can't get virus list";

  while (<VIRUS>) {
    chomp;
    eff_blancs($_);
    next if /^$/;

    s/Found//g;
    s/ virus / /g;
    s/ variant / /g;
    s/ trojan / /g;
    s/ the / /g;
    s/ or / /g;
    s/!/ /g;
    eff_blancs($_);
    chomp;
    $VIRUS{$_}++;
  }
  close VIRUS;

  my $nb = 0;
  foreach (reverse sort {$VIRUS{$a} <=> $VIRUS{$b}} keys %VIRUS) {
    printf " - %-30s  %5d\n", $_, $VIRUS{$_};
    $nb += $VIRUS{$_};
  }
  printf "\n   %-30s  %5d\n", "TOTAL", $nb;
  printf"   %-30s  %5d s\n", "Elapsed time :", time() - $ti;
}

printf "\n";

# ************************************************************************
#
#
if (-f $CLAMAVPATH) {
  %VIRUS = ();

  print <<T3;

    Quarantined virus : Identified by ClamAV

T3

  my $ti = time();
  my $av_args = $CLAMAVARGS;
  my $cmd = "$CLAMAVPATH $av_args /var/spool/jchkmail | grep -i FOUND |";

  #printf "* CMD $cmd\n";
  open VIRUS, $cmd || die "Can't get virus list";

  #printf "* CMD GOING $cmd\n";
  while (<VIRUS>) {
    chomp;
    eff_blancs($_);
    next if /^$/;

    my (undef, $virus, undef) = split;
    next if !defined($virus);

    $VIRUS{$virus}++;
  }
  close VIRUS;

  my $nb = 0;
  foreach (reverse sort {$VIRUS{$a} <=> $VIRUS{$b}} keys %VIRUS) {
    printf " - %-30s  %5d\n", $_, $VIRUS{$_};
    $nb += $VIRUS{$_};
  }
  printf "\n   %-30s  %5d\n", "TOTAL", $nb;
  printf"   %-30s  %5d s\n", "Elapsed time :", time() - $ti;
}

printf "\n";

# ************************************************************************
#
#
if (-f $SOPHOSPATH) {
  %VIRUS = ();

  print <<T3;

    Quarantined virus : Identified by Sophos sweep

T3

  my $ti = time();
  my $av_args = $SOPHOSARGS;
  my $cmd = "$SOPHOSPATH $av_args /var/spool/jchkmail |";

  open VIRUS, $cmd || die "Can't get virus list";

  while (<VIRUS>) {
    chomp;
    eff_blancs($_);
    next if /^$/;

    next unless />>> Virus.*'(.*)'/;

    my $virus = $1;
    eff_blancs($virus);
    chomp;
    $VIRUS{$virus}++;
  }
  close VIRUS;

  my $nb = 0;
  foreach (reverse sort {$VIRUS{$a} <=> $VIRUS{$b}} keys %VIRUS) {
    printf " - %-30s  %5d\n", $_, $VIRUS{$_};
    $nb += $VIRUS{$_};
  }
  printf "\n   %-30s  %5d\n", "TOTAL", $nb;
  printf"   %-30s  %5d s\n", "Elapsed time :", time() - $ti;
}

print "\n\n\n";

exit 0;



# ************************************************************************
#
#
sub clock2day {
  my ($c, undef) = @_;
  return 0 if ! defined($c);

  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday) = gmtime($c);

  return sprintf "%02d/%02d", $mday, $mon + 1;
  return sprintf "%02d/%02d", $mon + 1, $mday;
  return sprintf "%02d/%02d", $mday, $mon + 1;
}

# ************************************************************************
#
#
sub clock2month {
  my ($c, undef) = @_;
  return 0 if ! defined($c);

  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday) = gmtime($c);
  return $mon + 1;
}

# ************************************************************************
#
#
sub eff_blancs {
  return "" if $#_ < 0;
  $_[0] =~ s/^[ \t]+|[ \t]+$//g;
  return $_[0];
}

