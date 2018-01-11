package CCConf;
require 5.000;
require Exporter;
use Carp;

use CCTools;

use strict;
use warnings;

our $VERSION = '0.01';
our @ISA     = qw(Exporter);
our @EXPORT  = qw($VERSION
  cc_conf_init
  cc_conf_read
  cc_conf_get_sections
  cc_conf_get_names
  cc_conf_get_keys
  cc_conf_get
  cc_conf_get_ind
  cc_conf_get_count
  cc_conf_get_lines
  cc_conf_dump
  print_log_lines);

our @EXPORT_OK = qw();

my $debug = 0;

my $fconf = undef;

my %CFS = ();

my $kformat = "%-20s %-20s";

my @LOGS = qw();

# ***************************************************************
#
#
sub cc_conf_init {
  my ($fname, @DIRS) = @_;

  #print "* Module initialized\n";
  {
    my ($dir, $file);
    $dir = `dirname $fname`;
    chomp $dir;
    $file = `basename $fname`;
    chomp $file;
    if (defined $dir && $dir ne ".") {
      $fname = $file;
      @DIRS = ($dir, @DIRS);
    }
  }

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

  #  $clear = 1 unless (exists $CFREM{global});
  if ($clear) {
    %CFS = ();
  }

  my $ID = 0;

  if (defined $fname && $fname ne "") {
    my $ok = cf_conf_init($fname, @dirs);
  }
  my $nl = 0;
  open FIN, "< $fconf" || die "Can't open configuration file $fconf";

  my $section = undef;
  my $name    = undef;
  my $key     = "";

  while (my $line = <FIN>) {
    chomp $line;
    next if $line =~ /^\s*$/;
    next if $line =~ /^\s*#/;

    if ($line =~ /^<([0-9a-z_.-]+)>/i) {

      $ID++;
      $section            = lc($1);
      $name               = "";
      $key                = $ID;
      $key                = sprintf $kformat, $section, $name;
      $CFS{$key}{section} = $section;
      $CFS{$key}{name}    = $name;
      printf "Debut de la section $section\n" if $debug;
      next;
    }

    if ($line =~ /^<([0-9a-z_.-]+)\s+([0-9a-z_.-]+)>/i) {

      $ID++;
      $section            = lc($1);
      $name               = lc($2);
      $key                = $ID;
      $key                = sprintf $kformat, $section, $name;
      $CFS{$key}{section} = $section;
      $CFS{$key}{name}    = $name;

      printf "Debut de la section $section $name\n" if $debug;
      next;
    }

    if ($line =~ /^<\/([0-9a-z_.-]+)>/) {
      printf "Fin de la section $section\n" if $debug;
      $section = undef;
      $name    = undef;
      $key     = "";
      next;
    }

    unless (defined $key && $key ne "") {
      next;
    }

    unless (defined $section && $section ne "") {
    }
    $section = "global" unless defined $section && $section ne "";

    my ($pa, $pb) = split "\ ", $line, 2;
    $pa = cc_trim_spaces($pa);
    $pb = cc_trim_spaces($pb);
    unless (defined $pa && $pa ne "") {
      next;
    }
    $pb = "" unless defined $pb;
    if ($pa =~ /^rem/i) {
      push @{$CFS{$key}{rem}}, $pb;
    }
    $pa = lc($pa);
    if ($pa !~ /^rem/) {
      push @{$CFS{$key}{_txt_}}, $line;
    }
    push @{$CFS{$key}{$pa}}, $pb;
  }
  close FIN;

  return 1;
}

# ***************************************************************
#
#
sub cc_conf_get_sections {
  my @K = keys %CFS;
  return cc_uniq(grep {$_ = $CFS{$_}{section}} @K);
}

# ***************************************************************
#
#
sub cc_conf_get_names {
  my ($section, undef) = @_;
  $section = "global" unless defined $section && $section ne "";

  my @K = grep { exists $CFS{$_}{section} && $section eq $CFS{$_}{section}} keys %CFS;
  return cc_uniq(grep {$_ = $CFS{$_}{name}} @K);
}

