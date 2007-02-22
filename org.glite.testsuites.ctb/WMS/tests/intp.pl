#!/usr/bin/perl -w

use strict;

my $ncyc=shift; my $fnam=shift; my $shadowid=shift;

$|=1;

if(-e $fnam){unlink $fnam};
my $tim=getTime();
system(qq{echo "$tim: Send Welcome! to job" >> $fnam});
print "Welcome!\n";
system(qq{echo "Waiting Welcome! from job" >> $fnam});  
my $s=<>; chomp $s; my $len=length($s);
$tim=getTime();
system(qq{echo "-------- $tim: From job: $s" >> $fnam});
unless($s=~/Welcome/){
 system(qq{echo "============ Dialog: Failed" >> $fnam});
 finish();  
};

my $msg='msg'; my $i=1; my $ok=1;
while($ncyc){
 $ncyc--;
         #Send message
 $s="$msg$i"; 
 $tim=getTime();
 system(qq{echo "$tim: Send to job: $s" >> $fnam});
 print "$s\n";
 sleep(5);
         #Waiting for the feedback
 $tim=getTime();
 system(qq{echo "$tim: Waiting data from job" >> $fnam});
 $s=<>; chomp $s; $len=length($s);
 $tim=getTime();
 system(qq{echo "-------- $tim: From job: $s" >> $fnam});
 my $pat="$msg$i";
 unless($s=~/$pat/){
  system(qq{echo "============ Dialog: Failed" >> $fnam});
  $ok=0; last;
 };
 $i++;
};
if($ok){
 system(qq{echo "Dialog has been finished: " >> $fnam});
 system(qq{echo "============ Dialog: OK" >> $fnam});
};
finish();


sub finish{
 while(my $s=<>){
  system(qq{echo "$s" >> $fnam});
  sleep(10); 
 };
}
#
sub getTime{   #return date and time
 my $t=`date +'%D %T'`; chomp $t;
 return $t;
}
