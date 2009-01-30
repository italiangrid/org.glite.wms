#!/usr/bin/perl -w
use strict;
use POSIX;
use File::Basename;

my $NAME = "scas";
my $INT = 5;
my $LOG = "/var/tmp/scas-mon.log";

sub getprocent($$) {
	my ($val, $ent);
	unless(-d "/proc/$_[0]") {
		return undef unless open(PF, "</proc/$_[0]");
		$val = <PF>;
		close(PF);
		return $val;
	}
	return undef unless opendir(PE, "/proc/$_[0]");
	while($ent = readdir(PE)) {
		next unless $ent eq $_[1];
		closedir(PE);
		lstat("/proc/$_[0]/$ent");
		if(-l _) {
			$val = readlink("/proc/$_[0]/$ent");
		} elsif(-f _) {
			return undef unless open(PF, "</proc/$_[0]/$ent");
			$val = <PF>;
			close(PF);
		} elsif(-d _) {
			return undef unless opendir(PD, "/proc/$_[0]/$ent");
			my @con = readdir(PD);
			$val = $#con;
		}
		return $val;
	}
	closedir(PE);
	return undef;
}

die "Cannot open /proc: $!\n" unless opendir(PROC, "/proc");
die "Cannot open log file $LOG: $!\n" unless open(LOG, ">>$LOG");
exit unless fork() == 0;
chdir("/");
setsid();
select(LOG);
$| = 1;
while(1) {
	my ($fdc, $mem, $num) = (0, 0, 0);
	rewinddir(PROC);
	while($_ = readdir(PROC)) {
		next unless /^\d+$/;
		my $exe = getprocent($_, "exe");
		next unless $exe and basename($exe) eq $NAME;
		$fdc += getprocent($_, "fd");
		my @statm = split(/ /, getprocent($_, "statm"));
		$mem += $statm[0] << 2;
		$num++;
	}
	my @load = split(/ /, getprocent("loadavg", ""));
	print LOG join(",", time, @load[0..2], $fdc, $mem, $num) . "\n";
	sleep $INT;
}
