#!/usr/bin/perl -w
use strict;
use POSIX;

my (%users, %groups, %uids, %gids);
my $mask = shift;
die "Usage: userwipe.pl <uid mask>\n\n" unless $mask;
my $re = eval {qr/$mask/};
die "Error: bad regexp\n\n" if $@;
while(my ($name, $pw, $uid, $gid) = getpwent) {
	next unless $uid =~ /$re/;
	$users{$name} = 1;
	$gids{$gid} = 1;
	$uids{$uid} = 1;
	print "User $name ($uid) is to be erased\n";
}
while(my ($name, $pw, $gid, $memb) = getgrent) {
	my $cnt = 0;
	my @memb = split / /, $memb;
	map {$cnt++ if exists $users{$_}} @memb;
	next unless exists $gids{$gid} or ($cnt and $cnt == $#memb + 1);
	$groups{$name} = 1;
	print "Group $name ($gid) is to be erased\n";
}
print "Last chance to cancel. Type 'yes' to proceed: ";
exit unless lc(readline(STDIN)) eq "yes\n";
`/usr/sbin/userdel -r $_` foreach keys %users;
`/usr/sbin/groupdel $_` foreach keys %groups;
