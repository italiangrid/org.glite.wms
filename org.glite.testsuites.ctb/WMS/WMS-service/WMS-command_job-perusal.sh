#!/bin/sh

###############################################################################
#
# Test command glite-wms-job-perusal (getting output from job at run time).
#
# Test --version option
# Test --set option
# Test --get option
# Test --get option with --dir
# Test --get option with --nodisplay and --all
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
# Version: $Id$
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare "test glite-wms-job-perusal command." $@

fail=0

COMMAND=glite-wms-job-perusal
message "Check if command $COMMAND exists"
run_command "which $COMMAND" 1 "$COMMAND is not in the PATH"

# Test --version option
message ""
message "Test --version option"
run_command "$COMMAND --version" 0
if [[ $? -eq 0 ]] ; then
  verbose "Check the output command"
  run_command "grep \"WMS User Interface version\" $CMDOUT" 0
  if [[ $? -eq 0 ]] ; then
    verbose "We are testing $OUTPUT"
    message "Test success"
  else
    verbose "Version number not found ($OUTPUT)"
  fi
else
  fail=$(($fail+1))
  message "Command fails"
fi

# Create an ad hoc jdl for perusal job
remove $JDLFILE
echo "Executable = \"sleeper.sh\";" > $JDLFILE
echo "Arguments = \"out.txt\";" >> $JDLFILE
echo "StdOutput  = \"std.out\";" >> $JDLFILE
echo "StdError   = \"std.err\";" >> $JDLFILE
echo "InputSandbox = {\"${MYTMPDIR}/sleeper.sh\"};" >> $JDLFILE
echo "OutputSandbox = {\"std.out\",\"std.err\",\"out.txt\"};" >> $JDLFILE
echo "PerusalFileEnable = true;" >> $JDLFILE

# and its executable
echo "
#!/bin/sh

echo "This is sleeper"
echo "This is sleeper" > \$1

for ((i=1; i <= 100; i++))
do

  echo \"message \$i\" >> \$1
  sleep 15

done

echo \"Stop sleeping!\" >> \$1
echo \"Stop sleeping!\"
" >> ${MYTMPDIR}/sleeper.sh

## ... submit a job

verbose "Submit the job"
run_command "glite-wms-job-submit --config $CONFIG_FILE  $DELEGATION_OPTIONS --nomsg $JDLFILE" 1 "Job submission fails"
JOBID=$OUTPUT
debug "JobID is $JOBID"

## ... enable file perusal for out.txt

message ""
message "Test --set option"
run_command "$COMMAND --set --filename out.txt -f std.out $JOBID" 0
if [[ $? -eq 0 ]] ; then
	message "Test success"
else
	message "Test fails"
	fail=$(($fail+1))
fi

message ""
message "BEWARE default min perusal interval is 1000 secs, so this phase could take many minutes"
message "Test --get option"
while is_finished $JOBID
do
  sleep 60
  run_command "$COMMAND --get -f out.txt --dir $JOB_OUTPUT_DIR $JOBID" 0
	if [[ $? -ne 0 ]] ; then
  	message "Command fails"
  	fail=$(($fail+1))
	fi
done
verbose "List the retrieved parts of out.txt"
run_command "ls -l ${JOB_OUTPUT_DIR} | grep out.txt" 0
if [[ $? -eq 0 ]] ; then
  message "Test success"
else
  message "Test fails"
  fail=$(($fail+1))
fi


message "Test --get option with --dir"
run_command "$COMMAND --get -f std.out --dir $JOB_OUTPUT_DIR/perusal $JOBID" 0
if [[ $? -eq 0 ]] ; then
	run_command "ls $JOB_OUTPUT_DIR/perusal/std.out-*" 0
	if [[ $? -ne 0 ]] ; then
  	message "Test fails. No output seem to be obtained using glite-wms-job-perusal !"
		fail=$(($fail+1))
	else
		message "Test success"
	fi
else
	message "Command fails"
	fail=$(($fail+1))
fi

message "Test --get option with --nodisplay and --all"
run_command "$COMMAND --get -f out.txt --nodisplay --all --dir $JOB_OUTPUT_DIR/perusal $JOBID" 0
if [[ $? -eq 0 ]] ; then
	run_command "ls $JOB_OUTPUT_DIR/perusal/out.txt-*" 0
	if [[ $? -ne 0 ]] ; then
  	message "No output seem to be obtained using glite-wms-job-perusal !"
		fail=$(($fail+1))
  else
		verbose "Concatenate retrieved chunckes"
		for chunck in `ls $JOB_OUTPUT_DIR/perusal/out.txt-*` ; do
			debug "Next chunck is $chunck"
			run_command "cat $chunck >> ${JOB_OUTPUT_DIR}/perusal/out.txt" 0
		done
		verbose "Retrieve the output"
		run_command "glite-wms-job-output --noint --nosubdir --dir $JOB_OUTPUT_DIR $JOBID" 0
		if [[ $? -eq 0 ]] ; then
			verbose "Check if the output file differs from the previous retrieved ones"
			run_command "diff $JOB_OUTPUT_DIR/perusal/std.out-* $JOB_OUTPUT_DIR/std.out" 0
			if [[ $? -eq 0 ]] ; then
				run_command "diff $JOB_OUTPUT_DIR/perusal/out.txt $JOB_OUTPUT_DIR/out.txt" 0
				if [[ $? -eq 0 ]] ; then
    			message "Test success"
				else
					message "Test fails. Output file out.txt is not as expected"
					fail=$(($fail+1))
				fi
			else
				message "Test fails. Output file std.out is not as expected"
				fail=$(($fail+1))
			fi
		else
			message "Cannot retrieve job output"
			fail=$(($fail+1))
		fi
  fi
else
  message "Command fails"
  fail=$(($fail+1))
fi

## cleaning files
remove ${MYTMPDIR}/sleeper.sh
for file in `ls $JOB_OUTPUT_DIR/perusal/` ; do
  remove ${JOB_OUTPUT_DIR}/perusal/${file}
done
for file in `ls $JOB_OUTPUT_DIR/out.txt-*` ; do
  remove ${file}
done
rmdir $JOB_OUTPUT_DIR/perusal

# ... terminate

if [ $fail -ne 0 ] ; then
  exit_failure "$fail test(s) fail(s)"
else
  exit_success
fi

