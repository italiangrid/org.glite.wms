#!/usr/bin/perl -w

use Getopt::Std;
use strict;

#--------------------------------------------------------------
#                Long JDL testing
#
# Test script input options:
#    * -c WMS config file
#    * -r additional attributes (requirements) for JDL
#    * -j create JDL only
#    * -l path to the log file
#    * -n  N cycles for job monitoring  (default 300)
#    * -s sleep seconds before next job status checking (default 30)
#    * -h usage
#    * -e submit job with --debug option
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
#    * -z => AllowZippedISB=false
#    
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
my $deb=0; my $id; my ($s,$par); my @pars; my $stmsg=0;
my $jdlsize='';

                         #Input parameters
my %inp=(); #hash for input parameters
my %opt=(); getopts("c:r:l:n:s:v:i:o:d:jzhwe",\%opt);

if(defined $opt{h}){
 print qq{USAGE:
# Test script input options:
#    * -c WMS config file
#    * -r additional attributes (requirements) for JDL
#    * -j create JDL only
#    * -l path to the log file
#    * -n  N cycles for job monitoring  (default 300)
#    * -s sleep seconds before next job status checking (default 30)
#    * -h usage
#    * -e submit job with --debug option
###### JDL generation parameters:
#    * -v Environment params:
#         -v <N pairs>:<length of the parname and value>
#    * -i InputSandbox params(local files):
#         -i <N files>:<file size in KB>:<flie name length>
#    * -o OutputSandbox params:
#         -o <N files>:<file size in KB>:<flie name length>
#    * -d data with InputSandboxBaseURI
#         -d <N files>:<file size in KB>:<BaseURI>
#    * -z => AllowZippedISB=false
 };
 exit($retOK);
};

if(defined $opt{c}){$inp{confWms}=$opt{c}}; #wms config file
if(defined $opt{r}){$inp{reqJdl}=$opt{r}}; #additional parameters for jdl
if(defined $opt{l}){$inp{logfile}=$opt{l}}; #log file

              #Job monitoring time parameters
if(defined $opt{d}){$inp{sleep}=$opt{d}}   #sleep seconds
else{$inp{sleep}=30};
if(defined $opt{n}){$inp{sleepcnt}=$opt{n}}#number of cycles of status monitoring
else{$inp{sleepcnt}=300};
              #JDL generation parameters
if(defined $opt{v}){ #-------------- environment
 $s=$opt{v}; @pars=split ':',$s;   #<N pairs>:<length of the parname and value>
 unless( checkPars($pars[0],$pars[1])){
  printMsg("Environment parameters error",2);
  exit($retInp);
 };
 $inp{env}=[$pars[0],$pars[1]];
};
if(defined $opt{i}){ #-------------- InputSandbox parameters
 $s=$opt{i}; @pars=split ':',$s;   #<N files>:<file size in KB>:<file name length>
 unless( checkPars($pars[0],$pars[1])){
  printMsg("InputSandbox parameters error",2);
  exit($retInp);
 };
 $inp{inpBox}=[$pars[0],$pars[1],$pars[2]];
};
if(defined $opt{o}){ #-------------- OutputSandbox parameters
 $s=$opt{o}; @pars=split ':',$s;   #<N files>:<file size in KB><file name length>:
 unless( checkPars($pars[0],$pars[1])){
  printMsg("OutputSandbox parameters error",2);
  exit($retInp);
 };
 $inp{outBox}=[$pars[0],$pars[1],$pars[2]];
};

if(defined $opt{j}){$inp{jdlOnly}=1};  #create JDL only
if(defined $opt{z}){$inp{zip}=1};  #AllowZippedISB=false
$inp{cmdtype}='cmdProxy';

