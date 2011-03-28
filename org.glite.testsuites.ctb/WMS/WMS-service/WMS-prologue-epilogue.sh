#!/bin/sh

###############################################################################
#
# Test prologue/epilogue attribute
#
# Author: Alessio Gianelle <gianelle@pd.infn.it>
#
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

err=""

function set_jobjdl()
{
	echo "
	Executable = \"exe.sh\";
	Arguments = \"Executable Arguments\";
	StdOutput = \"std.out\";
	Prologue = \"prologue.sh\";
	PrologueArguments = \"Prologue Arguments\";
	Epilogue = \"epilogue.sh\";
	EpilogueArguments = \"Epilogue Arguments\";
	Environment={\"VAR=TestVariable\"};
	InputSandbox = {\"${MYTMPDIR}/exe.sh\", \"${MYTMPDIR}/epilogue.sh\",\"${MYTMPDIR}/prologue.sh\"};
	OutputSandbox = {\"std.out\", \"prologue.out\", \"epilogue.out\"};
	" >> ${MYTMPDIR}/PE.jdl
	
	echo "
	#!/bin/sh

	echo \"##########################\" >> prologue.out
	echo \"This is the prologue script\" >> prologue.out
	echo \"Start running at \`date +%H:%M:%S\`\" >> prologue.out
	echo \"My aurguments are: \$@\" >> prologue.out
	echo \"Check the value of the environment variable: \$VAR\" >> prologue.out
	echo \"Now we 'touch' the file 'prologue'\" >> prologue.out
	touch prologue
	echo \"Finish running at \`date +%H:%M:%S\`\" >> prologue.out
	echo \"##########################\" >> prologue.out
	" >> ${MYTMPDIR}/prologue.sh

	echo "
#!/bin/sh

echo \"##########################\" 
echo \"This is the executable\" 
echo \"Start running at \`date +%H:%M:%S\`\" 
echo \"Mine aurguments are: \$@\"
echo \"Check the presence of the file 'prologue'\"
ls -l prologue
echo \"Check the value of the environment variable: \$VAR\"
echo \"Now we 'touch' the file 'executable'\"
touch executable
echo \"Finish running at \`date +%H:%M:%S\`\"
echo \"##########################\" 
" >> ${MYTMPDIR}/exe.sh

echo "
#!/bin/sh

echo \"##########################\"  >> epilogue.out
echo \"This is the epilogue\"  >> epilogue.out
echo \"Start running at \`date +%H:%M:%S\`\" >> epilogue.out
echo \"Mine aurguments are: \$@\" >> epilogue.out
echo \"Check the value of the environment variable: \$VAR\" >> epilogue.out
echo \"Check the presence of the files 'prologue' and 'executable'\" >> epilogue.out
ls -l prologue >> epilogue.out
ls -l executable >> epilogue.out
echo \"Finish running at \`date +%H:%M:%S\`\" >> epilogue.out
echo \"All the jokes are done!\" >> epilogue.out
echo \"##########################\"  >> epilogue.out
" >> ${MYTMPDIR}/epilogue.sh

return 0
}

prepare "test prologue and epilogue attributes" $@

message ""
message "Test prologue and epilogue attributes"
set_jobjdl
run_command "glite-wms-job-submit $DELEGATION_OPTIONS --config $CONFIG_FILE --nomsg ${MYTMPDIR}/PE.jdl" 1 "Job submission failed"
JOBID=$OUTPUT
wait_until_job_finishes $JOBID
get_STATUS $JOBID
if [[ "$JOBSTATUS" == "Done (Success)" ]]; then
	verbose "Retrieve the output"
	run_command "glite-wms-job-output --dir $JOB_OUTPUT_DIR --noint --nosubdir $JOBID" 1 "I'm not able to retrieve output file for $JOBID"
	verbose "Check if the output files are correctly retrieved"
	if [ -f ${JOB_OUTPUT_DIR}/prologue.out ] && 
			[ -f ${JOB_OUTPUT_DIR}/std.out ] && 
   		[ -f ${JOB_OUTPUT_DIR}/epilogue.out ] ; then
  		verbose "All output files are retrieved"  
			# Check prologue output
			run_command "grep 'My aurguments are: Prologue Arguments' ${JOB_OUTPUT_DIR}/prologue.out" 1 "PrologueArguments attribute is not correctly set"
			run_command "grep 'Check the value of the environment variable: TestVariable' ${JOB_OUTPUT_DIR}/prologue.out" 1 "Environment variable is not correctly set"
 			# Check executable output
      run_command "grep 'Mine aurguments are: Executable Arguments' ${JOB_OUTPUT_DIR}/std.out" 1 "Arguments attribute is not correctly set"  
      run_command "grep 'Check the value of the environment variable: TestVariable' ${JOB_OUTPUT_DIR}/std.out" 1 "Environment variable is not correctly set"
			run_command "grep -- '-rw-r--r--' ${JOB_OUTPUT_DIR}/std.out | grep prologue" 1 "Prologue not run correctly"
			# Check epilogue output 
      run_command "grep 'Mine aurguments are: Epilogue Arguments' ${JOB_OUTPUT_DIR}/epilogue.out" 1 "Arguments attribute is not correctly set"               
      run_command "grep 'Check the value of the environment variable: TestVariable' ${JOB_OUTPUT_DIR}/epilogue.out" 1 "Environment variable is not correctly set"
      run_command "grep -- '-rw-r--r-- ' ${JOB_OUTPUT_DIR}/epilogue.out | grep prologue" 1 "Prologue not run correctly"  
      run_command "grep -- '-rw-r--r-- ' ${JOB_OUTPUT_DIR}/epilogue.out | grep executable" 1 "User's job not run correctly"  
			verbose "Output files are as expected"
	else
		err="FAIL. Output files are not retrieved"
	fi
else
	err="Warning. Job finishes with status: $JOBSTATUS; cannot retrieve output"
fi

## remove used files
remove ${MYTMPDIR}/epilogue.sh 
remove ${MYTMPDIR}/exe.sh 
remove ${MYTMPDIR}/prologue.sh 
remove ${MYTMPDIR}/PE.jdl
rm -rf ${JOB_OUTPUT_DIR}

if [ x"$err" != "x" ]; then
	exit_failure $err
else
  exit_success
fi

	
	


