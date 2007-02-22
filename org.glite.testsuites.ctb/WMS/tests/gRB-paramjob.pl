#!/usr/bin/perl -w

use Getopt::Std;
use strict;

#--------------------------------------------------------------
#                Parametric job testing  
#
#   The test script does the following:
# 1. Generation of the JDL file for parametric job using input parameters
# 2. Submit parametric job
# 3. Status monitoring
# 4. Retrieve output files for each node.
# 5. Check correctness of the following:
# - the parametric names and the number of the nodes submitted
# - parametric Arguments for executable (proper argument for each node)
# - parametric OutputSandbox(proper named output files from each node)
# - parametric InputSandbox (proper named file  for each node)
#
# Test is OK if all nodes have status "Done (Success)"  and
# all checking OK.
#
#   Input:
# -c <WMS config file>
# -p <Parameters config file> 
# -j  if test used for JDL generation only
#        Following options used if -p options is absent
# -n N subjobs                               
# -t parametric type: 1-string, 2-numbers (default 1)
# -d sleep seconds before next status checking (default 30)
# -s N cycles status checkinng before timeout  (default 200)
# -l path to the log file
# -m N cycles by 15 sec waiting for the main job get status Done
#--------------------------------------------------------------

sub Usage{
 print qq{
  Usage: 
  prmtest.pl <-c WMS config file> <-p Parameters config file> -j
  or
  prmtest.pl <-n N subjobs> <-t parametric type> <-l logfile>
 };
}
#--------------------------------------------------------------
#  Working directories:
my $basedir;
if(defined $ENV{PRM_BASE_DIR}){$basedir=$ENV{PRM_BASE_DIR}}
else{$basedir=$ENV{PWD}};
my $tmpdir;
if(defined $ENV{PRM_TMP_DIR}){$tmpdir=$ENV{PRM_TMP_DIR}}
else{$tmpdir="$basedir/tmp$$"};
my $bindir;
if(defined $ENV{PRM_BIN_DIR}){$bindir=$ENV{PRM_BIN_DIR}}
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
my %opt=(); getopts("c:p:n:t:d:s:l:m:j",\%opt);

if(defined $opt{c}){$inp{confWms}=$opt{c}}; #wms config file
if(defined $opt{p}){$inp{confPar}=$opt{p}}; #parameters file
if(defined $opt{j}){$inp{jdlOnly}=1};       #Generate JDL file only
if(defined $opt{l}){$inp{logfile}=$opt{l}}; #log file

if (exists $inp{confPar}){  #parameters are defined in the file
 my $rt=getConfigData($inp{confPar});
 if($rt){ Usage();  exit($retInp); };
}else{
 if(defined $opt{n}){$inp{nJobs}=$opt{n}} # N subjobs
 else{$inp{nJobs}=2}; #default N jobs=2
 if(defined $opt{t}){$inp{typePar}=$opt{t}} #parametric is strings or numbers
 else{$inp{typePar}=1}; #default type is strings
 if(defined $opt{d}){$inp{sleep}=$opt{d}}   #sleep seconds
 else{$inp{sleep}=30};
 if(defined $opt{s}){$inp{sleepcnt}=$opt{s}}#number of cycles of status checking
 else{$inp{sleepcnt}=200};
 if(defined $opt{m}){$inp{waitDone}=$opt{m}} #wait for main job will get status Done (15*10 sec)
 else{$inp{waitDone}=15};
 if( $inp{typePar}==2){# define attr Parameters,ParamStart, ParamStep
  $inp{Param}=$inp{nJobs};      #JDL Parameters attribute
  $inp{ParamStart}=0; #JDL ParameterStart attribute
  $inp{ParamStep}=1;  #JDL ParameterStep attribute
 }else{$inp{strVal}='job';}; #string for string list parametric(job1,job2...)
 $inp{expandJdl}="\n"; #additional attributes for jdl
};
#--------------------------------------------------------------
#   Used commands:
my $cmdSubmit='glite-wms-job-submit -a --nomsg';
my $cmdStatus='glite-wms-job-status';
my $cmdJobOutput='glite-wms-job-output --noint --dir';
my $cmdLoggingInfo='glite-wms-job-logging-info';
my $cmdCancel='glite-wms-job-cancel --noint ';
#--------------------------------------------------------------
my $tim=getTime(); 
my $jdlname="$tmpdir/prm.jdl"; #JDL file name
my %nods=();
my %state=();
my $stmsg=0;
my $jdlPar='';  #$jdlPar= jdl parametric attributes (Parameters,ParamStart,ParamStep)
my @params=();  #list of parameters (string or number list)
my $rt=defParams();    # define @params and $jdlPar
if ($rt){ #error in input parameters
 clearTmpDir(); exit($retInp);
}; 
#--------------------------------------------------------------
# Create JDL file
printMsg("------------------------ Parametric job testing $tim\n",1);
printMsg("------------------------ JDL file was created:\n",1);
my $jdl=createJdl(); #create JDL file and input data files
unless($jdl){
 printMsg("Can't create input data file",2);
 exit($retFail);
};
printMsg("$jdl",1);
exit($retOK) if $inp{jdlOnly};   #create JDL only

