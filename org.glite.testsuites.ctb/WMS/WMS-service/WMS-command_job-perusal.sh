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

COMMAND=glite-wms-job-perusal
message "Check if command $COMMAND exists"
run_command which $COMMAND

# Test --version option
message ""
message "Test --version option"
run_command $COMMAND --version
verbose "Check the output command"
run_command grep "\"WMS User Interface version\"" <<< "$OUTPUT"
message "We are testing $OUTPUT"

remove $JDLFILE
echo "Executable = \"sleeper.sh\";" > $JDLFILE
echo "Arguments = \"out.txt\";" >> $JDLFILE
echo "StdOutput  = \"std.out\";" >> $JDLFILE
echo "StdError   = \"std.err\";" >> $JDLFILE
echo "InputSandbox = {\"${MYTMPDIR}/sleeper.sh\"};" >> $JDLFILE
echo "OutputSandbox = {\"std.out\",\"std.err\",\"out.txt\"};" >> $JDLFILE
echo "PerusalFileEnable = true;" >> $JDLFILE


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
run_command glite-wms-job-submit $DELEGATION_OPTIONS --nomsg $JDLFILE
JOBID=$OUTPUT
debug "JobID is $JOBID"

## ... enable file perusal for out.txt

message ""
message "Test --set option"
run_command $COMMAND --set --filename out.txt -f std.out $JOBID

message ""
message "BEWARE default min perusal interval is 1000 secs, so this phase could take many minutes"
message "Test --get option"
while is_finished $JOBID
do
  sleep 60
  run_command $COMMAND --get -f out.txt --dir $JOB_OUTPUT_DIR $JOBID
done

verbose "List the retrieved parts of out.txt"
run_command ls -l ${JOB_OUTPUT_DIR} | grep out.txt
message "Test --get option with --dir"
run_command $COMMAND --get -f std.out --dir $JOB_OUTPUT_DIR/perusal $JOBID
if ! ls $JOB_OUTPUT_DIR/perusal/std.out-* ; then
  exit_failure "No output seem to be obtained using glite-wms-job-perusal !"
fi
message "Test --get option with --nodisplay and --all"
run_command $COMMAND --get -f out.txt --noint --nodisplay --all --dir $JOB_OUTPUT_DIR/perusal $JOBID
if ! ls $JOB_OUTPUT_DIR/perusal/out.txt-* ; then
  exit_failure "No output seem to be obtained using glite-wms-job-perusal !"
fi

verbose "Concatenate retrieved chunckes"
for chunck in `ls $JOB_OUTPUT_DIR/perusal/out.txt-*` ; do
  debug "Next chunck is $chunck"
  run_command "cat $chunck >> ${JOB_OUTPUT_DIR}/perusal/out.txt"
done


verbose "Retrieve the output"
run_command glite-wms-job-output --noint --nosubdir --dir $JOB_OUTPUT_DIR $JOBID

verbose "Check if the output file differs from the previous retrieved ones"
run_command diff $JOB_OUTPUT_DIR/perusal/std.out-* $JOB_OUTPUT_DIR/std.out
run_command diff $JOB_OUTPUT_DIR/perusal/out.txt $JOB_OUTPUT_DIR/out.txt
  

## cleaning files
remove ${MYTMPDIR}/sleeper.sh
for file in `ls $JOB_OUTPUT_DIR/perusal/` ; do
  remove ${JOB_OUTPUT_DIR}/perusal/${file}
done
for file in `ls $JOB_OUTPUT_DIR/out.txt-*` ; do
  remove ${file}
done
rmdir $JOB_OUTPUT_DIR/perusal


exit_success
