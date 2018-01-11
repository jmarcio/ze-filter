package CCTools;
require 5.000;
require Exporter;
use Carp;

use strict;
use warnings;

our $VERSION = '0.01';
our @ISA     = qw(Exporter);
our @EXPORT  = qw($VERSION
  cc_uniq
  cc_trim_spaces
  cc_whereis);

our @EXPORT_OK = qw();

my $debug = 0;

#############################################
#
#
#
sub cc_uniq {
  my @X = @_;
  my %Z = ();
  foreach (@X) {
    $Z{$_}++;
  }
  return sort keys %Z;
}

#############################################
#
#
#
sub cc_trim_spaces {
  my ($s, undef) = @_;
  return "" unless defined $s;
  $s =~ s/^\s+|\s+$//g;
  return $s;
}

#############################################
#
#
#

my @UsualPlaces = qw(/bin /usr/bin /usr/local/bin
		     /sbin /usr/sbin /usr/local/sbin
		     /usr/local/gnu/bin /usr/gnu/bin /opt/gnu/bin
		     /usr/sfw/bin /opt/sfw/bin);

sub cc_whereis {
  my ($f, $h, undef) = @_;

  my $found = undef;

  if (defined $h) {
    foreach my $p (@{$h}) {
      return "$p/$f" if -d $p && -f "$p/$f";
    }
    return $found;
  }
  foreach my $p (@UsualPlaces) {
    return "$p/$f" if -d $p && -f "$p/$f";
  }

  return $found;
}


1;

