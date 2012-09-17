#!/bin/bash
#
# File:     blah_exerciser.sh
# Authors:   Francesco Prelz (francesco.prelz@mi.infn.it),
#            SA3-IT <sa3-italia@mi.infn.it>
#
# Revision history:
#     21-Dec-2007: Original release
#     28-Nov-2008: Revision by SA3-IT <sa3-italia@mi.infn.it>
#
# Description:
#     Script to exercise the BLAH daemon by submitting, holding, release and monitoring jobs.
#



blex_submitted_jobs=""
blex_pending_submits=""
blex_pending_status=""
blex_pending_other=""
blex_n_success=0
blex_n_failure=0
blex_n_nojobid=0
blex_n_pending_commands=0
blex_loop_sleep_per_pending_command=2
blex_loop_sleep=0
blex_test_command=0
blex_start_submit=0
blex_end_submit=0
blex_end_all=0
blex_other_command_list=""

IFS=$' \t\n\r'


. $(dirname $0)/functions.sh


#load configuration variables
  load_conf_vars $@

. ./blah_exerciser_holdrelease_test.sh

#
# Create a pair of named pipes, 3 and 4, for communication with BLAH
  create_blah_in_out_pipe

read <&4
echo "REPLY == $REPLY"

if [ $blex_setup_glexec -gt 0 ]
then
  blex_other_command="CACHE_PROXY_FROM_FILE 1 $blex_x509userproxy"
  blex_enqueue_other_command
  blex_other_command="USE_CACHED_PROXY 1"
  blex_enqueue_other_command
  blex_submit_other_commands 0 # Synchronous commands
fi

blex_n_submitted_jobs=0

blex_start_submit=$(awk "BEGIN { print systime() }")
for blex_job_type in $blex_possible_job_types
do
  eval "blex_n_jobs=\$blex_n_${blex_job_type}_jobs"
  echo "Tot number of jobs being submitted:$blex_n_jobs"
  for (( i=0 ; i<blex_n_jobs ; i++ ))
  do
    if [ "$blex_job_type" == "DEFAULT" ]
    then
      blex_compose_submit_ad $blex_desired_batchsystem $blex_desired_queue
      blex_file_unique=""
    else
      eval "blex_compose_submit_ad_${blex_job_type} $blex_desired_batchsystem $blex_desired_queue"
    fi

    if [ "x$blex_submit_ad" != "x" ]
    then
      blex_compose_submit_command
      
      if [ $blex_debug -gt 0 ]
      then 
        echo "DEBUG: submit command: <$blex_submit_command>"
      fi
      
      echo $blex_submit_command >&3
 
      read blex_submit_result junk <&4

      if [ "$blex_submit_result" == "S" ]
      then
        blex_pending_submits="$blex_pending_submits $blex_sequence_no|$blex_job_type/$blex_file_unique"
        let blex_n_submitted_jobs++
      else
        failure "$0: ERROR submitting job $blex_submit_ad"
      fi
    else
      failure "$0: ERROR creating submit ad for $blex_job_type job" 
    fi
    # Give time for submit not to clog the local system
    sleep 5
    if [ -e /tmp/blah_err ];
    then
     count=1
     blaherr=0
     #Check for errors returned by BLAH
     cat /tmp/blah_err | while read line; do
     if [ "${line:0:5}" == "ERROR" ]; then 
       blaherr=1 
       echo "exiting: ${line}"
       /bin/rm /tmp/blah_err
       exit
     fi; 
     done
     if [ -e /tmp/blah_err ]; then
      /bin/rm /tmp/blah_err
     else
      failure 
     fi
    fi 
  done
done

blex_initial_submitted_jobs=$blex_n_submitted_jobs

while (( blex_n_submitted_jobs > 0 ))
do
  blex_wait_for_results
  blex_n_submitted_jobs=0
  for submitted_job in $blex_submitted_jobs
  do
    let blex_n_submitted_jobs++

    blex_compose_status_command $submitted_job

    if [ $blex_debug -gt 0 ]
    then 
      echo "DEBUG: status command: <$blex_status_command>"
    fi

    echo $blex_status_command >&3
  
    read blex_status_result junk <&4
    echo "status result: $blex_status_result"
    echo "junk: $junk"
    if [ "$blex_status_result" == "S" ]
    then
      blex_pending_status="$blex_pending_status $blex_sequence_no"
      if [ $blex_test_command -eq 0 ]
      then
        blex_test_command=$blex_sequence_no
        blex_test_command_start=$(awk "BEGIN { print systime() }")
      fi
    else
      #echo "$0: ERROR requesting status of job $submitted_job" 2>&1
      failure "$0: ERROR requesting status of job $submitted_job" 
    fi


  done

  if [ $blex_debug -gt 0 ]
  then 
    echo "DEBUG: $blex_n_submitted_jobs job(s) left."
  fi
#
# Wait for a time proportional to the number of *pending* commands
#
  blex_loop_sleep=0
  blex_n_pending_commands=0
  for junk in $blex_pending_submits
  do
    let blex_loop_sleep+=blex_loop_sleep_per_pending_command
    let blex_n_pending_commands++
  done
  for junk in $blex_pending_status
  do
    let blex_loop_sleep+=blex_loop_sleep_per_pending_command
    let blex_n_pending_commands++
  done
  for junk in $blex_pending_other
  do
    let blex_loop_sleep+=blex_loop_sleep_per_pending_command
    let blex_n_pending_commands++
  done
  if [ $blex_n_submitted_jobs -gt 0 ]
  then
    if [ $blex_debug -gt 0 ]
    then
      echo "DEBUG: sleeping $blex_loop_sleep seconds and continuing"
    fi
    sleep $blex_loop_sleep
  fi
done

blex_end_all=$(awk "BEGIN { print systime() }")

echo "QUIT" >&3

exec 3>&-
exec 4<&-

/bin/rm -f $blex_pipin $blex_pipout


echo "$0 exiting: Submitted $blex_initial_submitted_jobs jobs. $blex_n_success succeeded. $blex_n_nojobid returned no jobid. $blex_n_failure failed."
echo "Took $(( blex_end_submit - blex_start_submit )) seconds to submit."
echo "Took $(( blex_end_all - blex_start_submit )) seconds to complete."

success
