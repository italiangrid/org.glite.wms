#!/usr/bin/perl -w
use strict;
use Getopt::Std;
$Getopt::Std::STANDARD_HELP_VERSION = 1;
sub VERSION_MESSAGE() {}
sub HELP_MESSAGE() {
	warn "Usage: mkugconf.pl -v <vo1>,<vo2>,... [-g <group1>,<group2>,...] [-r <role1>,<role2>,...] [-n <number>]\n";
	exit;
}
sub compact($) {
	my @a = split /\W/, lc($_[0]);
	my $name = pop @a;
	my $pfx = "";
	$pfx .= substr($_, 0, 1) foreach @a;
	return $pfx . $name;
}
my %opts;
getopts("v:g:r:n:", \%opts);
$opts{n} = 10 unless $opts{n};
my $uid = 70000;
my $gid = 70000;
die "Value of -n should be numeric\n" unless $opts{n} =~ /^\d+$/;
HELP_MESSAGE() unless $opts{v};
$opts{g} .= "";
$opts{r} .= "";
die "Cannot create users.conf: $!\n" unless open(USR, ">users.conf");
die "Cannot create groups.conf: $!\n" unless open(GRP, ">groups.conf");
$\ = "\n";
$, = ":";
foreach my $vo (split /,/, $opts{v}) {
	my $nam = compact($vo);
	my $pgid = $gid;
	my ($sgid, $sgnam);
	my $gcnt = 1;
	foreach my $grp ("", split /,/, $opts{g}) {
		$gid++ if $grp;
		$gcnt++ if $grp;
		$sgid = $grp ? $gid : "";
		$sgnam = $grp ? $nam . $gcnt : "";
		foreach my $role ("", split /,/, $opts{r}) {
			my $ucnt = 1;
			my $sgif = $sgid ? ",$sgid" : ($role and ! $grp ? ",-" : "");
			my $sgnf = $sgnam ? ",$sgnam": ($role and ! $grp ? ",-" : "");
			my $fqan = "/$vo" . ($grp ? "/$grp" : "") . ($role ? "/ROLE=$role" : "");
			print GRP "\"$fqan\"", $sgnam, $sgid, $role, $vo;
			for(my $i = 1; $i <= $opts{n}; $i++) {
				print USR $uid++, $nam . $grp . $role . $ucnt++, $pgid . $sgif, "${nam}1" . $sgnf, $vo, $role;
			}
		}
	}
	$gid++;
	$gcnt++;
}
close(USR);
close(GRP);