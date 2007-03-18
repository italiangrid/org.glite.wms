#!/usr/bin/perl -w

use Getopt::Std;
use strict;

#--------------------------------------------------------------
#                Checkpointable job testing
#
#   The test script does the following:
# 1. Generation of the JDL file for Checkpointable job using input parameters
# 2. Submit  job (glite-wms-job-* or glite-job-* command)
#    The job try to compile C++ program which has to does the following:
#  - getCurrentStep
#  - do cycle from current step to last step (getNextStep) saving state as <item number>=<step name/number>
#  - if make command fail then run C++ job from InputSandbox
# 3. Status monitoring
# 4. Job status monitoring until job will get status Done/Abort/Canceled
# 5. Retrieve Checkpoint States saved by job using the command glite-wms-job-get-chkpt
# 6. Check Job Checkpoint States 
# 7. Retrive job output files
#
# Test script input options:
#    * -c WMS config file
#    * -r additional attributes (requirements) for JDL
#    * -t job steps type (1-number, 2- list of string default=2)
#    * -j N cycles for job monitoring  (default 300)
#    * -d sleep seconds before next status checking (default 30)
#    * -n N steps
#    * -s current step (default 1)
#    * -l path to the log file
#    * -w use glite-wms-* commands
#--------------------------------------------------------------
my $deb=0; my $id; 

#  Working directories:
my $basedir;
if(defined $ENV{CHK_BASE_DIR}){$basedir=$ENV{CHK_BASE_DIR}}
else{$basedir=$ENV{PWD}};
my $tmpdir;
if(defined $ENV{CHK_TMP_DIR}){$tmpdir="$ENV{CHK_TMP_DIR}/tmp$$"}
else{$tmpdir="$basedir/tmp$$"};
my $bindir;
if(defined $ENV{CHK_BIN_DIR}){$bindir=$ENV{CHK_BIN_DIR}}
else{$bindir=$basedir};

createTmpDir();
#--------------------------------------------------------------
#   return codes:

my $retOK=0;
if(defined $ENV{SAME_OK}){$retOK=$ENV{SAME_OK}};
my $retFail=1;   #test failed
if(defined $ENV{SAME_ERROR}){$retFail=$ENV{SAME_ERROR}};
my $retWar=2;    #test warning
if(defined $ENV{SAME_WARNING}){$retWar=$ENV{SAME_WARNING}};
my $retInp=10;   #something wrong in input. Can't run
if(defined $ENV{SAME_ERROR}){$retInp=$ENV{SAME_ERROR}};
#--------------------------------------------------------------
                         #Input parameters
my %inp=(); #hash for input parameters
my %opt=(); getopts("c:r:l:d:j:n:s:w",\%opt);

if(defined $opt{c}){$inp{confWms}=$opt{c}}; #wms config file
if(defined $opt{r}){$inp{reqJdl}=$opt{r}}; #additional parameters for jdl
if(defined $opt{l}){$inp{logfile}=$opt{l}}; #log file
              #Job monitoring time parameters
if(defined $opt{d}){$inp{sleep}=$opt{d}}   #sleep seconds
else{$inp{sleep}=30};
if(defined $opt{j}){$inp{sleepcnt}=$opt{j}}#number of cycles of status monitoring
else{$inp{sleepcnt}=300};

if(defined $opt{t}){$inp{tSteps}=$opt{t}} #steps string or numbers
else{$inp{tSteps}=2};
if(defined $opt{n}){$inp{nSteps}=$opt{n}} #n steps
else{$inp{nSteps}=5};
if(defined $opt{s}){$inp{cStep}=$opt{s}}
else{$inp{cStep}=2};

if(defined $opt{w}){$inp{cmdtype}='cmdProxy'}
else{$inp{cmdtype}='cmdNS'};