# ***************************************************************
#
#
sub cc_conf_get_keys {
  my ($section, $name, undef) = @_;
  $section = "global" unless defined $section && $section ne "";
  $name = "" unless defined $name;

  my @ID = grep {$section eq $CFS{$_}{section} && $name eq $CFS{$_}{name}}
    keys %CFS;
  my @K = qw();
  foreach my $s (@ID) {
    push @K, (grep {$_ !~ /^(name|section)$/i;} keys %{$CFS{$s}});
  }
  return cc_uniq(@K);
}

# ***************************************************************
#
#
sub cc_conf_get {
  my ($k, $section, $name, undef) = @_;

  $section = "global" unless defined $section && $section ne "";
  $name    = ""       unless defined $name    && $name    ne "";
  return undef unless defined $k;

  my $key = lc sprintf $kformat, $section, $name;
  if (!exists $CFS{$key}{$k}) {
    push @LOGS, sprintf "* Option %s/%s/%s not found", $section, $name, $k;
    return qw();
  }
  return @{$CFS{$key}{$k}};
}

# ***************************************************************
#
#
sub cc_conf_get_ind {
  my ($k, $i, $section, $name, undef) = @_;

  $section = "global" unless defined $section && $section ne "";
  $name    = ""       unless defined $name    && $name    ne "";
  return undef unless defined $k;

  my $key = lc sprintf $kformat, $section, $name;
  if (!exists $CFS{$key}{$k}) {
    push @LOGS, sprintf "* Option %s/%s/%s not found", $section, $name, $k;
    return undef;
  }
  my $n = $#{$CFS{$key}{$k}};
  return undef if $n < 0 || $i < -1 || $i > $n;
  return ${$CFS{$key}{$k}}[$i];
}

# ***************************************************************
#
#
sub cc_conf_get_count {
  my ($k, $section, $name, undef) = @_;

  $section = "global" unless defined $section && $section ne "";
  $name    = ""       unless defined $name    && $name    ne "";
  return undef unless defined $k;

  my $key = lc sprintf $kformat, $section, $name;
  if (!exists $CFS{$key}{$k}) {
    push @LOGS, sprintf "* Option %s/%s/%s not found", $section, $name, $k;
    return 0;
  }
  return $#{$CFS{$key}{$k}} + 1;
}

# ***************************************************************
#
#
sub cc_conf_get_lines {
  my ($section, $name, undef) = @_;

  $section = "global" unless defined $section && $section ne "";
  $name    = ""       unless defined $name    && $name    ne "";

  my $key = lc sprintf $kformat, $section, $name;
  if (!exists $CFS{$key}{_txt_}) {
    push @LOGS, sprintf "* Option %s/%s not found", $section, $name;
    return 0;
  }
  return @{$CFS{$key}{_txt_}};
}

# ***************************************************************
#
#
sub cc_conf_dump {
  print "\n* Dump ... \n\n";

  foreach my $id (sort keys %CFS) {
    my $s = "global";
    $s = $CFS{$id}{section} if exists $CFS{$id}{section};
    printf "* KEY  %s\n", $id;
    $s = $CFS{$id}{name} if exists $CFS{$id}{name};
    printf "  %-24s %s\n", "NAME", $s;
    foreach my $key (sort keys %{$CFS{$id}}) {
      next if $key =~ /^(name|section)$/i;
      if (defined $CFS{$id}{$key}) {
        my @X = @{$CFS{$id}{$key}};
        foreach my $x (@X) {
          printf "  %-24s %s\n", $key, $x;
        }
      }
    }

  }
}

# ***************************************************************
#
#
sub print_log {
  my ($hlog, undef) = @_;
  foreach (@LOGS) {
    if (defined $hlog) {
      print $hlog $_, "\n";
    } else {
      print STDERR $_, "\n";
    }
  }
}

# ***************************************************************
#
#
sub clear_log {
  @LOGS = qw();
}

1;

__END__

