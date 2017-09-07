#! /usr/bin/perl -w

use strict;
use Getopt::Long;

my $action = undef;
my $domain = "";
my $default = "USER-UNKNOWN";

my $ok = GetOptions('d=s' => \$domain,
		    'a=s' => \$action);

my $fname = $ARGV[0];

if (length($domain) == 0 && defined $fname) {
  if ($fname =~ /(.*)\.(relay|unknown)$/) {
    $domain = $1;
    $action = "USER-UNKNOWN" unless defined $action && length($action) > 0;
  }
  if ($fname =~ /(.*)\.local$/) {
    $domain = $1;
    $action = "LOCAL-NET" unless defined $action && length($action) > 0;
  }
  if ($fname =~ /(.*)\.domain$/) {
    $domain = $1;
    $action = "DOMAIN-NET" unless defined $action && length($action) > 0;
  }
  if ($fname =~ /(.*)\.(known|friend)$/) {
    $domain = $1;
    $action = "KNOWN-NET" unless defined $action && length($action) > 0;
  }
}

push @ARGV, "$domain.plus" if (-f "$domain.plus");

print <<EOT;
#
# Access entries for domain $domain
#
EOT

if (length($domain) > 0) {
  printf "CheckRcptDomain:%-43s      YES\n", "$domain";
  printf "RcptAccess:%-48s      %s\n", "$domain", $action;
  printf "#\n";
}

my %USERS = ();

$USERS{postmaster} = "OK";
$USERS{abuse} = "OK";
$USERS{root} = "KNOWN-NET";

while (<>) {
  chomp;

  next if /^\s*$/;
  next if /^\s*#/;
  next if /^\s*@/;

  ($_, undef)  = split "#", $_, 2;

  my ($user, $spec, undef)  = split " ", $_;

  #if (defined($spec) && $spec =~ /^(intranet|local|domain|friend|known)$/i) {
  if (defined($spec)) {
    $spec =~ tr/A-Z/a-z/;
    $USERS{$user} = $spec;
  } else {
    $USERS{$user} = "" if !exists($USERS{$user}) || !defined($USERS{$user});
  }
}

#      OK           : grant access
#      REJECT       : reject all messages for this recipient
#      LOCAL-NET    : accept messages only from LOCAL NETs
#      DOMAIN-NET   : accept messages only from LOCAL NETs
#      FRIEND-NET   : accept messages only from LOCAL NETs
#      USER-UNKNOWN : This is an user unknown (usually
my %SPEC = ("known" => "KNOWN-NET",
	    "local" => "LOCAL-NET",
	    "domain" => "DOMAIN-NET",
	    "friend" => "FRIEND-NET",
	    "known"  => "KNOWN-NET",
            "spamtrap" => "SPAMTRAP",
	    "intranet" =>"KNOWN-NET");

foreach my $user (sort keys %USERS) {
  my $spec = "OK";

  $spec =  $USERS{$user} if length($USERS{$user}) > 0;
  $spec = $SPEC{$spec} if exists $SPEC{$spec};
  printf "RcptAccess:%-48s      %s\n", "$user\@$domain", $spec;
}

print <<EOT;
#
#
#
EOT
