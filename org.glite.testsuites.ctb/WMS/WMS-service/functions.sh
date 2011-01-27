#!/bin/sh

###############################################################################
#
# Common functions for the WMS commands test suite
#
# The package is intended to test the WMS client software installed on a UI 
# and indirectly the WMS service itself or computing infrastructure.
#
# The test suite requires some configuration parameters, see wms-command.conf
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
#
###############################################################################

# Special echo; if required (LOG=1) save texts on a log file
# NAGIOS mode means no output!
function myecho()
{
	if [ $NAGIOS -ne 1 ]; then
		if [ $LOG -eq 1 ]; then
    	echo " ===> $@" >> $TESTLOGFILE
 	 	else
    	echo " ===> $@" 
  	fi
	fi
}

# Basic messages, verbose = 1 
function message()
{
  if [ $VERBOSE -ge 1 ]; then
    myecho ">>> $@"
  fi
}

# Verbose messages, verbose = 2
function verbose()
{
  if [ $VERBOSE -ge 2 ]; then
    myecho "$@"
  fi
}

# Debug messages, verbose = 3
function debug()
{
  if [ $VERBOSE -eq 3 ]; then
    myecho "DEBUG: $@"
  fi
}

# remove unwanted spaces from argument $1
function trim() 
{ 
  echo $1; 
}

# remove a file ($1) only if exists
function remove()
{
  debug "Remove file $1..."
  [[ -f $1 ]] && rm -f $1

  return 0
}

# FAILURE exit: arg is the failure reason
# Exit code: 2 (as required by NAGIOS)
function exit_failure()
{
	# No cleanup for debug
	#cleanup
 
  myecho ""
	myecho "Test: $NAME"
  myecho "Started: $START_TIME"
  myecho "Ended  :" $(date +%H:%M:%S)
  myecho ""
  myecho "    >>> TEST FAILED <<<"
  myecho " >>> failure reason: $@ <<< "
  myecho ""
  myecho " Test directory $MYTMPDIR" 
  myecho " has not been cleaned for debug purpose"


  if [ $LOG -eq 1 ] ; then
    echo "Test log file is $TESTLOGFILE"
    echo ""
    echo " Test directory $MYTMPDIR has not been cleaned for debug purpose"
    echo ""
    echo "    >>> TEST FAILED <<<"
  	echo " >>> failure reason: $@ <<< "
  fi

	if [ $NAGIOS -eq 1 ] ; then
		echo "$NAME CRITICAL: $@"
	fi
  exit 2
}

# GOOD exit
# Exit code: 0
function exit_success()
{
  cleanup

  myecho ""
	myecho "Test: $NAME"
  myecho "Started: $START_TIME"
  myecho "Ended  :" $(date +%H:%M:%S)
  myecho ""
  myecho "    === test PASSED === "

  if [ $LOG -eq 1 ] ; then
    echo "Test log file is $TESTLOGFILE"
    echo ""
    echo "    === TEST PASSED === "
  fi

	if [ $NAGIOS -eq 1 ] ; then
  	echo "$NAME OK: Test Passed"
	fi
				
	exit 0
}

# Warning exit: arg is the warning message
# Exit code: 1 (as required by NAGIOS)
function exit_warning()
{
	cleanup

  myecho ""
	myecho "Test: $NAME"
  myecho "Started: $START_TIME"
  myecho "Ended  :" $(date +%H:%M:%S)
  myecho ""
 	myecho "  *** Warning: $@ *** "
	myecho ""		
	
  if [ $LOG -eq 1 ] ; then
    echo "Test log file is $TESTLOGFILE"
    echo ""
    echo "  *** Warning: $@ *** "	
	  echo ""	
  fi

  if [ $NAGIOS -eq 1 ] ; then
    echo "$NAME WARNING: $@"
	fi
	
  exit 1
}

# ... exit on timeout: try cancel job and exit
# $1 must be the JOBID
function exit_timeout()
{
  verbose "Try to remove job..."
  run_command glite-wms-job-cancel --noint $1
  sleep 60
  get_STATUS $1
  if [[ $JOBSTATUS == "Cancelled" ]]; then
    verbose 'Job has been succesfully cancelled'
  else
    message 'WARNING: Job has NOT been cancelled'
  fi
  exit_warning "Timeout reached"
}

