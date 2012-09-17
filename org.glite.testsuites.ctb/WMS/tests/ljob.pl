#!/usr/bin/perl -w

use Getopt::Std;
use strict;

#--------------------------------------------------------------
#                JDL testing (submitted job)
#
###### JDL generation parameters:
#    * -v Environment params:
#         -v <N pairs>:<length of the parname and value>
#    * -i InputSandbox params(local files):
#         -i <N files>:<file size in KB>:<flie name length>
#    * -o OutputSandbox params:
#         -o <N files>:<file size in KB>:<flie name length>
#    * -d data with InputSandboxBaseURI
#         -d <N files>:<file size in KB>:<BaseURI>
#    
#--------------------------------------------------------------
my $deb=0; my $id; my ($s,$par);
my ($n,$len,$size,$ok,$OK);
                         #Input parameters
my %opt=(); getopts("v:i:o:d:",\%opt);
$OK=1;
              #Check Environment variables
if(defined $opt{v}){ #-------------- environment
 $s=$opt{v};
 ($n,$len)=split ':',$s;   #<N pairs>:<length of the parname and value>
 print("==================== Check Environment: ========================\n");
 $ok=1;
 my $nam='nam_'; my $val='val_';
 for my $i (1..$len){$nam.='n'; $val.='v'};
 for my $i (1..$n){
  unless(exists $ENV{"$nam$i"}){
   $ok=0; $OK=0;  print("Error: Env $nam$i was not set\n"); next;
  };
  unless($ENV{"$nam$i"} eq "$val$i"){
   $ok=0; $OK=0;  print(qq{Error: Env $nam$i=$ENV{"$nam$i"}, must be "$val$i"\n}); next;
  };
 };
 if($ok){print "Check OK\n"}else{print "Check Fail!\n"};
};
              #Check input files
$ok=1;
if(defined $opt{i}){ #-------------- InputSandbox parameters
 $s=$opt{i};
 ($n,$size,$len)=split ':',$s;   #<N files>:<file size in KB>:<flie name length>
 $size*=1024;
 print("==================== Check Input files: ========================\n");
 $ok=1;
 my $fname='finp_';
 for my $i (1..$len){$fname.='f'};
 for my $i (1..$n){
  unless(-e "$fname$i"){
   $ok=0; $OK=0;  print("Error: Input file $fname not exists\n"); next;
  };
  $s=-s "$fname$i";
  unless($s == $size){
   $ok=0; $OK=0;  print("Error: Input file $fname length=$s, must be $size\n"); next;
  };
 };
 if($ok){print "Check OK\n"}else{print "Check Fail!\n"};
};
             # Save Output files
if(defined $opt{o}){ #-------------- OutputSandbox parameters
 $s=$opt{o};
 ($n,$size,$len)=split ':',$s;   #<N files>:<file size in KB>:<flie name length>
 my $fname='fout_'; for my $i (1..$len){$fname.='f'};
 system("dd if=/dev/zero of=outfile bs=1024 count=$size > /dev/null 2>&1");
 unless(-e "outfile"){
  print("Error: Can not create output files\nTest: Failed");
  exit(0);
 };
 for my $i (1..$n){
  my $nam="$fname$i";
  system("ln -s outfile $nam");
 };
};
print("----------------------------------------\n");
if($OK){print "Test from job: OK!\n"}else{print "Test from job: Fail!\n"};
