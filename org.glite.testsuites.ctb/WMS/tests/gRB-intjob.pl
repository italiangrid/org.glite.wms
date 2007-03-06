#!/usr/bin/perl -w

#--------------------------------------------------------------
#                Interactive job testing
#
#   The test script does the following:
# 1. Generation of the JDL file for interactive job using input parameters
# 2. Submit  job (glite-job-submit --nolisten)
# 3. Status monitoring
# 4 Job status monitoring
# 5 Provide dialog with the submitted job
# 6 Retrive job output file
# 7 Remove pipes and shadow processes 
#
# Test script input options:
#    * -c WMS config file
#    * -r additional attributes (requirements) for JDL
#    * -t how many times repeat dialog transaction (write - read) (default 2)
#    * -j timeout for job (default 180 sec)
#    * -d sleep seconds before next status checking (default 30)
#    * -s N cycles status checking before timeout (default 200)
#    * -l path to the log file 
#--------------------------------------------------------------

use strict;
use Getopt::Std;


#------------------------------------
#  return code
#------------------------------------
my $retOK=0;
if(defined $ENV{SAME_OK}){$retOK=$ENV{SAME_OK}};
my $retFail=1;   #test failed
if(defined $ENV{SAME_ERROR}){$retFail=$ENV{SAME_ERROR}};
my $retWar=2;    #test warning
if(defined $ENV{SAME_WARNING}){$retWar=$ENV{SAME_WARNING}};
my $retInp=10;   #something wrong in input. Can't run
if(defined $ENV{SAME_ERROR}){$retInp=$ENV{SAME_ERROR}};
#------------------------------------
#  Working directory
#------------------------------------
my $basedir;
if(defined $ENV{INT_BASE_DIR}){$basedir=$ENV{INT_BASE_DIR}}
else{$basedir=$ENV{PWD}};
my $tmpdir;
if(defined $ENV{INT_TMP_DIR}){$tmpdir="$ENV{INT_TMP_DIR}/tmp$$"}
else{$tmpdir="$basedir/tmp$$"};
my $bindir;
if(defined $ENV{INT_BIN_DIR}){$bindir=$ENV{INT_BIN_DIR}}
else{$bindir=$basedir};

#------------------------------------
#  Input parameters
#------------------------------------
my %inp=(); #hash for input parameters
my %opt=(); getopts("c:r:l:d:s:t:j:w",\%opt);

if(defined $opt{c}){$inp{confWms}=$opt{c}}; #wms config file
if(defined $opt{r}){$inp{reqJdl}=$opt{r}}; #additional parameters for jdl
if(defined $opt{l}){$inp{logfile}=$opt{l}}; #log file
              #Job monitoring time parameters
if(defined $opt{d}){$inp{sleep}=$opt{d}}   #sleep seconds
else{$inp{sleep}=30};
if(defined $opt{s}){$inp{sleepcnt}=$opt{s}}#number of cycles of status checking
else{$inp{sleepcnt}=200};
              #Number of output from test to job (0=> waiting for the job output only)
if(defined $opt{t}){$inp{ncycles}=$opt{t}}
else{$inp{ncycles}=2};

if(defined $opt{j}){$inp{jtimeout}=$opt{j}}
else{$inp{jtimeout}=120};

if(defined $opt{w}){$inp{cmdtype}='cmdProxy'}
else{$inp{cmdtype}='cmdNS'};

#------------------------------------
#  Used commands
#------------------------------------
my ($cmdSubmit,$cmdCancel,$cmdStatus,$cmdJobOutput);
if($inp{cmdtype} eq 'cmdProxy'){
 $cmdSubmit='glite-wms-job-submit -a --nolisten ';
 $cmdCancel='glite-wms-job-cancel --noint ';
 $cmdStatus='glite-wms-job-status ';
 $cmdJobOutput="glite-wms-job-output --noint --dir $tmpdir ";
}else{
 $cmdSubmit='glite-job-submit --nolisten --nogui ';
 $cmdCancel='glite-job-cancel --noint ';
 $cmdStatus='glite-job-status';
 $cmdJobOutput="glite-job-output --noint --dir $tmpdir ";
};
#------------------------------------
#  Create tmp directory
#------------------------------------
my $stmsg=0; my $outdir;
my $jdlname="$tmpdir/intjdl.jdl";
my $ex=createTmpDir();
unless($ex){
 printMsg("Can't create tmp directory",2);
 exit($retFail);
};
#------------------------------------
#  Create Jdl file
#------------------------------------

my $jdl=createJdl(); #create JDL file
unless($jdl){
 printMsg("Can't write Jdl file into temp directory",2);
 exit($retFail);
};
my $tim=getTime();
printMsg("------------------------ Interactive job testing $tim\n",1);
printMsg("------------------------ JDL file was created:\n",1);
printMsg("$jdl",1);

#------------------------------------
#  Run interactive job 
#------------------------------------
my ($id,$port,$host,$shadowID,$pipein,$pipeout)=(0,0,0,0,0,0,0);;
printMsg("------------------------ Submit interactive job",1);
runJob($jdlname);
unless($id){         #job was't submitted
 printMsg("Submitting failed",2);
 cleanUp(0);  exit($retFail);
};
chomp($id);
system(qq{echo "$id" > $tmpdir/jobID});

