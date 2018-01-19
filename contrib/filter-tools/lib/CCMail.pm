package CCMail;
require 5.000;
require Exporter;
use Carp;

use strict;
use warnings;

our $VERSION = '0.01';
our @ISA     = qw(Exporter);
our @EXPORT  = qw($VERSION cc_send_mail CCMailSendMsg CCMailSendRaw);

our @EXPORT_OK = qw();

my $smBin = "/usr/bin/sendmail";

sub cc_send_mail {
  my ($from, $to, $subject, $body, undef) = @_;

  my $MAILPROGRAM = "| $smBin -oi -f $from $to";

  open(MAIL, $MAILPROGRAM) || die "Can't call sendmail";
  print MAIL "From: $from\r\n";
  print MAIL "To:   $to\r\n";
  print MAIL "Subject: $subject\r\n";

  print MAIL "\r\n";

  print MAIL "$body\r\n";
  print MAIL "\r\n";
  close(MAIL);
}

sub CCMailSendMsg {
  my ($from, $to, $subject, $body, @headers, undef) = @_;

  my $MAILPROGRAM = "| $smBin -oi -f $from $to";

  open(MAIL, $MAILPROGRAM) || die "Can't call sendmail";

  if (@headers) {
    chomp @headers;
    chomp @headers;
    print MAIL join "\r\n", @headers;
  }
  if (defined $from && $from ne "") {
    print MAIL "From: $from\r\n";
  }
  if (defined $to && $to ne "") {
    print MAIL "To:   $to\r\n";
  }
  if (defined $subject && $subject ne "") {
    print MAIL "Subject: $subject\r\n";
  }

  print MAIL "\r\n";

  print MAIL "$body\r\n";
  print MAIL "\r\n";
  close(MAIL);
}

sub CCMailSendRaw {
  my ($from, $to, $body, undef) = @_;

  my $MAILPROGRAM = "| $smBin -oi -f $from $to";

  open(MAIL, $MAILPROGRAM) || die "Can't call sendmail";

  if (defined $from && $from ne "") {
    print MAIL "From: $from\r\n";
  }
  if (defined $to && $to ne "") {
    print MAIL "To:   $to\r\n";
  }

  print MAIL "$body\r\n";
  print MAIL "\r\n";
  close(MAIL);
}


foreach my $d (("/usr/sbin", "/usr/lib")) {
  if (-x "$d/sendmail") {
    $smBin = "$d/sendmail";
    last;
  }
}

1;
