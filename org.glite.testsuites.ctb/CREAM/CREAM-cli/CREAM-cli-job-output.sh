#!/bin/sh

###############################################################################
#
# Test glite-ce-job-output command. Test success if all the required (3) files 
# are retrieved, and if the standard output (out.txt) contains the hostname of
# the worker node where job has been run.
#
# TEST 1: output retrieve without option
# TEST 2: output retrieve with --dir option
# TEST 3: check --noint option
# TEST 4: check if --input option works
# TEST 5: check if command works with multiple JOBID
#
# Author: Alessio Gianelle <sa3-italia@mi.infn.it>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare $@


### check the output of the job saved in $1 directory
function check_output
{

DIR=$1

# there must be 3 files: out.txt, err.txt and sbd.jdl
if [ -a ${DIR}/out.txt ] && [ -a ${DIR}/err.txt ] && [ -a ${DIR}/sbd.jdl ] ; then
	a=`glite-ce-job-status -L 2 $JOBID | grep "Worker Node" | awk -F= '{print $NF}'`
	RWN=$(trim $a)  ## remove unwanted spaces
	WN=`cat ${DIR}/out.txt`
	debug "The contents of stdout is ${WN}, the expected worker node is ${RWN}"
	# file out.txt must contains the worker node hostname where job runs
	if [[ "[${WN}]" == "${RWN}" ]] ; then
		return 0
	else
		return 1
	fi
fi

return 2

}


FAILED=0

my_echo ""

TESTCOMMAND="${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-output"

if [ ! -x ${TESTCOMMAND} ] ; then
  exit_failure "Command ${TESTCOMMAND} not exists, test could not be performed!"
fi

