package RDConf;
require 5.000;
require Exporter;
use Carp;

use strict;
use warnings;

our $VERSION = '0.01';
our @ISA     = qw(Exporter);
our @EXPORT  = qw($VERSION cc_conf_init cc_conf_read cc_conf_get
  cc_conf_get_last cc_conf_dump);

our @EXPORT_OK = qw();

my $debug = 0;

my $fconf = undef;

my %CONF  = ();
my %CFREM = ();

# ***************************************************************
#
#
sub cc_conf_init {
  my ($fname, @DIRS) = @_;

  #print "* Module initialized\n";

  foreach my $dir (@DIRS) {
    my $cf = "$dir/$fname";
    if (-f $cf) {
      $fconf = $cf;
      return (1, "OK : $fconf");
    }
  }
  return (0, "File $fname not found");
}

# ***************************************************************
#
#
sub cc_conf_read {
  my ($clear, $fname, @dirs) = @_;

  $clear = 1 unless (exists $CFREM{global});
  if ($clear) {
    %CONF = ();
    $CFREM{global} = "";
  }

  if (defined $fname && $fname ne "") {
    my $ok = cf_conf_init($fname, @dirs);
  }
  my $nl = 0;
  open FIN, "< $fconf" || die "Can't open configuration file $fconf";

  my $section = undef;
  while (my $line = <FIN>) {
    chomp $line;
    next if $line =~ /^\s*$/;
    next if $line =~ /^\s*#/;

    if ($line =~ /^<([0-9a-z_.-]+)>/i) {
      $section = $1;
      printf "Debut de la section $section\n" if $debug;
      next;
    }

    if ($line =~ /^<\/([0-9a-z_.-]+)>/) {
      printf "Fin de la section $section\n" if $debug;
      $section = undef;
      next;
    }

    $section = "global" unless defined $section && $section ne "";

    my ($pa, $pb) = split "\ ", $line, 2;
    unless (defined $pa && $pa ne "") {
      next;
    }
    $pb = "" unless defined $pb;

    if ($pa =~ /^rem/i) {
      $CFREM{$section} = $pb;
    }

    push @{$CONF{$section}{$pa}}, $pb;

    #print "* Error : $line\n";

  }
  close FIN;

  return 1;
}

# ***************************************************************
#
#
sub cc_conf_get {
  my ($key, $section, undef) = @_;

  $section = "global" unless defined $section && $section ne "";
  if (!exists $CONF{$section}{$key}) {
    printf "* Section %s Var %s not found\n", $section, $key;
    return undef;
  }

  return @{$CONF{$section}{$key}};
}

# ***************************************************************
#
#
sub cc_conf_get_last {
  my ($key, $section, undef) = @_;

  $section = "global" unless defined $section && $section ne "";
  if (!exists $CONF{$section}{$key}) {
    printf "* Section %s Var %s not found\n", $section, $key;
    return undef;
  }

  return ${$CONF{$section}{$key}}[-1];
}

# ***************************************************************
#
#
sub cc_conf_dump {
  print "\n* Dump ... \n\n";
  foreach my $section (keys %CONF) {
    printf "* Section      : %s \n", $section;

    foreach my $key (sort keys %{$CONF{$section}}) {
      if (defined $CONF{$section}{$key}) {
        my @X = @{$CONF{$section}{$key}};
        foreach my $x (@X) {
          printf "  %-20s %s\n", $key, $x;
        }
      }
    }

    print "\n\n";
  }
}

1;

__END__