#--------------------------------------------------------------
#   Used commands:
my ($cmdSubmit,$cmdCancel,$cmdStatus,$cmdJobOutput,$cmdGetChkpt);
if($inp{cmdtype} eq 'cmdProxy'){
 $cmdSubmit='glite-wms-job-submit -a ';
 $cmdCancel='glite-wms-job-cancel --noint ';
 $cmdStatus='glite-wms-job-status ';
 $cmdJobOutput="glite-wms-job-output --noint --dir $tmpdir ";
 $cmdGetChkpt="glite-wms-job-get-chkpt ";
}else{
 $cmdSubmit='glite-job-submit ';
 $cmdCancel='glite-job-cancel --noint ';
 $cmdStatus='glite-job-status ';
 $cmdJobOutput="glite-job-output --noint --dir $tmpdir ";
 $cmdGetChkpt="glite-job-get-chkpt ";
};

#--------------------------------------------------------------
#        Check checkpointing parameters

my $stmsg=0;
if($inp{cStep}>$inp{nSteps}){
 printMsg("Error: Input parameter CurrentStep=$inp{cStep} > N steps= $inp{nSteps}",2);
 exit($retInp);
};
#--------------------------------------------------------------
my %jobsteps; #hash of the needed steps
if($inp{tSteps}==1){
 for my $i ($inp{cStep}..$inp{nSteps}){   $jobsteps{$i}=$i; }
}else{
 for my $i ($inp{cStep}..$inp{nSteps}){ 
  my $step="step$i"; $jobsteps{$i}=$step;
 };
};

#--------------------------------------------------------------
#                Create JDL
my $tim=getTime(); 
my $jdlname="$tmpdir/chk.jdl"; #JDL file name
# Create JDL file
printMsg("------------------------ Checkpointable job testing $tim\n",1);
printMsg("------------------------ JDL file is created:\n",1);
my $jdl=createJdl(); #create JDL file and input data files
printMsg("$jdl",1);

#--------------------------------------------------------------
#    Run job
if($deb){$id='https://lxb0744.cern.ch:9000/k3zFnQqNhEgn4Jox0Ll2jg'}
else{
$id=runJob($jdlname); #submit Checkpointable job
};
unless($id){    #something wrong
 printMsg("Can't submit job ",2);
 $tim=getTime(); printMsg("$tim: Test failed",2);
 clearTmpDir(); exit($retFail);
};
chomp($id);
system(qq{echo "$id" > $tmpdir/jobID});
printMsg("$tim Checkpointable job submitted (N steps=$inp{nSteps}, CurrentStep=$inp{cStep})\n ID=$id",1);
printMsg("=================================== Start job status monitoring",1);
#--------------------------------------------------------------
#    Job monitoring
my ($status, $dest, $ecode, $rea)=qw(? ? ? ?);
my $st=Monitor();  #monitoring job execution

if($st){ # job not finished (timeout)
 printMsg("Timeout: Job not finished",2);
 printMsg("Job $id is canceling",1);
 system("$cmdCancel $id"); clearTmpDir();
 $tim=getTime(); printMsg("$tim: Test failed",2);
 exit($retFail);
};
#--------------------------------------------------------------
#   Check job status
#--------------------------------------------------------------
my $OK=1;
$st=$status;
unless($st=~/Success/){ 
 $OK=0;
 printStatus();
};
printMsg("Checkpointable job finished with status $st.",1);
if($st=~/Done/){ #try retrieve std output and error
 my $ret=getOutput(); $OK=0 if $ret;
};
unless($OK){
 clearTmpDir();
 $tim=getTime(); printMsg("$tim: Test failed",2);
 exit($retFail);
};
#--------------------------------------------------------------
#    Get and check subjobs checkpoints
#--------------------------------------------------------------