# ... exit on Ctrl^C
function exit_interrupt()
{
  trap - SIGINT

	exit_warning "Interrupted by user"
	
}

# ... cleanup temporary files
function cleanup()
{
  verbose ""
  verbose "Cleaning up $MYTMPDIR ..."

  remove $OUTPUTFILE
  remove $LOGFILE
  remove $JOBIDFILE
  remove $JDLFILE
  remove $CONFIG_FILE
  remove $TMPFILE

  remove ${JOB_OUTPUT_DIR}/std.out
  remove ${JOB_OUTPUT_DIR}/std.err
  remove ${JOB_OUTPUT_DIR}/out.txt

  debug "Remove ${JOB_OUTPUT_DIR} directory ..."

  [[ -d ${JOB_OUTPUT_DIR} ]] && rmdir ${JOB_OUTPUT_DIR}

  [[ -f $PROXY ]] && voms-proxy-destroy -file $PROXY 

  debug  "Remove $MYTMPDIR directory ..."

  [[ -d "$MYTMPDIR" ]] && rmdir $MYTMPDIR

}

# ... run command successfuly or exit with cleanup
# --> set OUTPUT with command's output
function run_command()
{
  verbose ""
  verbose "[" $(date +%H:%M:%S) "] $" $@

  OUTPUT=$(eval $@ 2>&1)

  if [ $? -ne 0 ]; then
    verbose "${OUTPUT}"
    exit_failure "$1 failed"
  fi

  debug "${OUTPUT}"
  verbose " -> Command success" 

  return 0
}

# ... run "fail" command (success if return value != 0) or exit with cleanup
# --> set OUTPUT with command's output
function run_command_fail()
{
  verbose ""
  verbose "[" $(date +%H:%M:%S) "] $" $@

  OUTPUT=$(eval $@ 2>&1)

  if [ $? -eq 0 ]; then
    debug "${OUTPUT}"
    exit_failure "$1 not failed as required"
  fi

  debug "${OUTPUT}"
  verbose " -> Command successfully failed"

  return 0
}

# ... print help message and exit
function usage()
{
  echo "Usage: "
  echo ""
  echo "$NAME [-h] [-l] [-d <level>] [-c <conf>] [-i] [-n] [-W] [-L] [-C] [-v]"
  echo ""
  echo " -h          this help"
  echo " -l          save output in a file"
  echo " -d <level > print verbose messages (level = (1|2|3)"
  echo " -i          interactive mode (it asks for proxy pwd)" 
	echo " -n          nagios mode"			
	echo " -c <conf>   configuration file"
	echo " -W <wms>    WMS host (required if conf file is not used)"
	echo " -L <lb>     LB host (required if conf file is not used)"
	echo " -C <ce>     CE host"
	echo " -v <vo>     User VO (required if conf file is not used)"
	echo ""
  exit 0
}

# ... get user passwd
# --> set PASS
function set_pwd()
{
  echo "Enter the user proxy passwd:"
  read -s PASS
  return 0
}

# ... create proxy file $1 with validity $2 (default 24:00)
# --> required: VO 
function set_proxy()
{
  if [ -z $2 ] ; then
    VALID="24:00"
  else
    VALID=$2
  fi
  
  verbose "Initializing proxy file ..."
  OUTPUT=$(eval "echo $PASS | voms-proxy-init -voms ${VO} -verify -valid $VALID -bits 1024 -pwstdin -out $1" 2>&1)
  if [ $? -eq 0 ] ; then
    debug $OUTPUT
    export X509_USER_PROXY=$1
  else
    echo $OUTPUT
    exit_failure "Failed to create user proxy"
  fi
  return 0
}

### JDL definitions

# define a simple jdl and save it in $1
function set_jdl()
{
  remove $1
  echo "Executable = \"/bin/hostname\";" >> $1
  echo "StdOutput  = \"std.out\";" >> $1
  echo "StdError   = \"std.err\";" >> $1
  echo "OutputSandbox = {\"std.out\",\"std.err\"};" >> $1
  echo "ShallowRewtryCount = 3; " >> $1

  return 0
}

# define a 5 minutes long jdl and save it in $1
function set_longjdl()
{
  remove $1
  echo "Executable = \"/bin/sleep\";" >> $1
  echo "Arguments = \"300\";" >> $1

  return 0
}

