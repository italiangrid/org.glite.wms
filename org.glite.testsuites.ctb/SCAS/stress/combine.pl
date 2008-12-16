#!/usr/bin/perl -w
use strict;

my ($rp, $nr, %data, @lr) = 0;

while(my $file = shift) {
	next unless open(FILE, "<$file");
	while(<FILE>) {
		chomp;
		my @fld = split /,/;
		$nr = $#fld if $. == 1;
		my $key = shift @fld;
		for(my $i = 0; $i < $nr; $i++) {
			if(defined $data{$key}[$rp + $i]) {
				$data{$key}[$rp + $i] += $fld[$i];
				$data{$key}[$rp + $i] /= 2;
			} else {
				$data{$key}[$rp + $i] = $fld[$i];
			}
			$lr[$rp + $i] = $key;
		}
	}
	close(FILE);
	$rp += $nr;
}
for(my $i = 0; $i < $rp; $i++) {
	my $old = 0;
	foreach(sort keys %data) {
		if(defined $data{$_}[$i]) {
			$old = $data{$_}[$i];
		} else {
			$data{$_}[$i] = $_ < $lr[$i] ? $old : 0;
		}
	}
}
$\ = "\n";
print join ",", $_, @{$data{$_}} foreach sort keys %data;
