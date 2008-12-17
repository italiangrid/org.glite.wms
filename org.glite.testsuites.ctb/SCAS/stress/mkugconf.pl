#!/usr/bin/perl -w
use strict;
use Getopt::Std;
$Getopt::Std::STANDARD_HELP_VERSION = 1;
sub VERSION_MESSAGE() {}
sub HELP_MESSAGE() {
	warn "Usage: mkugconf.pl -v <vo1>,<vo2>,... [-g <group1>,<group2>,...] [-a <attr1>,<attr2>,...] [-n <number>]\n";
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
getopts("v:g:a:n:", \%opts);
$opts{n} = 10 unless $opts{n};
my $uid = 70000;
my $gid = 70000;
die "Value of -n should be numeric\n" unless $opts{n} =~ /^\d+$/;
HELP_MESSAGE() unless $opts{v};
$opts{g} .= "";
$opts{a} .= "";
die "Cannot create users.conf: $!\n" unless open(USR, ">users.conf");
die "Cannot create groups.conf: $!\n" unless open(GRP, ">groups.conf");
$\ = "\n";
$, = ":";
foreach my $vo (split /,/, $opts{v}) {
	my $nam = compact($vo);
	my $pgid = $gid;
	my ($ucnt, $gcnt, $sgid, $sgnam) = (1, 1);
	foreach my $grp ("", split /,/, $opts{g}) {
		$gid++ if $grp;
		$gcnt++ if $grp;
		$sgid = $grp ? $gid : "";
		$sgnam = $grp ? $nam . $gcnt : "";
		foreach my $att ("", split /,/, $opts{a}) {
			my $fqan = "/$vo" . ($grp ? "/$grp" : "") . ($att ? "/ROLE=$att" : "");
			print GRP "\"$fqan\"", $sgid, $sgnam, $att, $vo;
			$sgid = $sgnam = "-" if $att and ! $grp;
			for(my $i = 1; $i <= $opts{n}; $i++) {
				print USR $uid++, $nam . $ucnt++, $pgid . ($sgid ? ",$sgid" : ""), "${nam}1" . ($sgnam ? ",$sgnam" : ""), $vo, $att;
			}
		}
	}
	$gid++;
	$gcnt++;
}
close(USR);
close(GRP);