#printMsg("------------------------ Check checkpoints:",1);
my $ret=checkChkpts();
unless($ret){
  printMsg("Check failed!!!",2);
  $tim=getTime();
  printMsg("$tim: Test failed",2);
  exit($retFail);
}else{
  printMsg("Check OK!",1);
};
$tim=getTime();
printMsg("$tim: Test OK",1);
clearTmpDir(); 
exit($retOK);
#--------------------------------------------------------------
#             Utilities
#
#--------------------------------------------------------------
#   Final jobs status
#--------------------------------------------------------------
sub printStatus{
printMsg("------------------------ Final status:\n",1);
open(INP,"$cmdStatus $id |");
while(my $s=<INP>){
 chomp $s; printMsg($s,1);
};
close(INP);
}
#--------------------------------------------------------------
sub printMsg{
 my $msg=shift; my $tp=shift; my $msgType;
 my $nl="\n";
 if(defined $ENV{SAME_OK}){  #SAM environment
  $nl='<br>';
  $msg=~s/\n/\<br\>/sg;
 };
 if($tp==1){
  unless($stmsg==1){
   if(defined $ENV{SAME_OK}){print "<font color=black><br>"};
   $msg="============ Info =============$nl$msg";
   $stmsg=1; 
  };
 } #info message
 elsif($tp==2){
  unless($stmsg==2){
   if(defined $ENV{SAME_OK}){print "<font color=red><br>"};
   $msg="============ Error =============$nl$msg";
   $stmsg=2; 
  }; 
 }    #error message
 elsif($tp==3){
  unless($stmsg==3){
   if(defined $ENV{SAME_OK}){print "<font color=blue><br>"};
   $msg="============ Warning =============$nl$msg";
   $stmsg=3; 
  };
 }  #warning message
 print("$msg$nl");
 if(exists $inp{logfile}){
  my $f="$inp{logfile}";
  `echo "$msg" >> $f`;
 };
}
#--------------------------------------------------------------
#      Job status monitoring
#--------------------------------------------------------------
sub Monitor{
  my $s; my $st; my $dest;
 my $tcnt=$inp{sleepcnt};
 while($tcnt){ 
  $tcnt--;
  ($st,$dest)=getStatus();
  $status=$st;
  $tim=getTime();
  printMsg("$tim $tcnt Job: $st $dest",1); 
  if($st=~/Done/ || $st=~/Abort/ || $st=~/Cancel/){return 0};
  sleep($inp{sleep});
 }
 return 1;
}
#------------------------------------
# Get job status
#------------------------------------
sub getStatus{
 my $cmd="$cmdStatus $id 2>&1 |";
 my $st=0; my $dest=' ';
 open(INP,"$cmd");
 while(my $s=<INP>){
   if($s=~/Current Status:/){$s=~/:\s*(.*)/; $st=$1};
   if($s=~/Destin/){$s=~/Dest.*?:\s*(.*)/; $dest=$1;};
 };
 close(INP);
 return ($st,$dest);
}
##--------------------------------------------------------------
#             Create jdl file for checkpointable job
#--------------------------------------------------------------
sub createJdl{
 my $jdl=qq~[
 JobType = "Checkpointable";
 Executable ="chkptmake.sh";
 StdOutput = "stdout.log";
 StdError = "stderr.log";
 OutputSandbox = {"stdout.log", "stderr.log"};
 InputSandbox={"file://chkjob.cpp","file://chkptmake.sh","file://makeWN","chkjob1"};
 CurrentStep=$inp{cStep};
~;
my $jsteps='JobSteps=';
if($inp{tSteps}==1){$jsteps.="$inp{nSteps};\n"}
else{
 $jsteps.='{';
 for my $i (1..$inp{nSteps}){$jsteps.="\"step$i\","};
 chop $jsteps; $jsteps.="};\n";
};
$jdl.=$jsteps;
if(defined($inp{reqJdl})){$jdl.="$inp{reqJdl}\n"};
$jdl.="]\n";
 open(OUT,">$jdlname") or return 0;
 print(OUT $jdl);
 close(OUT);
 return $jdl;
}
#--------------------------------------------------------------
#     Run job and get jobID
#--------------------------------------------------------------
sub runJob{
 my $jdl=shift; 
 my $wms=' ';
 if(exists $inp{confWms}){ #config file
  if($inp{cmdtype} eq 'cmdProxy'){$wms="-c $inp{confWms}"}
  else{$wms="--config-vo $inp{confWms}"};
 };
 my $cmd="$cmdSubmit $wms $jdl 2>&1 |";
# open(INP,"glite-wms-job-submit -a --nomsg $jdl 2>&1 |");
# print "$cmd\n"; exit;
 open(INP,"$cmd");
 my $id='';
 while(my $s=<INP>){
  chomp $s;
  printMsg($s,1);
  next if $s=~/^\s*$/;
  $id.="$s\n"; 
 };
 close(INP);
 if($id=~/successfully submitted/){
  $id=~/.*(https.*?)$/ms; $id=$1;
  return $id;
 };
 return 0;
}
#--------------------------------------------------------------
#        Retrieve and check job output
#--------------------------------------------------------------
sub getOutput{
 printMsg("=================================== Retrieve output files",1);
 my $ret=''; my $outdir;
 my $cmd="$cmdJobOutput --logfile $tmpdir/retrievelog $id |";
 open(INP,"$cmd");
 my $st=0;
 while(my $s=<INP>){
  chomp $s; #printMsg("$s",1);

  if($s=~/successfully retrieved/){$st=1; next};
  if($st){
   $outdir=$s; chomp $outdir; $outdir=~s/\s//g;
   #printMsg("Outdir=$outdir",1);
   $st=0;
  };
 };
 close(INP);
 unless(defined($outdir)){
  printMsg("Job output files were not retrieved",2);
  $ret=1;
 }else{
   unless((-e "$outdir") && (-d "$outdir")){
    printMsg("No directory $outdir for output files",2); $ret=1;
   }else{
    unless(-e "$outdir/stdout.log" and -e "$outdir/stderr.log"){
     printMsg("Job output files were not retrieved",2); $ret=1;
    };
   };
 };
 if($ret){ # print glite-job-output logfile
  if(-e "$tmpdir/retrievelog"){
   $ret=`cat "$tmpdir/retrievelog"`; system(qq{rm "$tmpdir/retrievelog"});
  };
  return 1;
 };
 printMsg("Retrieving OK",1);
  printMsg("=================================== Check output files",1);
 if(-s "$outdir/stderr.log"){ # There are errors
  $ret=1;
  printMsg("-------------- There are messages in the StdError file:",1);
  open(INP,"$outdir/stderr.log");  while(my $s=<INP>){chomp $s; printMsg($s,1)}; close(INP);
 };
 if(-s "$outdir/stdout.log"){
  printMsg("-------------- Submitted job report file:",1);
  open(INP,"$outdir/stdout.log");  
  while(my $s=<INP>){
   chomp $s; printMsg($s,1)
  }; 
  close(INP);
 };
 
 system(qq{rm "$outdir/stdout.log"}) if -e "$outdir/stdout.log";
 system(qq{rm "$outdir/stderr.log"}) if -e "$outdir/stderr.log";
 system(qq{rm "$tmpdir/retrievelog"}) if -e "$tmpdir/retrievelog";
 #rmdir($outdir);
 return $ret;
}
#--------------------------------------------------------------
#        Get and check subjobs checkpoints
#--------------------------------------------------------------
sub checkChkpts{
 printMsg("=================================== Retrive checkpoints from LB",1);
 my $item; my $step;
 my $start; my %chkpt; my $s;
 my $cmd="$cmdGetChkpt $id |";
 my $state=0;  my $currentstep=0;
  open(INP,"$cmd");   #job-get-chkpt
  while($s=<INP>){  
   chomp $s;
   printMsg("$s",1);
   if($s=~/successfully/){$state=1; next};   
   next unless $state;
   if($s=~/CurrentStep/){$s=~/.*=\s*(.*)\s*/; $currentstep=$1;};
   if($s=~/UserData/){$state=2; next};
   next unless $state==2;
   if($s=~/item/){$s=~/item(\d+).*=\s*\"(.*)\"/; $item=$1; $step=$2; $chkpt{$item}=$step; 
#    print "item=$item, step=$step\n";
   };
  };
  close(INP);
  unless($state==2){
   printMsg("Error: Retrieving failed",2); return 0;
  };
  
  printMsg("=================================== Check job user data and checkpointing",1);
         #print Job steps
 printMsg("---------- Job steps must be:",1);
 $s='';
 for my $i (sort {$a <=> $b} keys %jobsteps){$s.=$jobsteps{$i}.','};
 chop $s;
 printMsg("$s",1);
 printMsg("---------- Job got the following steps:",1);
 $s='';
 for my $i (sort {$a <=> $b} keys %chkpt){$s.=$chkpt{$i}.','};
 chop $s;
 printMsg("$s",1);
         #check correctness job chkpts- all chkpts were passed
 for my $i ($inp{cStep}..$inp{nSteps}){
  $step=$jobsteps{$i}; 
  my $j=$i-$inp{cStep}+1; #print "1:$i,$step,$chkpt{$j};\n";
  return 0 unless exists $chkpt{$j};
  unless($step eq $chkpt{$j}){return 0};
  $chkpt{$j}=0;
 }; 
 return 1;
}
#--------------------------------------------------------------
sub getTime{   #return date and time
 my $t=`date +'%D %T'`; chomp $t;
 return $t;
}
#--------------------------------------------------------------
#        Create tmp directory
#--------------------------------------------------------------
sub createTmpDir{
 unless(-e $tmpdir){
  my $r=mkdir($tmpdir);
  unless($r){
   printMsg("Can't create tmp directory",2);
   exit($retInp);
  };
 }else{
  clearTmpDir();
 };
}
#--------------------------------------------------------------
#        Clear tmp directories
#--------------------------------------------------------------
sub clearTmpDir{
 clearDir("$tmpdir/out");  #clear dir for output files
 clearDir("$tmpdir");      #clear tmp dir
 rmdir "$tmpdir";
}
sub clearDir{
my $dir=shift;
if(-e $dir){  
 open(INP,"ls $dir |");
 while(my $f=<INP>){
  chomp $f; 
  if(-d "$dir/$f"){
   `rm -f $dir/$f/*`;
   #print "Remove: rm -f $dir/$f/*\n";
   rmdir "$dir/$f";
   #print "Remove: rmdir $dir/$f\n";
  }elsif(-f "$dir/$f"){
   unlink "$dir/$f";
   #print "Remove: unlink $dir/$f"; 
  };
 };
 close(INP);
}
}
#--------------------------------------------------------------
#        Resubmition reason
#--------------------------------------------------------------

sub getResub{
my $id=shift; my $rh=shift;

my ($dest,$ecode,$rea)=();
my $cmd="glite-wms-job-logging-info -v 3 $id |";
my $st=1; #1-Event: Match, 2-dest_id, 3-Event: Done, 4-exit_code,5-reason

open(INP,"$cmd");
while(my $s=<INP>){
 if($st==1){if( $s=~/Event: Match/){$st=2; next}}
 elsif($st==2){ if($s=~/dest_id/){$s=~/dest_id\s*=\s*(.*)\s*/; $dest=$1; $st=3; next}}
 elsif($st==3){ if($s=~/Event: Done/){$st=4; next}}
 elsif($st==4){ if($s=~/exit_code/){$s=~/exit_code\s*=(.*)\s*/; $ecode=$1; $st=5; next}}
 elsif($st==5){
  if($s=~/reason/){
   $s=~/reason\s*=(.*)\s*/; $rea=$1; next if $rea=~/nil/;
   if($ecode){$st=1;
   }else{
     if($rea=~/nil/){$st=3; next};
     $st=0;
   };
   unless(exists $rh->{$dest}){$rh->{$dest}=[0,[0]]};
   my $n=$rh->{$dest}[0]; $n++; $rh->{$dest}[0]=$n;
   $n=$rh->{$dest}[1][0]; $n++; $rh->{$dest}[1][$n]=$rea; $rh->{$dest}[1][0]=$n;
   unless($st){last};
  };
 }
};
close(INP);
}
