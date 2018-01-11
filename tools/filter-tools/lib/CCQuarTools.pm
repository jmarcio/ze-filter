package CCQuarTools;
#
#
#
require 5.000;
require Exporter;
use Carp;

use Sys::Syslog;
use CCsyslog;

use strict;
use warnings;

our $VERSION = '0.01';
our @ISA     = qw(Exporter);
our @EXPORT  = qw(
  $VERSION
  DoFreeMsg
  DoDiscardMsg
);

our @EXPORT_OK = qw();

# ***************************************************************
# Local variables
#

# ------------------------------------------------
#
#
sub do_log {
  foreach (@_) {
    syslog('notice', $_);
  }
}


my $debug = 0;
my $verbose = 0;

# ------------------------------------------------
#
#
sub DoFreeMsg {
  my ($hConf, $id, $h, undef) = @_;

  my ($file, $from, $subj, $size, $xfile, @rcpt, $result);

  printf "* Letting pass %s\n", $id if $debug;

  $file   = $h->{quar};
  @rcpt   = @{$h->{rcpt}};
  $result = $h->{result};

  my $msg = "";

  return (-1, "") unless defined $file && $file ne "";
  my $qfile = "$hConf->{dirspool}/$file";
  return (-1, "Spool file not found") unless -f $qfile;
  return (0,  "Not a clean file")     unless $result == 0;

  # OK
  my $ok = 1;

  my $qok = 1;
  if ($verbose) {
    printf "  Unquaranting message %s\n", ($hConf->{doit} ? "" : "(dry run)");
  }
  foreach my $to (@rcpt) {
    if ($hConf->{doit}) {
      my $r = DoUnquarantine($hConf, $id, $file, $to);
      do_log("$id : Clean $to ");
    } else {
      do_log("$id : Clean $to (dry run)");
    }
  }
  if ($hConf->{doit}) {
    system("mv $qfile $hConf->{dirgood}") if $ok;
  }
  $h->{action} = "free";

  return ($result, $msg);
}

# ------------------------------------------------
#
#
sub DoDiscardMsg {
  my ($hConf, $id, $h, undef) = @_;

  my ($file, $from, $subj, $size, $xfile, @rcpt, $result, $msg);

  printf "* Discarding %s\n", $id if $debug;

  $file   = $h->{file};
  @rcpt   = @{$h->{rcpt}};
  $result = $h->{result};
  $msg    = $h->{msg};

  return (-1, "") unless defined $file && $file ne "";
  my $qfile = "$hConf->{dirspool}/$file";
  return (-1, "Spool file not found") unless -f $qfile;

  # virus
  if ($result == 1) {
    # $nMsgsDiscard++;
    my $sdry = $hConf->{doit} ? "" : "(dry-run)";

    print "  Moving infected file to virus spool\n" if $verbose || $debug;
    foreach my $to (@rcpt) {
      do_log("$id : Virus $to ($msg) $sdry");
    }
    if ($hConf->{doit}) {
      system("mv $qfile $hConf->{dirbad}");
    }
    $h->{action} = "block";
  }

  return ($result, $msg);
}

# ------------------------------------------------
#
#
sub DoUnquarantine {
  my ($hConf, $id, $file, $rcpt, undef) = @_;
  my $result = 0;

  if ($rcpt =~ /<(.+)>/) {
    $rcpt = $1;
  }

  if ($verbose) {
    print "        $rcpt\n";
  }

  my $ftmp = "/tmp/$file.tmp";
  ChangeReply($hConf, $file, $ftmp);

  if (-f $ftmp) {
    my $cmd = "$hConf->{smbin} -Am -bm -f $hConf->{mfrom}  $rcpt < $ftmp";

    my $r = system("$cmd");
    $result = ($r == 0);

    unlink $ftmp;
  }

  return $result;
}

# ------------------------------------------------
#
#
sub ChangeReply {
  my ($hConf, $fmsg, $fout, undef) = @_;

  return 0 unless defined $fmsg && $fmsg ne "";
  $fmsg = "$hConf->{dirspool}/$fmsg";

  return 0 unless -f $fmsg;

  unlink $fout if -f $fout;
  open FOUT, ">$fout" || die "$fout";
  open FMSG, "<$fmsg" || die "$fmsg";

  my $nl = 0;
  while (<FMSG>) {
    my $in_header = 1 .. /^$/;

    unless ($in_header) {
      if (/^\.$/) {
        print FOUT " \n";
      } else {
        print FOUT $_;
      }
      next;
    }

    next if /^From /;

    if (/^(Return-Path|Reply-To):/) {
      unless ($hConf->{mreplyto} eq "preserve") {
        print FOUT "X-", $_;
        next;
      }
      if ($hConf->{mreplyto} eq "preserve") {
        print FOUT $_;
        next;
      }
      next;
    }

    $nl++;

    if ($nl == 1 && $hConf->{mreplyto} !~ /^(remove|preserve)$/) {
      print FOUT "Return-Path: $hConf->{mreplyto}\n";
      print FOUT "Reply-To: $hConf->{mreplyto}\n";
    }
    print FOUT $_;
  }

  close FMSG;
  close FOUT;
}

# ***************************************************************
#
#

return 1;