sub checkPars{
 my ($a1,$a2)=@_;
 unless( $a1=~/^\d+$/){return 0};
 unless( $a2=~/^\d+$/){return 0};
 return 1
}
#--------------------------------------------------------------

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
#   Used commands:
my ($cmdSubmit,$cmdCancel,$cmdStatus,$cmdJobOutput,$cmdGetChkpt);
if($inp{cmdtype} eq 'cmdProxy'){
 $cmdSubmit='glite-wms-job-submit -a  ';
 $cmdCancel='glite-wms-job-cancel --noint ';
 $cmdStatus='glite-wms-job-status ';
 $cmdJobOutput="glite-wms-job-output --noint --dir $tmpdir/out ";
}else{
 $cmdSubmit='glite-job-submit ';
 $cmdCancel='glite-job-cancel --noint ';
 $cmdStatus='glite-job-status ';
 $cmdJobOutput="glite-job-output --noint --dir $tmpdir/out ";
};
if(defined $opt{e}){$cmdSubmit.='--debug '};
#--------------------------------------------------------------
#                Create JDL
my $timjdl=time();
my $tim=getTime(); 
my $jdlname="$tmpdir/long.jdl"; #JDL file name
# Create JDL file
printMsg("------------------------ Long JDL job testing $tim\n",1);
printMsg("------------------------ Start JDL file creation\n",1);
my $jdl=createJdl(); #create JDL file
$timjdl=time()-$timjdl;
unless($jdl){
 printMsg("Can not create JDL",2);
 exit($retFail);
};
$tim=getTime();
printMsg("$jdl",1);
printMsg("$tim: JDL has been created successfully",1);

printMsg("------------------------",1);
printMsg("$jdlsize",1);
if(exists $inp{jdlOnly}){
 clearTmpDir(); 
 exit
};

#--------------------------------------------------------------
#    Run job
my $timrun=time();
unless($deb){
$id=runJob($jdlname); #submit  job
$timrun=time()-$timrun;
};
unless($id){    #something wrong
 printMsg("Can't submit job ",2);
 $tim=getTime(); printMsg("$tim: Test failed",2);
 clearTmpDir(); exit($retFail);
};
printMsg("JDL creation time=$timjdl sec",1);
printMsg("Job submission time=$timrun sec",1);

chomp($id);
system(qq{echo "$id" > $tmpdir/jobID});
$tim=getTime();
printMsg("$tim Long JDL job submitted\n ID=$id",1);
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
printMsg("Long JDL job finished with status $st.",1);
if($st=~/Succes/){ #try retrieve std output and error
 my $ret=getOutput(); $OK=0 if $ret;
};
printMsg("================================================",1);
printMsg("$jdlsize",1);
clearTmpDir();

