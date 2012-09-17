#!/bin/sh

# Internal variables to collect statistics on submission's times
max=0
min=999999
tot=0
DATE=`date +%m%d%H%M`
# To count failed submissions
FAILED=0

# Home many jobids to store in a single file
JOF=50

# Default values
USER=1
JOBS=100
NODES=40
FREQ=60 
LOGFILE=""

# Prefix for the working directory 
TESTDIR=`pwd`

# Directory with user's proxy
PROXY=${TESTDIR}/proxy

# echo function
myecho()
{
	if [[ -f $LOGFILE ]]; then 
		echo -e $1 >> $LOGFILE
	else
		echo -e $1
	fi
}

# Help 
help()
{
  echo -e "Usage: $0 [option] -c conf -j jdl\n"
	echo -e "This test requires that for every user X there is a valid proxy in the directory"
	echo -e "'proxy' with name userX.proxy, where X is an integer in range(Num_user)\n"
  echo -e " -h \t\t\t This help;"
  echo -e " -t Num_jobs \t\t Number of collections to submit x user (default $JOBS);"
  echo -e " -n Num_jdls \t\t Number of nodes x collection (default $NODES);"
  echo -e " -j jdl \t\t Jdl to use (required);"
  echo -e " -u Num_user \t\t Number of users (default $USER);" 
  echo -e " -p proxy \t\t Directory with user's proxy (default $PROXY);" 
  echo -e " -f Frequency \t\t Seconds between two submissions (default $FREQ);"
  echo -e " -c conf \t\t Configuration file to use to submit (required);"
  echo -e " -r CEID \t \t Resource's name to use to submit"
  echo -e " -l LogFile \t \t Log messages on LogFile (defaut stdout)"
  exit 0
}

# Parse options
while getopts "ht:n:u:p:f:j:c:r:l:" arg
do
  case "$arg" in
  h) help ;;
  t) JOBS=$OPTARG ;;
  n) NODES=$OPTARG ;;
  u) USER=$OPTARG ;;
  f) FREQ=$OPTARG;;
  j) JDL="$OPTARG";;
  c) CONF="$OPTARG";;
  r) RESOURCE="$OPTARG";;
  p) PROXY="$OPTARG";;
  l) LOGFILE="$OPTARG";;
  *) echo "Not recognized argument"; exit -1 ;;
  esac
done

# The configuration file to use for submissions is required
if [ "x$CONF" = "x" ]; then
  echo "The configuration file is required!"
  exit 1
fi

# The jdl is required
if [ "x$JDL" = "x" ]; then
  echo "A JDL is required!"
  exit 1
fi

# Check the existence of the proxy
for (( u=0; $u < $USER ; u = $(($u +  1)) )); do
`voms-proxy-info -exists -valid 0:10 -file ${PROXY}/user${u}.proxy 2> /dev/null`
  if [[ $? -eq 1 ]]; then
    echo "Proxy file for user $u not exists or is expiring, please create a valid proxy."
    exit 1
  fi

done

# Check if the configuration file exists
if  [[ ! -f $CONF ]]; then
  echo "Configuration file: $CONF not exists, please create it."
  exit 1
fi

if [[ "x$LOGFILE" != "x" ]]; then
	rm -f $LOGFILE
	touch $LOGFILE
fi

# Working directory
WORKDIR=${TESTDIR}/Test_`date +%s`
if [[ ! -d ${WORKDIR} ]]; then
  mkdir ${WORKDIR}
fi

# Prefix for the directory for the output files
OUTPUTDIR=${WORKDIR}/output
mkdir $OUTPUTDIR

# Create the collection to use  
COLLDIR=$WORKDIR/collection
mkdir $COLLDIR

# Create $NODES simbolic links to $JDL
for (( j=0; $j < $NODES ; j = $(($j +1)) )); do
  ln -s `pwd`/$JDL $COLLDIR/jdl_${j}
done  

# Start the test
myecho "Test starts on `date`: submit $(( $JOBS * $USER )) collections (of $NODES nodes) with a frequency of $FREQ seconds using $USER users.\n"

for (( i=0 ; $i < $JOBS ; i = $(($i + 1)) )); do
  myecho "Submission n. $(( $i + 1 )):" 
	for (( u=0; $u < $USER ; u = $(($u +  1)) )); do
    # Change JobID file every $JOF jobs
    JOBID=${OUTPUTDIR}/USER${u}_$(( $i / $JOF )).jobid
    start=`date +%s`
    myecho "`date`: user $u is submitting..."
    export X509_USER_PROXY=${PROXY}/user${u}.proxy
    if [ "x$RESOURCE" != "x" ]; then
			opt_res="--nodes-resource $RESOURCE"
		else
			opt_res=""
		fi
		if [[ -f $LOGFILE ]]; then
	    /usr/bin/glite-wms-job-submit -a -c $CONF -o $JOBID $opt_res --collection $COLLDIR >> $LOGFILE 2>&1
		else
      /usr/bin/glite-wms-job-submit -a -c $CONF -o $JOBID $opt_res --collection $COLLDIR
    fi
    fail=$?
		end=`date +%s`
		lenght=$(( $end - $start ))
    if [ $fail -gt 0 ]; then
      FAILED=$(( $FAILED + 1 ))
      myecho "----> SUBMISSION FAILED (after $lenght seconds)!"
    else
      # Calculate submission's times statistics
      if [ $lenght -gt $max ]; then
        max=$lenght
      fi
      if [ $lenght -lt $min ]; then
        min=$lenght
      fi
      tot=$(( $tot + $lenght ))
      myecho "Submission takes $lenght seconds."
    fi
    # Sleep $FREQ seconds between two submssions, except last time
    if ! (( (( $(( $JOBS - $i )) == 1 )) && (( $(( $USER - $u )) == 1 )) )); then
    		LS=$(( $FREQ - $lenght ))
    		if [ $LS -gt 0 ]; then	
      		myecho "Wait $LS seconds before the next submission....\n"
      		sleep $LS
    		fi
		fi
  done;
done;

# Total jobs
TJ=$(( $JOBS * $USER ))

# Report statistics
if [ $(( $TJ - $FAILED)) -eq 0 ]; then
	myecho "ALL submissions fail!!!"
else
	myecho "$(( $TJ - $FAILED)) jobs submitted in $tot seconds: $min/$(( $tot / $(( $TJ - $FAILED)) ))/$max (min/avg/max)"
	if [ $FAILED -gt 0 ] ; then
  	myecho "$FAILED submission(s) fail(s)"
	fi
fi

# TEST finished
myecho "Test finishes at `date`"