# define a jdl with ISB and save it in $1
function set_isbjdl()
{
  remove $1
  echo "Executable = \"/bin/ls\";" >> $1
  echo "Arguments = \"-la\";" >> $1
  echo "StdOutput  = \"std.out\";" >> $1
  echo "StdError   = \"std.err\";" >> $1
  echo "OutputSandbox = {\"std.out\",\"std.err\"};" >> $1
  echo "InputSandbox = {\"$1\"};" >> $1

  return 0  
}


# add given requirements ($1) to jdl
# require: JDLFILE
function set_requirements()
{
  set_jdl $JDLFILE
  echo "Requirements = $1;" >> $JDLFILE
}


############################################3

# define configuration file and save it in $1
# --> require: WMS
# --> require: VO
# --> require: LB
# --> require: DEFAULTREQ
function set_conf()
{
  remove $1
  echo "[" >> $1
  echo "  WMProxyEndPoints = {\"https://${WMS}:7443/glite_wms_wmproxy_server\"};" >> $1
  echo "  LBAddresses= {\"${LB}\"};" >> $1
  echo "  jdlDefaultAttributes = [" >> $1 
  echo "    VirtualOrganisation = \"${VO}\";" >> $1
  echo "    Requirements = ${DEFAULTREQ} ;" >> $1
  echo "    Rank =  -other.GlueCEStateEstimatedResponseTime;" >> $1
  echo "    SignificantAttributes = { \"Requirements\",\"Rank\" };" >> $1
  echo "  ]" >> $1
  echo "]" >> $1

  return 0
}


# ... prepare everything
# $1 contains test description
function prepare()
{
  # First arguments contains test description
  DESC="$1"
  shift 

  # Test id
  ID=`date +%Y%m%d%H%M%S`
  NAME=$(basename $0 .sh)
	
  # Default values
  VERBOSE=0
  LOG=0
  NOPROXY=1
	NAGIOS=0
	CE=""
	
  # ... define default values for optional parameters
  NUM_STATUS_RETRIEVALS=60
  SLEEP_TIME=30
  DELEGATION_OPTIONS="-a"
  DEFAULTREQ="other.GlueCEStateStatus == \"Production\""
	CONF="wms-command.conf"
	
  while getopts "hld:ic:nW:v:L:C:" arg;
    do
    case "$arg" in
      h) usage ;;
      l) LOG=1 ;;
      d) VERBOSE=$OPTARG ;;
      i) NOPROXY=0 ;;
			c) CONF=$OPTARG ;;
			n) NAGIOS=1 ;;
			W) WMS=$OPTARG ;;
			v) VO=$OPTARG ;;
			L) LB=$OPTARG ;;
			C) CE=$OPTARG ;;
      *) echo "Illegal option '${OPTARG}'"; exit 3 ;;
    esac
  done
	
  # source configuration values
	if [ -f $CONF ] ; then
		source $CONF
	fi
	
	if [ -z $WMS ] || [ -z $LB ] || [ -z $VO ] ; then
		exit_failure "Required argument is not set"
	fi
	
  # ... create temporary directory
  MYTMPDIR=`pwd`/wms-cli-test_${ID}
  mkdir $MYTMPDIR || exit_failure "Fail to create temporary directory"

  # ... create log file
  if [ $LOG -eq 1 ]; then
    TESTLOGFILE=wms-command_${ID}.log
    remove $TESTLOGFILE
  fi

  myecho "+++++++++++++++++++++++++++++++++++++++++++++++++++++"
  myecho "+ TestSuite of the gLite-WMS command line interface  "
  myecho "+ Description: $DESC "
  myecho "+++++++++++++++++++++++++++++++++++++++++++++++++++++"

  START_TIME=$(date +%H:%M:%S)

  # ... define common directory and file names
  JOB_OUTPUT_DIR=$MYTMPDIR/jobOutput
  LOGFILE=$MYTMPDIR/log.txt
  OUTPUTFILE=$MYTMPDIR/output.txt
  JOBIDFILE=$MYTMPDIR/job.id
  PROXY=$MYTMPDIR/proxy.file
  TMPFILE=$MYTMPDIR/file.tmp
  JDLFILE=$MYTMPDIR/example.jdl
  CONFIG_FILE=$MYTMPDIR/wms.conf

  # Create default jdl and config file
  set_jdl $JDLFILE
  set_conf $CONFIG_FILE

  # Create proxy if required
  if [ $NOPROXY -ne 1 ] ; then
    set_pwd
    set_proxy "$PROXY"
  else
    tmp=`voms-proxy-info -timeleft`
    if [[ $tmp -eq 0 ]] ; then
			set_proxy "$PROXY" # try to create a proxy without pwd
		fi
	fi
	tmp=`voms-proxy-info -timeleft`	
	if [[ $tmp -eq 0 ]] ; then
		exit_failure "I don't find neither create any valid proxy"
  fi

  # ... set a trap for Ctrl^C
  trap exit_interrupt SIGINT
}