#### Create a JDL with SBD
printf "
[
JobType = \"Normal\";
Executable = \"/bin/hostname\";
outputsandbox={\"out.txt\", \"err.txt\", \"sbd.jdl\"};
outputsandboxbasedesturi=\"gsiftp://localhost\";
inputsandbox={\"$(dirname $0)/functions.sh\", \"${MYTMPDIR}/sbd.jdl\"};
StdOutput=\"out.txt\";
StdError=\"err.txt\";
]" > ${MYTMPDIR}/sbd.jdl

### Submit some jobs




### TEST 1, output retrieve without option

my_echo "TEST 1: check if command works"

wait_until_job_finishes ${MYTMPDIR}/sbd.jdl

run_command "${TESTCOMMAND} $JOBID"
DIR=`echo $COM_OUTPUT | awk '{print $NF}'`

check_output $DIR

if [ $? -eq 0 ] ; then
	success
else
	if [ $? -eq 1 ] ; then
  	failure "Test failed: output has been correctly retrieved, but it is wrong"
	else
		failure "Test failed: output has not been correctly retrieved"
	fi
  ((FAILED++)) # continue
fi

# remove output directory
rm -rf $DIR

### TEST 2, output retrieve with --dir option

my_echo "TEST 2: check if --dir option works"

wait_until_job_finishes ${MYTMPDIR}/sbd.jdl

run_command "${TESTCOMMAND} --dir ${MYTMPDIR} $JOBID"
DIR=`echo $COM_OUTPUT | awk '{print $NF}'`

# DIR must be a subdirectory of MYTMPDIR
if [[ ${MYTMPDIR} < ${DIR} ]] ; then
	check_output $DIR
	if [ $? -eq 0 ] ; then
  	success
	else
  	if [ $? -eq 1 ] ; then
    	failure "Test failed: output has been correctly retrieved, but it is wrong"
  	else
    	failure "Test failed: output has not been correctly retrieved"
  	fi
  	((FAILED++)) # continue
	fi
else
	failure "Test failed: output is not saved in the given directory"
	((FAILED++)) # continue
fi

### TEST 3, check --noint option

my_echo "TEST 3: check if --noint option works"

wait_until_job_finishes ${MYTMPDIR}/sbd.jdl

run_command "${TESTCOMMAND} --dir ${MYTMPDIR} $JOBID"

debug "Retrieve again input files"

run_command "${TESTCOMMAND} --noint --dir ${MYTMPDIR} $JOBID"
DIR=`echo $COM_OUTPUT | awk '{print $NF}'`

check_output $DIR

if [ $? -eq 0 ] ; then
  success
else
  if [ $? -eq 1 ] ; then
    failure "Test failed: output has been correctly retrieved, but it is wrong"
  else
    failure "Test failed: output has not been correctly retrieved"
  fi
  ((FAILED++)) # continue
fi

### TEST 4, check if --input option works

my_echo "TEST 4: check if --input option works"

### Submit 5 jobs
debug "Submitting 5 jobs..."
i=0
while [ $i -lt 5 ] ; do
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a --output $MYTMPDIR/jobid -r $CREAM ${MYTMPDIR}/sbd.jdl
  if [ $? -ne 0 ]; then
    exit_failure ${COM_OUTPUT}
  fi
	((i++))
done

### Wait until all jobs finish
debug "Waiting until jobs finish"
OK=0
while [ $OK -ne 5 ]; do 
	OK=`glite-ce-job-status -i $MYTMPDIR/jobid | grep -c "DONE"`
	my_echo "${OK}/5 jobs finish.."
	sleep 5
done

FAIL=0
run_command "${TESTCOMMAND} --input $MYTMPDIR/jobid --dir ${MYTMPDIR}/multi"
IFS=$'\n'
for line in $COM_OUTPUT; do 
	JOBID=`echo $line | awk -F[ '{print $2}' | awk -F] '{print $1}'`
	DIR=`echo $line | awk '{print $NF}'`
	debug "Check output of jobs ${JOBID} under ${DIR}"
	check_output $DIR
	if [ $? -ne 0 ] ; then
		failure "Test failed: the output retrieved for job $JOBID is wrong"
		FAIL=1
	fi
done

if [ $FAIL -eq 1 ] ; then
	((FAILED++)) # continue
else
	success
fi

rm $MYTMPDIR/jobid 

### TEST 5, check if command works with multiple JOBID

my_echo "TEST 5: check if command works with multiple JOBID"

### Submit 3 jobs
debug "Submitting 3 jobs..."
i=0
while [ $i -lt 3 ] ; do
  run_command ${GLITE_LOCATION:-/opt/glite}/bin/glite-ce-job-submit -a -o $MYTMPDIR/jobid -r $CREAM ${MYTMPDIR}/sbd.jdl
  if [ $? -ne 0 ]; then
    exit_failure ${COM_OUTPUT}
  fi
	extract_jobid ${COM_OUTPUT}
	JID[${i}]="${JOBID}"
  ((i++))
done

### Wait until all jobs finish
debug "Waiting until jobs finish"
OK=0
while [ $OK -ne 3 ]; do
  OK=`glite-ce-job-status -i $MYTMPDIR/jobid | grep -c "DONE"`
  my_echo "${OK}/3 jobs finish.."
  sleep 5
done

run_command "${TESTCOMMAND} --dir ${MYTMPDIR}/multi ${JID[0]} ${JID[1]} ${JID[2]}"
IFS=$'\n'
for line in $COM_OUTPUT; do
  JOBID=`echo $line | awk -F[ '{print $2}' | awk -F] '{print $1}'`
  DIR=`echo $line | awk '{print $NF}'`
  debug "Check output of jobs ${JOBID} under ${DIR}"
  check_output $DIR
  if [ $? -ne 0 ] ; then
    failure "Test failed: the output retrieved for job $JOBID is wrong"
    FAIL=1
  fi
done

if [ $FAIL -eq 1 ] ; then
  ((FAILED++)) # continue
else
  success
fi



## FINISHED

if [ $FAILED -gt 0 ] ; then
	exit_failure "$FAILED test(s) failed on 5 differents tests"
else
	exit_success
fi
