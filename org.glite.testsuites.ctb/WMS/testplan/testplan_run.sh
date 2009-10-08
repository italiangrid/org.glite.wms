#!/bin/sh

statuses[0]=Submitted
statuses[1]=Waiting
statuses[2]=Ready
statuses[3]=Scheduled
statuses[4]=Running
statuses[5]=Done
statuses[6]=Abort

MAX_STATUS_ITERATIONS=100
CHECK_STATUS_INTERVAL=30

log() {
  echo "`date`: $1"
}

report_header() {
  echo "<html>" > $1
  echo "<head>" >> $1
  echo "..." >> $1
  echo "</head>" >> $1
  echo "<body>" >> $1
  echo "<table border=\"1\">" >> $1
}

report_line() {
  echo "<tr><td>$1</td><td>$2</td></tr>" >> $3
}

report_footer() {
  echo "</table>" >> $1
  echo "</body>" >> $1
  echo "</html>" >> $1
}

get_status() { # 1 - joibid
  jobstatus=`glite-wms-job-status $1|awk '/urrent Status/{print $3}'|| echo ERROR`
}

submit_single_job() { # 1 - jdl  2 - wms.conf / returns jobid
  tmpfile=`mktemp tmp.XXXXXXXXXX` || exit 1
  glite-wms-job-submit --noint -o $tmpfile -a -c $2 $1
  if [ $? != 0 ]; then
    jobid="ERROR"
  else
    jobid=`tail -1 $tmpfile` 
  fi
}

usage=0
exit_code=-1
exit_while=-1
if [ -z $1 ]; then
  usage=1
else
  if [ -z $2 ]; then
    usage=1
  fi
fi
if [ $usage -eq 1 ]; then
  echo "usage: testplan_run test_dir wms.conf"
else
	report_name="$app_path/report.html"
	rm -rf $report_name
	report_header $report_name
  app_path=`pwd`
	cd $1
	for jdl in `find . -name \*jdl -maxdepth 1 -type f`; do
    log "processing $jdl"
    submit_single_job $jdl "$app_path/$2"
		base="${jdl:0:${#jdl}-4}"

    if [ -r "${base}_pre" ]; then
      . "${base}_pre" # precondition
    fi
    if [ "x$jobid" == "xERROR" ]; then
      log "got an error while submitting..."
    else
      log "job submitted with id: $jobid"
    fi
    rm -f $tmpfile
    jobstatus=X
    iterations=1
    while [ true ]; do
      sleep $CHECK_STATUS_INTERVAL
      get_status $jobid
      if [ "X$jobstatus" == "XERROR" ]; then
        log "Error getting status for job: $jobid"
        break
      else
        log "waiting for $jobid to complete, current status: $jobstatus"
      fi
      if [ "x$jobstatus" == "x${statuses[5]}" ]; then
        exit_while=1
      fi
      if [ "x$jobstatus" == "x${statuses[6]}" ]; then
        exit_while=1
      fi
      if [ $exit_while -eq 1 ]; then
        exit_code=`glite-wms-job-status $jobid|awk '/Exit code/{print $3}' || echo 1`
        exit_status=`glite-wms-job-status -v 3  $jobid|awk '/t Status/{print $4}' || echo 1`
        log "$jobid done $exit_status with exit code $exit_code"
        break
      fi
      iterations=`expr $iterations + 1`
      if [ $iterations -gt $MAX_STATUS_ITERATIONS ]; then
        log "hit maximum number of iterations while checking for status"
        exit_code=-2
        break
      fi
    done
    if [ "x$jobstatus" == "x${statuses[5]}" ]; then # get output
      output_dir=`glite-wms-job-output $jobid|awk '/tmp/{print $3}' || echo 1`
    fi
    if [ -r "${base}_post" ]; then
      . "${base}_post" # postcondition
    fi
  done
fi
report_footer $report_name
cd -
