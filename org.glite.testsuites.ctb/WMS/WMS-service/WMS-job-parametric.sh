#!/bin/sh

###############################################################################
#
# Test parametric jobs
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
#
# TEST 01: Test integer version of Parameters
# TEST 02: Test integer version of Parameters changing 'start' and 'step' values
# TEST 03: Test list version of Parameters
# TEST 04: Test submission and check the output
# TEST 05: Test submission using a list as parameters
#
# Known issues: bug #79308 parameters should not be negative (SegFault)
#               bug #79027 ParameterStart must be equal to 0
#               bug #79144 NodesCollocation is not support 
#               bug #79165 Parameters list must be composed by strings
#
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

fail=0
err=""

function set_paramjdlbase()
{
	remove $JDLFILE
	echo "
  JobType = \"parametric\";
  Executable = \"/bin/echo\";
  Arguments = \"_PARAM_\";
  StdOutput  = \"Std-_PARAM_.out\";
  StdError   = \"std-_PARAM_.err\";
  OutputSandbox = {\"Std-_PARAM_.out\",\"std-_PARAM_.err\"};
  InputSandbox = {\"$JDLFILE\", \"$MYTMPDIR/In-_PARAM_.txt\"};
	Requirements = regexp(\"_PARAM_\" , other.GlueCEUniqueID);
	ShallowRetryCount = 3;
	RetryCount = 3;" >> $JDLFILE
  return 0
}

### start=$1, last=$2, step=$3 
function set_paramINT()
{
	echo "Parameters = $2;" >> $JDLFILE 
	echo "ParameterStart = $1;" >> $JDLFILE
	echo "ParameterStep = $3;"	>> $JDLFILE
	return 0

}

### list=$1
function set_paramLIST()
{
	echo "Parameters = { $1 };" >> $JDLFILE
	return 0
}

# $1 is the jdl file
# set $JOBID
function submit ()
{
  run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg --register-only $1" 1 "Job submission failed"
  JOBID=$OUTPUT
  return 0
} 

prepare "test parametric jobs" $@

message ""
message "TEST 01: Test integer version of Parameters"
touch $MYTMPDIR/In-0.txt $MYTMPDIR/In-1.txt $MYTMPDIR/In-2.txt $MYTMPDIR/In-3.txt $MYTMPDIR/In-4.txt
set_paramjdlbase
set_paramINT 0 5 1
submit $JDLFILE
run_command "glite-wms-job-info --jdl --noint -o $TMPFILE $JOBID" 1 "I'm not able to retrieve jdl info for $JOBID"
for ((i=0; i < 5 ; i++)) ; do
	run_command "grep Node_$i $TMPFILE" 0 
	if [[ $? -eq 1 ]] ; then err="Node name not found." ; fi 
	run_command "grep 'Environment = { \"ParameterValue=$i\" };' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Environment attribute not set." ; fi
	run_command "grep 'Arguments = \"$i\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Arguments attribute not set."; fi
	run_command "grep 'StdOutput  = \"Std-$i.out\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err StdOutput attribute not set." ; fi
	run_command "grep 'StdError  = \"std-$i.err\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err StdError attribute not set." ; fi
	run_command "grep 'OutputSandbox = { \"Std-$i.out\",\"std-$i.err\" }' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err OutputSandbox attribute not set." ; fi
	run_command "grep 'regexp(\"$i\",other.GlueCEUniqueID)' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Requirements attribute not set." ;  fi
done

if [[ $err != "" ]] ; then
	message "Test 01 FAILS. $err"
	fail=$(($fail+1))
else
	message "Test success!"
fi
run_command "glite-wms-job-cancel --noint $JOBID" 0
if [[ $? -eq 1 ]] ; then
	verbose "Warning: I'm not able to cancel $JOBID"
fi

err=""

message ""
message "TEST 02: Test integer version of Parameters changing 'start' and 'step' values"
touch $MYTMPDIR/In-5.txt $MYTMPDIR/In-8.txt $MYTMPDIR/In-11.txt 
set_paramjdlbase
set_paramINT 2 12 3
submit $JDLFILE
run_command "glite-wms-job-info --jdl --noint -o $TMPFILE $JOBID" 1 "I'm not able to retrieve jdl info for $JOBID"
for i in 2 5 8 11; do
	run_command "grep Node_$i $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="Node name not found." ; fi  
  run_command "grep 'Environment = { \"ParameterValue=$i\" };' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Environment attribute not set." ; fi
  run_command "grep 'Arguments = \"$i\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Arguments attribute not set."; fi
  run_command "grep 'StdOutput  = \"Std-$i.out\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err StdOutput attribute not set." ; fi
  run_command "grep 'StdError  = \"std-$i.err\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err StdError attribute not set." ; fi
  run_command "grep 'OutputSandbox = { \"Std-$i.out\",\"std-$i.err\" }' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err OutputSandbox attribute not set." ; fi
  run_command "grep 'regexp(\"$i\",other.GlueCEUniqueID)' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Requirements attribute not set." ;  fi
done

if [[ $err != "" ]] ; then
  message "Test 02 FAILS. $err"
	fail=$(($fail+1))
else
  message "Test success!"
fi
run_command "glite-wms-job-cancel --noint $JOBID" 0
if [[ $? -eq 1 ]] ; then
  verbose "Warning: I'm not able to cancel $JOBID"
fi

err=""

message ""
message "TEST 03: Test list version of Parameters"
touch $MYTMPDIR/In-Alfa.txt $MYTMPDIR/In-beta.txt $MYTMPDIR/In-42.txt $MYTMPDIR/In-delta_3.txt
set_paramjdlbase
set_paramLIST "Alfa, beta, 42, delta_3"
submit $JDLFILE
run_command "glite-wms-job-info --jdl --noint -o $TMPFILE $JOBID" 1 "I'm not able to retrieve jdl info for $JOBID"
for i in Alfa beta 42 delta_3; do
	run_command "grep Node_$i $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="Node name not found." ; fi
  run_command "grep 'Environment = { \"ParameterValue=$i\" };' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Environment attribute not set." ; fi
  run_command "grep 'Arguments = \"$i\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Arguments attribute not set."; fi
  run_command "grep 'StdOutput  = \"Std-$i.out\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err StdOutput attribute not set." ; fi
  run_command "grep 'StdError  = \"std-$i.err\"' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err StdError attribute not set." ; fi
  run_command "grep 'OutputSandbox = { \"Std-$i.out\",\"std-$i.err\" }' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err OutputSandbox attribute not set." ; fi
  run_command "grep 'regexp(\"$i\",other.GlueCEUniqueID)' $TMPFILE" 0
	if [[ $? -eq 1 ]] ; then err="$err Requirements attribute not set." ;  fi
done

if [[ $err != "" ]] ; then
  message "Test 03 FAILS. $err"
	fail=$(($fail+1))
else
  message "Test success!"
fi
run_command "glite-wms-job-cancel --noint $JOBID" 0
if [[ $? -eq 1 ]] ; then
  verbose "Warning: I'm not able to cancel $JOBID"
fi

err=""

message ""
message "TEST 04: Test submission and check the output"
touch $MYTMPDIR/In-6.txt $MYTMPDIR/In-9.txt
set_paramjdlbase
### WARNING
### Due to bug #79027 parameterstart must be equal to 0
set_paramINT 0 12 3
echo "Requirements = true;" >> $JDLFILE ### Overwrite requirements
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE" 1 "Job submission failed"
JOBID=$OUTPUT
wait_until_job_finishes $JOBID
get_STATUS $JOBID
ret=0
if [[ "$JOBSTATUS" == "Done (Success)" ]]; then
	verbose "Retrieve the output"
	run_command "glite-wms-job-output --dir $JOB_OUTPUT_DIR --nosubdir $JOBID" 1 "I'm not able to retrieve output file for $JOBID"
	verbose "Check if the status is correct"
	run_command "grep \"Warning - JobPurging not allowed\" $CMDOUT" 0
	if [[ $? -eq 1 ]] ; then
		sleep 5
		get_STATUS $JOBID
      if [[ "$JOBSTATUS" != *Cleared* ]]; then
        err="Warning. Final status of job ($JOBID) is not 'CLEARED'"
				ret=1
      fi
  else
    err="Warning. WMS is not recognized by the LB, JobPurging not allowed!"
		ret=1
  fi
  verbose "Check if the output files are correctly retrieved"
	if [ -f ${JOB_OUTPUT_DIR}/Node_0/Std-0.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_0/std-0.err ] && 
			[ -f ${JOB_OUTPUT_DIR}/Node_3/Std-3.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_3/std-3.err ] && 
			[ -f ${JOB_OUTPUT_DIR}/Node_6/Std-6.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_6/std-6.err ] && 
			[ -f ${JOB_OUTPUT_DIR}/Node_9/Std-9.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_9/std-9.err ] ; then
		verbose "Output files are OK"		
  else
    err="FAIL. Output files are not correctly retrieved"
		ret=2
  fi
else
  err="Warning. Job finishes with status: $JOBSTATUS; cannot retrieve output"
	ret=1
fi

if [ $ret -eq 1 ] ; then
  verbose ${err}
elif [ $ret -eq 2 ] ; then
	message "Test 04 FAILS. ${err}"
	fail=$(($fail+1))
else
	message "Test success!"
fi

err=""
ret=0
message ""
message "TEST 05: Test submission using a list as parameters"
touch $MYTMPDIR/In-gamma.txt
set_paramjdlbase
set_paramLIST "Alfa, beta, gamma"
echo "Requirements = true;" >> $JDLFILE ### Overwrite requirements
#echo "Nodescollocation = true;" >> $JDLFILE
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg $JDLFILE" 1 "Job submission failed"
JOBID=$OUTPUT
wait_until_job_finishes $JOBID
#verbose "Check if the attribute NodesCollocation works"
#run_command "glite-wms-job-status $JOBID | grep Destination | sort -u | wc -l" 1 "I'm not able to retrieve the status of $JOBID"
#if [[ $OUTPUT -ne 1 ]] ; then
#	err="Nodes are not submitted to the same CE. See if bug #79144 applies."
#	ret=2
#fi
	
#if [[ $ret -eq 0 ]] ; then 
	get_STATUS $JOBID
	if [[ "$JOBSTATUS" == "Done (Success)" ]]; then
  	verbose "Retrieve the output"
  	run_command "glite-wms-job-output --dir $JOB_OUTPUT_DIR --noint --nosubdir $JOBID" 1 "I'm not able to retrieve output file for $JOBID"
  	verbose "Check if the status is correct"
  	run_command "grep \"Warning - JobPurging not allowed\" $CMDOUT" 0
  	if [[ $? -eq 1 ]] ; then
    	sleep 5
    	get_STATUS $JOBID
    	if [[ "$JOBSTATUS" != *Cleared* ]]; then
      	err="Warning. Final status of job ($JOBID) is not 'CLEARED'"
				ret=1
    	fi
  	else
    	err="Warning. WMS is not recognized by the LB, JobPurging not allowed!"
			ret=1
  	fi
  	verbose "Check if the output files are correctly retrieved"
  	if [ -f ${JOB_OUTPUT_DIR}/Node_Alfa/Std-Alfa.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_Alfa/std-Alfa.err ] && 
    	 [ -f ${JOB_OUTPUT_DIR}/Node_beta/Std-beta.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_beta/std-beta.err ] && 
    	 [ -f ${JOB_OUTPUT_DIR}/Node_gamma/Std-gamma.out ] && [ -f ${JOB_OUTPUT_DIR}/Node_gamma/std-gamma.err ] ; then
  		verbose "All output files are retrieved"   
			if [[ `cat ${JOB_OUTPUT_DIR}/Node_Alfa/Std-Alfa.out` == "Alfa" ]] &&
				[[ `cat ${JOB_OUTPUT_DIR}/Node_beta/Std-beta.out` == "beta" ]] &&
				[[ `cat ${JOB_OUTPUT_DIR}/Node_gamma/Std-gamma.out` == "gamma" ]] ; then
				verbose "Output files are as expected"
			else
				err="FAIL. Output files are not as expected"
				ret=2
			fi
  	else
    	err="FAIL. Output files are not correctly retrieved"
    	ret=2
  	fi
	else
  	err="Warning. Job finishes with status: $JOBSTATUS; cannot retrieve output"
  	ret=1
	fi
#fi

if [ $ret -eq 1 ] ; then
  message ${err}
elif [ $ret -eq 2 ] ; then
  message "Test 05 FAILS. ${err}"
  fail=$(($fail+1))
else
  message "Test success!"
fi

## remove used files
rm -rf $MYTMPDIR/In-*
rm -rf ${JOB_OUTPUT_DIR}

# ... terminate

if [ $fail -ne 0 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