#--------------------------------------------------------------
#    Run job

my $id=runJob($jdlname); #submit parametric job
unless($id=~/https/){    #something wrong
 printMsg("Can't submit job ($id)",2);
 clearTmpDir(); exit($retFail);
};
chomp($id);
system(qq{echo "$id" > $tmpdir/jobID});
printMsg("$tim Parametric job submitted (N subjobs=$inp{nJobs})\n ID=$id",1);
#--------------------------------------------------------------
#    Job monitoring
my $n;
my $ejob=Monitor();  #monitoring job execution
if($ejob < $inp{nJobs}){ # not all jobs finished (timeout)
 $n=$inp{nJobs}-$ejob;
 printMsg("Timeout: $n subjobs not finished",2);
 printMsg("Job $id is canceling",1);
 for my $nod (sort keys %nods){
  printMsg("$nod ID=$nods{$nod}[1]",1);
 };
 system("$cmdCancel $id");
 clearTmpDir();
 $tim=getTime();
 printMsg("$tim: Test failed",2);
 exit($retFail);
};
#--------------------------------------------------------------
#   Final jobs status
printMsg("All subjobs finished.\nNow waiting for the main job get status Done",1);
my $cnt=$inp{waitDone}; my $sw;
while($cnt){
 $cnt--;
 open(INP,"$cmdStatus $id |");
 while($sw=<INP>){
  next unless $sw=~/Current Status:/;
  last;
 };
 close(INP);
 if($sw=~/Done/){last};
 sleep(15);
};
unless($cnt){ #time out
 printMsg("Timeout: Main job can't get status Done",2);
 printMsg("Job $id is canceling",1);
 system("$cmdCancel $id");
 clearTmpDir();
 $tim=getTime();
 printMsg("$tim: Test failed",2);
 exit($retFail);
};

printMsg("Parametric job finished. ID=$id\n",1);
printMsg("------------------------ Final status:\n",1);
open(INP,"$cmdStatus $id |");
while(my $s=<INP>){
 chomp $s; printMsg($s,1);
};
close(INP);
#--------------------------------------------------------------
#   Check correctness of the subjob names and their number

printMsg("------------------------ Check correctness of the subjob names and their number",1);
my @subNodes=sort keys %nods;
my $s1=join ',',@params;
my $s2=join ',',@subNodes;
unless($#subNodes==$#params){ #check number of nodes
 printMsg("Wrong nodes number:\n Must be $#params\n, we have $#subNodes",2);
 clearTmpDir(); exit($retFail);
};
for my $i (0..$#params){
 my $nod="Node_$params[$i]";
 unless(exists $nods{$nod}){
  printMsg("Wrong nodes names:\n Must be $#params\n, we have $#subNodes",2);
  clearTmpDir(); exit($retFail);
 };
};
printMsg("Check OK",1);
#--------------------------------------------------------------
#   Check subjobs status

my $nOK=0;
printMsg("------------------------ Check subjobs status:",1);
for my $st (sort keys %state){
 printMsg("Status: $st $state{$st} subjobs from $inp{nJobs}",1);
 if($st=~/Done \(Success\)/){$nOK=$state{$st}};
};
if($nOK<$inp{nJobs}){checkReason()}  #check unsuccess reason
else{printMsg("Check OK",1)};
#--------------------------------------------------------------
#    Get and check jobs output