printMsg("=================================================================",1);
#print "jobID=$id\nHost=$host\nShadowID=$shadowID\npipeIN=$pipein\npipeOUT=$pipeout\n";
#------------------------------------
#  Check pipes and shadow process
#------------------------------------
my $env=1;
my $rt=`ps -af | grep shadow.*$port | sed /grep/d | wc -l`;
chomp $rt;
unless($rt){ # no shadow process
 printMsg("No grid-console-shadow in memory",2);
 $env=0;
};
unless( -e $pipein){
  printMsg("Input pipe does not exists",2);
  $env=0;  
};
unless( -e $pipeout){
  printMsg("Output pipe does not exists",2);
  $env=0;
};
unless($env){
 printMsg("Test Failed",2);
 cleanUp(0);
 exit($retFail);
};

#------------------------------------
#  Run background process which communicate with job
#------------------------------------
printMsg("Run background process to communicate with job\n",1);
my $intout="$tmpdir/intproto.txt";
system("./intp.pl $inp{ncycles} $intout $shadowID < $pipeout > $pipein &");
#------------------------------------
#  Main cycle
#------------------------------------
my $tcnt=$inp{sleepcnt}; my $status=' '; my $dest=' '; my $ok=1; 
my $doDialog=1; my $dialogState=0; my $dlgMsg=' '; my $lendt=0;
$tim=getTime();  
printMsg("$tim: Start the main cycle:",1);
while($tcnt){
 $tcnt--; $tim=getTime();
 ($status,$dest)=getStatus();
 printMsg("$tim $tcnt Job: $status $dest",1);
 if($status=~/Abort/ or $status=~/Done \(Succes/ or $status=~/Cancell/){last};
 if($status=~/Running/ and $doDialog){
  $doDialog=Dialog();
 };
 sleep($inp{sleep});
};
my $d=`cat $intout`; chomp $intout;
printMsg("================= Test script dialog:",1);
printMsg("$d",1);

my $needcancel=0;
if($status=~/Done \(Succes/){
 getJobOutput($id);
 $rt=checkJobOutputFiles();
 printMsg("=====================================================\n",1);
 if($rt){$ok=0};
}else{printMsg("Timeout",2); $needcancel=1; $ok=0};

if($doDialog){
 printMsg("Dialog is not finished",2); $ok=0;
};
cleanUp($needcancel);
if($ok){
 printMsg("Test OK!",1);  exit($retOK);
}else{
 printMsg("Test failed!",2); exit($retFail);
};
#------------------------------------
#  Execute dialog
#------------------------------------
sub Dialog{
 my $out=checkOutput();
 unless(defined($out)){return 1};    #file is empty or no new data
 chomp $out;
 printMsg($out,1);
 if($out=~/ Dialog: /s){
  if($out=~/Failed/s){$ok=0}else{$ok=1};
  return 0;
 };
 return 1;
}

sub checkOutput{
 my $f=$intout;
 unless(-e $f){return};
 unless(-s $f){return};
 my $len=(-s $f);
 unless($len > $lendt){return};
 my $d=`cat $f`;
 $d=substr($d,$lendt);
 $lendt=$len;
 return $d;
}
######################################
#  Utilities
######################################
#------------------------------------
# Create JDL file
#------------------------------------
sub createJdl{
 my $jdl=qq~[
 JobType = "Interactive";
 Executable ="intjob.sh";
 Arguments = "$inp{jtimeout} $inp{ncycles}";
 InputSandbox = {"intjob.sh"};
 OutputSandbox={"out.txt"};
 ShallowRetryCount=5;
# Requirements=RegExp(".*lxb20.*",other.GlueCEUniqueID);
~;
 if(exists $inp{reqJdl}){$jdl.="$inp{reqJdl}\n"};
 $jdl.="]\n";
 open(OUT,">$jdlname") or return 0;
 print(OUT $jdl); close(OUT);
 return $jdl;
};
#------------------------------------
# Submit Interactive Job
#------------------------------------
sub runJob{
 my $jdl=shift; my $st=0;
 my $wms=' '; 
 if(exists $inp{confWms}){ #config file
  if($inp{cmdtype} eq 'cmdProxy'){$wms="-c $inp{confWms}"}
  else{$wms="--config-vo $inp{confWms}"};
 };
# $wms="-c $inp{confWms}" if exists $inp{confWms};
 my $cmd="$cmdSubmit $wms $jdl > $tmpdir/submitlog |";
# my $cmd="glite-job-submit $wms --nolisten --nogui --debug $jdl > $tmpdir/submitlog |";
 open(CMD,"$cmd");
 while(<CMD>){print}; close(CMD);
 open(INP,"$tmpdir/submitlog");
 while(my $s=<INP>){
  print "$s";
  if($s=~/successfully submitted/){$st=1; next};
  next if $st==0;
  if($s=~/https/){$s=~/(https.*)/; $id=$1; next};
  if($s=~/Host/){$s=~/Host:\s*(.*)/; $host=$1; next};
  if($s=~/Port/){$s=~/Port:\s*(.*)/; $port=$1; next};
  if($s=~/process Id:/){$s=~/(\d+)/; $shadowID=$1; next};
  if($s=~/Input/){$s=~/Input Stream  location:\s*(.*)/; $pipein=$1; next};
  if($s=~/Output/){$s=~/Output Stream  location:\s*(.*)/; $pipeout=$1; last};
 };
 close(INP);
}
#------------------------------------
# Get job status
#------------------------------------
sub getStatus{
 my $cmd="$cmdStatus $id 2>&1 |";
 my $st=0; my $dest=' ';
 open(INP,"$cmd");
 while(my $s=<INP>){
#   print "$s";
   if($s=~/Current Status:/){$s=~/:\s*(.*)/; $st=$1};
   if($s=~/Destin/){$s=~/Dest.*?:\s*(.*)/; $dest=$1;};
 };
 close(INP);
 return ($st,$dest);
}
#--------------------------------------------------------------
#        Retrieve job output
#--------------------------------------------------------------
sub getJobOutput{
 printMsg("-------------- Retrieve job output files",1);
# mkdir "$tmpdir/out";
 my $id=shift; my $ret='';
 my $cmd="$cmdJobOutput --logfile $tmpdir/retrievelog $id |";
 open(INP,"$cmd");
 my $st=0;
 while(my $s=<INP>){
  chomp $s; #printMsg("$s",1);

  if($s=~/successfully retrieved/){$st=1; next};
  if($st){
   $outdir=$s; chomp $outdir; $outdir=~s/\s//g;
   printMsg("Outdir=$outdir",1);
   $st=0;
  };
 };
 close(INP);
 return 0;
}
#--------------------------------------------------------------
#        Check and print job output
#--------------------------------------------------------------
sub checkJobOutputFiles{
 my $ret=0;
 unless(defined($outdir)){
  printMsg("Job output files were not retrieved",2); 
  $ret=1;
 }else{
   unless((-e "$outdir") && (-d "$outdir")){
    printMsg("No directory $outdir for output file",2); $ret=1;
   }else{
    unless(-e "$outdir/out.txt"){
     printMsg("Job output file was not retrieved",2); $ret=1;
    };
   };
 };
 if($ret){ # print glite-job-output logfile
  if(-e "$tmpdir/retrievelog"){
   $ret=`cat "$tmpdir/retrievelog"`; system(qq{rm "$tmpdir/retrievelog"});
  };
  return 1;
 };
 printMsg("================= Submitted job report file:",1); 
 open(INP,"$outdir/out.txt");  while(my $s=<INP>){chomp $s; printMsg($s,1)}; close(INP);
 
 system(qq{rm "$outdir/out.txt"});
 system(qq{rm "$tmpdir/retrievelog"}); 
 rmdir($outdir);
 return 0;
}
#------------------------------------
#   print messages(stdout and logfile)
#------------------------------------
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
#------------------------------------
#
sub getTime{   #return date and time
 my $t=`date +'%D %T'`; chomp $t;
 return $t;
}
#------------------------------------
#        Create tpm directory
#------------------------------------
sub createTmpDir{
 unless(-e $tmpdir){
  my $r=mkdir($tmpdir);
  unless($r){return 0};
 }else{
  clearTmpDir();
 };
 return 1;
}
#------------------------------------
#        Clean up tmp directory
#------------------------------------
sub clearTmpDir{
my $dir=$tmpdir;
if(-e $tmpdir){
 if(-e "$tmpdir/intout"){
  unlink("$tmpdir/intout"); 
 };
 if(-e $jdlname){
  unlink($jdlname);
 };
 rmdir "$tmpdir";
};
}
#------------------------------------
#        Clean up listener and pipes
#------------------------------------
sub cleanUp{
 my $mod=shift;
 my $prc=`ps -af | grep intp.pl | sed /grep/d`;
 chomp $prc;
 if($prc){    #kill background process
  $prc=~/(\d+)[^\d]/; $prc=$1;
  `kill $prc`;
 };
     #kill shadow processes
 my $cmd="ps -af | grep shadow.*$port | sed /grep/d |";
 open(INP,"$cmd");
 while(my $s=<INP>){
  $s=~/(\d+)[^\d]/; $prc=$1;
#  print "Kill shadow process $prc\n";
  system("kill $prc");
 };
 close(INP);
     #remove files and pipes
 if(-e $pipein){unlink($pipein)};
 if(-e $pipeout){unlink($pipeout)};
 if(-e "$tmpdir/submitlog"){unlink("$tmpdir/submitlog")};
 if(defined($intout) and -e "$intout"){unlink("$intout")};
 clearTmpDir();
# unless($ok){
#  if($id=~/https/){ 
#   printMsg("===================================================",1);
#   open(INP,"glite-job-logging-info -v 3 $id |");
#   while(my $s=<INP>){chomp $s; printMsg($s,1)}; close(INP);
#  };
# };
 if($mod){
  system("$cmdCancel $id 2>&1 >/dev/null") if $id=~/https/;
 };
}