unless($OK){
 $tim=getTime(); printMsg("$tim: Test failed",2);
 exit($retFail);
};
printMsg("Test OK",1);
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
  if($st=~/Done/ || $st=~/Exit/ || $st=~/Abort/ || $st=~/Cancel/){return 0};
  sleep($inp{sleep});
  my $cmd="glite-wms-job-logging-info -v 2 $id 2>&1 | grep -B 3 FAILED |";
  my $resub=0;
  open(INP,"$cmd");
  while(my $s=<INP>){
   if($s=~/reason/){$s=~/=\s*(.*)\s*$/; $resub=$1; last};
  };
  close(INP);
  if($resub){
   $status="Done(Failed)\nReason=$resub\n"; return 0;
  };
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
#             Create jdl file job
#--------------------------------------------------------------
sub createJdl{
 my $jdl=qq{[
 Executable ="ljob.pl";
 StdError="stderr.log";
 StdOutput="stdout.log";
 RetryCount=0;
 ShallowRetryCount=0;
 Requirements=RegExp(".*cern.*",other.GlueCEUniqueID);
 };
 if(exists $inp{zip}){$jdl.="AllowZippedISB=false;\n"};

 my $arg='';
              #Environment
 my $env=getEnvironments(\$arg);
 if($env){$jdl.=$env; $jdlsize.="Environment length: ".length($env)."\n"};
              #InputSandbox
 my $inpSB=qq~InputSandbox={"file://ljob.pl"~;
 my ($ret,$r)=getInputSandbox(\$arg);
 unless($ret){return 0};
 if($r){$inpSB.="$r\n"};
 $inpSB.=" };\n";
 $jdl.=$inpSB; $jdlsize.="InputSandbox length: ".length($inpSB)."\n";
              #OutputSandbox
 my $outSB=qq~OutputSandbox={"stdout.log","stderr.log"~;
 $r=getOutputSandbox(\$arg);
 if($r){$outSB.=$r};
 $outSB.="\n };\n";
 $jdl.=$outSB; $jdlsize.="OutputSandbox length: ".length($outSB)."\n";

 if($arg){$jdl.=qq{Arguments="$arg";\n}};
 

 if(defined($inp{reqJdl})){$jdl.="$inp{reqJdl}\n"};
 $jdl.="]\n";
 open(OUT,">$jdlname") or return 0;
 print(OUT $jdl);
 close(OUT);
 $jdlsize.="JDL file length: ".length($jdl)."\n";
 return $jdl;
}
sub getEnvironments{
 return 0 unless exists $inp{env};
 my $arg=shift;
 my $n=$inp{env}[0]; my $len=$inp{env}[1]; my $env="Environment={\n";
 my $nam='nam_'; my $val='val_';
 for my $i (1..$len){$nam.='n'; $val.='v'};
 for my $i (1..$n){
  $env.=qq{"$nam$i=$val$i"};
  $env.=',' unless $i==$n;
  $env.="\n";
 };

 $env.="};\n";
 $$arg.=" -v $n:$len";
 return $env;
}
sub getInputSandbox{
 return (1,0) unless exists $inp{inpBox};
 my $arg=shift;
 my $n=$inp{inpBox}[0]; my $size=$inp{inpBox}[1]; my $len=$inp{inpBox}[2];
 my $fname='finp_';
 for my $i (1..$len){$fname.='f'};
 my $inpSB='';
 for my $i (1..$n){
   my $nam=",\n\"file://$tmpdir/$fname$i\"";   $inpSB.="$nam";
 };
 my $r=createInpFiles($fname,$size,$n);
 return (0,0) unless $r;
 $$arg.=" -i $n:$size:$len";
 return (1,$inpSB);
}

sub createInpFiles{
 my ($fname,$size,$n)=@_;
 system("dd if=/dev/zero of=$tmpdir/inpfile bs=1024 count=$size > /dev/null 2>&1");
 unless(-e "$tmpdir/inpfile"){
  printMsg("Can not create input files",1);
  return 0;
 };
 for my $i (1..$n){
  my $nam="$tmpdir/$fname$i";
  system("cp $tmpdir/inpfile $nam");
 };
 return 1;
}
sub getOutputSandbox{
 return 0 unless exists $inp{outBox};
 my $arg=shift;
 my $n=$inp{outBox}[0]; my $size=$inp{outBox}[1]; my $len=$inp{outBox}[2];
 my $fname='fout_';
 for my $i (1..$len){$fname.='f'};
 my $outSB='';
 for my $i (1..$n){
   my $nam=",\n\"$fname$i\"";   $outSB.="$nam";
 };
 $$arg.=" -o $n:$size:$len";
 return $outSB;
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
  printMsg("=================================== Check std output files",1);
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
 printMsg("=================================== Check generated output files",1);
 if(exists $inp{outBox}){
  my $n=$inp{outBox}[0]; my $size=$inp{outBox}[1]; my $len=$inp{outBox}[2];
  my $fname='fout_';
  for my $i (1..$len){$fname.='f'};
  my $ok=1;
  for my $i (1..$n){
   unless(-e "$outdir/$fname$i"){
    $ok=0; printMsg("Error: File $fname$i not retrieved",2);
   };
  };
  if($ok){
   printMsg("Check OK",1);
  }else{printMsg("Check Failed",2); $ret=1};
 };
 system(qq{rm "$outdir/stdout.log"}) if -e "$outdir/stdout.log";
 system(qq{rm "$outdir/stderr.log"}) if -e "$outdir/stderr.log";
 system(qq{rm "$tmpdir/retrievelog"}) if -e "$tmpdir/retrievelog";
 #rmdir($outdir);
 return $ret;
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
 
 if(-e "$tmpdir/out"){
  clearDir("$tmpdir/out");  #clear dir for output files
 };
 clearDir("$tmpdir");      #clear tmp dir
 rmdir "$tmpdir";
}
sub clearDir{
 my $dir=shift;
# if(-e $dir){  
#  `rm -f $dir/*`;
#  `rmdir $dir`;
# };

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