if($nOK>0){
 printMsg("------------------------ Check subjobs output:",1);
 my $retOut=getJobsOutput($id); #get jobs output
 my $ret=checkJobsOutput(); #check
 if($ret){
  printMsg("Check failed!!!",2);
  $tim=getTime();
  printMsg("$tim: Test failed",2);
  exit($retFail);
 }else{
  printMsg("Check OK!",1);
 };
};
$tim=getTime();
printMsg("$tim: Test done success",1);
clearTmpDir(); 
exit($retOK);
#--------------------------------------------------------------
#             Utilities
#
#
#--------------------------------------------------------------
sub checkReason{
 printMsg("Show unsuccess reasons:",1);
 for my $nod (sort keys %nods){
  my $st=$nods{$nod}[2];
  next if $st=~/Done \(Success\)/;
  my $nid=$nods{$nod}[1]; #subjob ID
  printMsg("+++++++Subjob $nod, ID=$nid:",1);
  open(INP,"$cmdLoggingInfo -v 2 $nid | grep 'reason' |");
  while(my $s=<INP>){	
   next if $s=~/nil/ or $s=~/USER/;
   chomp $s;
   printMsg($s,1);
  };
  close(INP);
 }; 
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
sub defParams{
 if($inp{typePar}==1){  #string parameters
    #check nJobs has valid value
  if($inp{nJobs}=~/\D+/ or $inp{nJobs}==0){
   printMsg('Invalid nJobs value',2); return 1;
  };
  
  $jdlPar="Parameters={"; #Parametric attributes for JDL
  for my $i (1..$inp{nJobs}){
   my $p="$inp{strVal}$i"; 
   push @params,$p; #list of parameters
   $jdlPar.=$p; if($i==$inp{nJobs}){$jdlPar.="};"}else{$jdlPar.=','};
  };
 }else{                 #numbers parameters
    # check Param, ParamStart, ParamStep values
  if($inp{Param}=~/\D+/ or $inp{ParamStart}=~/\D+/ or $inp{ParamStep}=~/\D+/){
   printMsg('Invalid Param, ParamStart or ParamStep values',2); return 1;
  };
  
  my $nn=0;
  for(my $i=$inp{ParamStart}; $i<$inp{Param}; $i+=$inp{ParamStep}){push @params,$i; $nn++};
  $jdlPar="Parameters=$inp{Param};\n ParameterStart=$inp{ParamStart};\n ParameterStep=$inp{ParamStep};\n"; 
  $inp{nJobs}=$nn;
  if($nn==0){
   printMsg('Invalid Param, ParamStart or ParamStep values',2); return 1;
  };
 };
 return 0;
 #print "$jdlPar\n"; exit;
}
#--------------------------------------------------------------
#      Check jobs output
#--------------------------------------------------------------
sub checkJobsOutput{
 my $ret=0;
 my $outdir="$tmpdir/out";
 for my $nod (@params){   #for each node
  my $node="Node_$nod";   #node name
  my $dir="$outdir/$node";#directory with node output data
  unless(-e $dir){ #no output directory 
   $nods{$node}[8]=-1; 
   printMsg("$node: No directory for output files",3);
   $ret=1;
   next;
  }; 
      ############# Check StdOutput and StdError
  my $fname="$dir/stdout_$nod"; 
  unless(-e $fname){
   $nods{$node}[8]=-2;
   printMsg("$node: No StdOutput file",3);
   $ret=1;
  }else{ #output file contain of $nod value(argument for subjob)
    my $s=`cat $fname 2>&1`;
   unless($s=~/$nod/){#invalid arguments for subjob
    printMsg("$node: invalid arguments for subjob",3);
    $ret=1;
   };
  };
  $fname="$dir/stderr_$nod";
  unless(-e $fname){
   $nods{$node}[8]=-3;
   printMsg("$node: No StdError file",3);
   $ret=1;
  }else{
   if(-s $fname){  #StdError file has length > 0
    $nods{$node}[8]=-4;
    my $er=`cat $fname`;
    printMsg("$node: StdError file is not empty:",3);
    printMsg("$er",3);
    $ret=1;
   };
  };
    ############ Check Output sandbox
  $fname="$dir/outdata_$nod";
  unless(-e $fname){
   $nods{$node}[8]=-5;
   printMsg("$node: No Output data file",3);
   $ret=1;
  };
 };
 return $ret;
}
#--------------------------------------------------------------
#      Job status monitoring
#--------------------------------------------------------------
sub Monitor{
 my $ejob=0; my $s;
 while($inp{sleepcnt}){ 
  $inp{sleepcnt}--;
  checkStatus($id);     
  getCurrentJobStatus();
  $ejob=0;
  $s="$inp{sleepcnt}: ";
  for my $st(sort keys %state){
   $s.="$st: $state{$st} *** ";
   if($st=~/Done \(Success\)/ || $st=~/Abort/ || $st=~/Cancel/){$ejob+=$state{$st}};
  };
  printMsg("$s",1);
  if($ejob==$inp{nJobs}){last};
  sleep($inp{sleep});
 }
 return $ejob
}
#--------------------------------------------------------------
#             Create jdl file for parametric job and inp files
#--------------------------------------------------------------
sub createJdl{
 my $jdl=qq~[
 JobType = "Parametric";
 Executable ="paramjob.sh";
 Arguments = "_PARAM_";
 StdOutput="stdout__PARAM_";
 StdError="stderr__PARAM_";
 ShallowRetryCount=5;
 $jdlPar
 InputSandbox = {"file://$bindir/paramjob.sh","file://$tmpdir/inpdata__PARAM_"};
 OutputSandbox={"stdout__PARAM_","stderr__PARAM_","outdata__PARAM_"};
# Requirements=RegExp(".*lxb20*.*",other.GlueCEUniqueID);
 $inp{expandJdl}
] 
~;
 open(OUT,">$jdlname") or return 0;
 print(OUT $jdl);
 close(OUT);
 my $dat='0123456789';
# open(OUT,">$tmpdir/inpdata");
# print(OUT $dat); close(OUT);
 for my $i (0..$inp{nJobs}-1){
  my $f="inpdata_$params[$i]";
#  system("ln -s $tmpdir/inpdata $tmpdir/$f"); 
  open(OUT,">$tmpdir/$f") or return 0;
  print(OUT $dat);
  close(OUT);
#  print "$f,\n";
 };
 return $jdl;
}
#--------------------------------------------------------------
#      Total statistic for nodes
#--------------------------------------------------------------
sub getCurrentJobStatus{
 %state=();
 for my $nod (keys %nods){
  my $st=$nods{$nod}[2]; #node current status
  $state{$st}=0 unless exists $state{$st};
  $state{$st}+=1;
 };
}
#--------------------------------------------------------------
#     Run job and get jobID
#--------------------------------------------------------------
sub runJob{
 my $jdl=shift; 
 my $wms=' '; $wms="-c $inp{confWms}" if exists $inp{confWms};
 my $cmd="$cmdSubmit $wms $jdl 2>&1 |";
# open(INP,"glite-wms-job-submit -a --nomsg $jdl 2>&1 |");
 open(INP,"$cmd");
 my $id='';
 while(my $s=<INP>){
  chomp $s;
  printMsg($s,1);
  next if $s=~/^\s*$/;
  $id.=$s; #last;
 };
 close(INP);
 return $id;
}
#--------------------------------------------------------------
#           Get status of the nodes
#--------------------------------------------------------------
sub checkStatus{
 my $id=shift;
 my $cmd="$cmdStatus $id |";
# open(INP,"glite-wms-job-status $id |");
 open(INP,"$cmd");
 my $start=0; my ($node,$nid,$st,$ecode,$dest,$strea)=qw(? ? ? ? ? ?);
 my $ptrn='.*?:\s*(.*)$';
 while(my $s=<INP>){
  if($s=~/Nodes info/){$start=1; next};
  next unless $start;
  if ($s=~/Node Name:/){$s=~/$ptrn/; $node=$1; next};
  if ($s=~/https:/){$s=~/(https:.*)$/; $nid=$1; next};
  if ($s=~/Current Status/){$s=~/$ptrn/; $st=$1;  next};
  if ($s=~/Destination:/){$s=~/$ptrn/; $dest=$1;  next};
  if ($s=~/Status Reason:/){$s=~/$ptrn/; $strea=$1; next};
  if ($s=~/Exit code:/){$s=~/$ptrn/; $ecode=$1; next};
  if ($s=~/Submitted/){
   $nods{$node}=[0,$nid,$st,$dest,$strea,$ecode,0,0,0];
  };
 };
 close(INP);
}
#--------------------------------------------------------------
#        Get nodes output
#--------------------------------------------------------------
sub getJobsOutput{
 my $id=shift; my $ret='';
# my $cmd="glite-wms-job-output --noint --dir $tmpdir $id |";
 my $cmd="$cmdJobOutput $tmpdir/out $id |";
 open(INP,"$cmd");
 while(my $s=<INP>){  $ret.=$s; };
 close(INP);
 return $ret
}
#--------------------------------------------------------------
sub getTime{   #return date and time
 my $t=`date +'%D %T'`; chomp $t;
 return $t;
}
#--------------------------------------------------------------
#        Create tpm directory
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
#########################################################
#        Read configuration file
#########################################################
sub getConfigData{
 my $f=shift;
 unless(-e $f){return 1};
 my ($arg,$val);
 open(INP,"$f") or return 1;
 while(my $s=<INP>){
  next if $s=~/^#/;
  if($s=~/^\</){  #multilines attribute
   $s=~/^\<(.*?)\>/; $arg=$1; $val='';
   while($s=<INP>){
    if($s=~/$arg/){last};
    $val.=$s;
   };
  }else{
   $s=~/\s*(.*?)\s*=\s*(.*);/;
   ($arg,$val)=($1,$2);
  }; 
  $inp{$arg}=$val;
 };
 close(INP);
 return 0;
}