# ... delegate proxy and (re-)define DELEGATION_OPTIONS
# --> set: DELEGATION_OPTIONS
function define_delegation()
{
  DELEGATION_OPTIONS="-d del_${ID}"
  verbose "Delegating proxy ..."
  run_command glite-wms-job-delegate-proxy $DELEGATION_OPTIONS -c $CONFIG_FILE  

  return 0
}

# Extract the destination CE of the job given as input ($1 must be a valid JOBID)
# --> set CENAME
function get_CE()
{
  CENAME="Destination not available"

  verbose "Look for the destination..."

	# Waiting until job is matched
	run_command glite-wms-job-status $1
	while [[ "$JOBSTATUS" == *Submitted* ]] 
	do
		sleep 3
		run_command glite-wms-job-status $1
	done

  tmp=`grep -m 1 Destination <<< "$OUTPUT" | sed -e "s/Destination://"`
	if [[ $? -eq 1 ]] ; then
		exit_failure "Job $1 doesn't match"
	fi
	
	if [[ x${tmp} != "x" ]] ; then
  	CENAME=$(trim "$tmp")
	fi

  verbose "CE id is: $CENAME"

  return 0  
}

 
# Extract the "status" of the job given as input ($1 must be a valid JOBID)
# --> set JOBSTATUS
function get_STATUS()
{
  JOBSTATUS="Unknown" 
 
  verbose "Check job's status..." 
  run_command glite-wms-job-status $1

  tmp=`grep -m 1 'Current Status' <<< "$OUTPUT" | awk -F: '{print $2}'`

  JOBSTATUS=$(trim "$tmp")

  verbose "Job status is: $JOBSTATUS"

  return 0
}


# Check if job given as input is finished ($1 must be a valid JOBID)
# returns if job is not finished 
# returns 1 if job is DoneOK, 2 if Aborted 
# returns 3 if jobs is Cancelled and 4 if DoneFailed
# returns 5 if jobs is Cleared
function is_finished()
{

  get_STATUS "$1"
  
  # ... exit if it is Aborted
  if [[ "$JOBSTATUS" == *Aborted* ]]; then
    verbose "Job was Aborted !"
    return 2
  fi

  # ... or Cancelled
  if [[ "$JOBSTATUS" == *Cancelled* ]]; then
    verbose "The job has been cancelled !"
    return 3
  fi

  # ... or Failed
  if [[ "$JOBSTATUS" == *Done*Failed* ]]; then
    verbose "The job finished with failure !"
    return 4
  fi

  # ... or Cleared
  if [[ "$JOBSTATUS" == *Cleared* ]]; then
    verbose "The job has been cleared !"
    return 5
  fi

  # ... go to the next step if it is a success
  if [[ "$JOBSTATUS" == *Done* ]]; then
    verbose "Job finished !"
    return 1
  fi

  return 0
}

# .... wait until job $1 is done or time is out
# Need these two variables: $SLEEP_TIME $NUM_STATUS_RETRIEVALS
function wait_until_job_finishes()
{

  verbose "Wait until job finishes..."
	
  i=0

  while is_finished $1
  do
    if [ $i -ge $NUM_STATUS_RETRIEVALS ]; then
      exit_timeout $1
    fi
  
    verbose "Job is $JOBSTATUS... sleeping $SLEEP_TIME seconds ($i/${NUM_STATUS_RETRIEVALS}) ..."
    sleep $SLEEP_TIME
    
    ((i++))

  done

}

