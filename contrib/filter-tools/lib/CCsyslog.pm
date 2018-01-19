package CCsyslog;
require 5.000;
require Exporter;
use Carp;
use Sys::Syslog;

use strict;
use warnings;

our $VERSION = '0.01';
our @ISA     = qw(Exporter);
our @EXPORT  = qw(cc_syslog_open cc_syslog_close
  cc_syslog_info cc_syslog_notice
  cc_syslog_warning cc_syslog_error);

our @EXPORT_OK = qw();

my $debug = 0;

my $log_facility = "local2";
my $log_opts     = 'pid,cons,ndelay,nowait';
my $log_program  = 'cc-tools';
my $log_ok       = 0;

#############################################################################
#
#
sub cc_syslog_open {
  my ($program, $facility, undef) = @_;
  $log_program  = $program  if defined $program  && $program  ne "";
  $log_facility = $facility if defined $facility && $facility ne "";
  unless ($log_ok) {
    openlog($log_program, $log_opts, $log_facility);
    $log_ok = 1;
  }
}

#############################################################################
#
#
sub cc_syslog_close {
  if ($log_ok) {
    close_log();
    $log_ok = 0;
  }
}

#############################################################################
#
#
sub cc_syslog_info {
  cc_syslog_open() unless $log_ok;
  foreach (@_) {
    print "INFO : $_\n" if $debug >= 2;
    syslog('info', 'INFO : %s', $_);
  }
}

# #############################################################################
#
#
sub cc_syslog_notice {
  cc_syslog_open() unless $log_ok;
  foreach (@_) {
    print "NOTICE : $_\n" if $debug >= 1;
    syslog('notice', 'NOTICE : %s', $_);
  }
}

# #############################################################################
#
#
sub cc_syslog_warning {
  cc_syslog_open() unless $log_ok;
  foreach (@_) {
    print "WARNING : $_\n" if $debug >= 1;
    syslog('warning', 'WARNING : %s', $_);
  }
}

# #############################################################################
#
#
sub cc_syslog_error {
  cc_syslog_open() unless $log_ok;
  foreach (@_) {
    printf "ERROR : $_\n" if $debug >= 0;
    syslog('err', 'ERROR : %s', $_);
  }
}

#############################################################################
#
#

print "Initialisation CCsyslog OK\n" if $debug;

1;
