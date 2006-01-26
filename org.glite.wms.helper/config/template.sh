#!/bin/sh

log_event() # 1 - event
{
export GLITE_WMS_SEQUENCE_CODE=`$lb_logevent\
  --jobid="$GLITE_WMS_JOBID"\
  --source=LRMS\
  --sequence="$GLITE_WMS_SEQUENCE_CODE"\
  --event="$1"\
  --node=$host\
  || echo $GLITE_WMS_SEQUENCE_CODE`
}

log_error() # 1 - reason for failure
{
  echo "$1"
  echo "$1" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$lb_logevent\
   --jobid="$GLITE_WMS_JOBID"\
   --source=LRMS\
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="$1"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
}

truncate() # 1 - file, 2 - bytes num.
{
  perl -e '
    my $file = "'$1'";
    my $len = "'$2'";
    my $trunc_ok=0;
    if (open(F,">> ".$file)) {
      $trunc_ok=1 if (truncate(F, $len));
      close(F);
    }
    if ($trunc_ok) {
      exit(0)
    } else {
      exit(1)
    }'
}

sort_output_by_size()
{
  number_of_elements=${#__output_file[@]}
  comparisons=`expr $number_of_elements \- 1`
  count=1
  while [ $comparisons -gt 0 ];
  do
    index=0
    while [ $index -lt $comparisons ];
    do
      fs_1=`stat -t ${__output_file[$index]} | awk '{print $2}'`
      index2=`expr $index + 1`
      fs_2=`stat -t ${__output_file[$index2]} | awk '{print $2}'`
      if [ $fs_1 -gt $fs_2 ]; then
        index2=`expr $index + 1`
        temp=${__output_file[$index]}
        __output_file[$index]=${__output_file[$index2]}
        __output_file[$index2]=$temp
      fi
      index=`expr $index + 1`
    done
    comparisons=`expr $comparisons \- 1`
    count=`expr $count + 1`
  done
}

globus_url_retry_copy() # 1 - source, 2 - dest
{
  count=0
  succeded=0
  sleep_time=0
  while [ $count -le ${__file_tx_retry_count} -a $succeded -eq 0 ];
  do
    time_left=`grid-proxy-info -timeleft 2> /dev/null` || 0;
    if [ $time_left -lt $sleep_time ]; then
      return 1
    fi
    sleep "$sleep_time"
    if [ $sleep_time -eq 0 ]; then
      sleep_time=300
    else
      sleep_time=`expr $sleep_time \* 2`
    fi
    globus-url-copy $1 $2
    succeded=$?
    count=`expr $count + 1`
  done
  return $succeded
}

doExit() { # 1 - status
  status=$1

  echo "jw exit status = ${status}"
  echo "jw exit status = ${status}" >> "${maradona}"

  globus_url_retry_copy "file://${workdir}/${maradona}" "${__maradonaprotocol}"
  if [ $? != 0 ]; then
    $status=$?
  fi

  cd ..
  rm -rf ${newdir}

  exit $status
}

doDSUploadTmp()
{
  filename="${__dsupload}"
  echo "#" >> $filename.tmp
  echo "# Autogenerated by JobWrapper!" >> $filename.tmp
  echo "#" >> $filename.tmp
  echo "# The file contains the results of the upload and registration" >> $filename.tmp
  echo "# process in the following format:" >> $filename.tmp
  echo "# <outputfile> <lfn|guid|Error>" >> $filename.tmp
  echo "" >> $filename.tmp
}

doDSUpload()
{
  filename="${__dsupload}"
  mv -fv $filename.tmp $filename
}

doCheckReplicaFile()
{
  sourcefile=$1
  filename="${__dsupload}"
  exit_status=0
  if [ ! -f "${workdir}/$sourcefile" ]; then
    echo "$sourcefile    Error: File $sourcefile has not been found on the WN $host" >> $filename.tmp
    exit_status=1
  fi
  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFile()
{
  sourcefile=$1
  filename="${__dsupload}"
  exit_status=0

  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $local" >> $filename.tmp
  else
    echo "$sourcefile    Error: $local" >> $filename.tmp
    exit_status=1
  fi
  
  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFilewithLFN()
{
  sourcefile=$1
  lfn=$2
  filename="${__dsupload}"
  exit_status=0
  
  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $lfn" >> $filename.tmp
  else
    localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
    result=$?
    if [ $result -eq 0 ]; then
      echo "$sourcefile $localnew" >> $filename.tmp
    else
      echo "$sourcefile Error: $local; $localnew" >> $filename.tmp
      exit_status=1
    fi
  fi
  
  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFilewithSE()
{
  sourcefile=$1
  se=$2
  filename="${__dsupload}"
  exit_status=0

  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -d $se 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile   $local" >> $filename.tmp
  else
    localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
    result=$?
    if [ $result -eq 0 ]; then
      echo "$sourcefile $localnew" >> $filename.tmp
    else
      echo "$sourcefile Error: $local; $localnew" >> $filename.tmp
      exit_status=1
    fi
  fi

  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFilewithLFNAndSE()
{

  sourcefile=$1
  lfn=$2
  se=$3
  filename=${__dsupload}
  exit_status=0

  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn -d $se 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $lfn" >> $filename.tmp
  else
    localse=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -d $se 2>&1`
    result=$?
    if [ $result -eq 0 ]; then
      echo "$sourcefile    $localse" >> $filename.tmp
    else
      locallfn=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn 2>&1`
      result=$?
      if [ $result -eq 0 ]; then 
        echo "$sourcefile    $locallfn" >> $filename.tmp
      else
        localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
        result=$?
        if [ $result -eq 0 ]; then
          echo "$sourcefile    $localnew" >> $filename.tmp
        else
          echo "$sourcefile    Error: $local; $localse; $locallfn; $localnew" >> $filename.tmp
          exit_status=1
        fi    
      fi
    fi
  fi
	  
  echo "" >> $filename.tmp
  return $exit_status
}

function send_partial_file {
  # Use local variables to avoid conflicts with main program
  local TRIGGERFILE=$1
  local DESTURL=$2
  local POLL_INTERVAL=$3
  local FILESUFFIX=partialtrf
  local GLOBUS_RETURN_CODE
  local SLICENAME
  local LISTFILE=`pwd`/listfile.$$
  local LAST_CYCLE=""
  local SLEEP_PID
  local MD5
  local OLDSIZE
  local NEWSIZE
  local COUNTER
  # Loop forever (need to be killed by main program)
  while [ -z "$LAST_CYCLE" ] ; do
    # Go to sleep, but be ready to wake up when the user job finishes
    sleep $POLL_INTERVAL & SLEEP_PID=$!
    trap 'LAST_CYCLE="y"; kill -ALRM $SLEEP_PID >/dev/null 2>&1' USR2
    wait $SLEEP_PID >/dev/null 2>&1
    # Retrieve the list of files to be monitored
    if [ "${TRIGGERFILE:0:9}" == "gsiftp://" ]; then
      globus-url-copy ${TRIGGERFILE} file://${LISTFILE}
    elif [ "${TRIGGERFILE:0:8}" == "https://" ]; then
      htcp ${TRIGGERFILE} file://${LISTFILE}
    fi
    # Skip iteration if unable to get the list
    # (can be used to switch off monitoring)
    if [ "$?" -ne "0" ] ; then
      continue
    fi
    for SRCFILE in `cat $LISTFILE` ; do
      # SRCFILE must contain the full path
      if [ "$SRCFILE" == "`basename $SRCFILE`" ] ; then
        SRCFILE=`pwd`/$SRCFILE
      fi
      if [ -f $SRCFILE ] ; then
        # Point to the "state" variables of the current file
        # (we will use indirect reference)
        MD5=`expr "$(echo $SRCFILE | md5sum)" : '\([^ ]*\).*'`
        OLDSIZE="OLDSIZE_$MD5"
        COUNTER="COUNTER_$MD5"
        # Initialize variables if referenced for the first time
        if [ -z "${!OLDSIZE}" ]; then eval local $OLDSIZE=0; fi
        if [ -z "${!COUNTER}" ]; then eval local $COUNTER=1; fi
        # Make a snapshot of the current file
        cp $SRCFILE ${SRCFILE}.${FILESUFFIX}
        NEWSIZE=`stat -c %s ${SRCFILE}.${FILESUFFIX}`
        if [ "${NEWSIZE}" -gt "${!OLDSIZE}" ] ; then
          let "DIFFSIZE = NEWSIZE - $OLDSIZE"
          SLICENAME=$SRCFILE.`date +%Y%m%d%H%M%S`_${!COUNTER}
          tail -c $DIFFSIZE ${SRCFILE}.${FILESUFFIX} > $SLICENAME
          if [ "${DESTURL:0:9}" == "gsiftp://" ]; then
            globus-url-copy file://$SLICENAME ${DESTURL}/`basename $SLICENAME`
          elif [ "${DESTURL:0:8}" == "https://" ]; then
            htcp file://$SLICENAME ${DESTURL}/`basename $SLICENAME`
          fi
          GLOBUS_RETURN_CODE=$?
          rm ${SRCFILE}.${FILESUFFIX} $SLICENAME
          if [ "$GLOBUS_RETURN_CODE" -eq "0" ] ; then
            let "$OLDSIZE = NEWSIZE"
            let "$COUNTER += 1"
          fi # else we will send this chunk toghether with the next one
        fi # else the file size didn't increase
      fi
    done
  done
  # Do some cleanup
  if [ -f "$LISTFILE" ] ; then rm $LISTFILE ; fi
}

if [ "${__input_base_url}:-1" != "/" ]; then
  __input_base_url="${__input_base_url}/"
fi

if [ "${__output_base_url}:-1" != "/" ]; then
  __output_base_url="${__output_base_url}/"
fi

if [ -n "${__gatekeeper_hostname}" ]; then
  export GLITE_WMS_LOG_DESTINATION="${__gatekeeper_hostname}"
fi

if [ -n "${__jobid}" ]; then
  export GLITE_WMS_JOBID="${__jobid}"
fi

GLITE_WMS_SEQUENCE_CODE="$1"
shift

if [ -z "${GLITE_WMS_LOCATION}" ]; then
  export GLITE_WMS_LOCATION="${GLITE_LOCATION:-/opt/glite}"
fi

if [ -z "${EDG_WL_LOCATION}" ]; then
  export EDG_WL_LOCATION="${EDG_LOCATION:-/opt/edg}"
fi

lb_logevent=${GLITE_WMS_LOCATION}/bin/glite-lb-logevent
if [ ! -x "$lb_logevent" ]; then
  lb_logevent="${EDG_WL_LOCATION}/bin/edg-wl-logev"
fi

#customization point #1
if [ -f "${GLITE_LOCAL_CUSTOMIZATION_DIR}/cp_1.sh" ]; then
  . "${GLITE_LOCAL_CUSTOMIZATION_DIR}/cp_1.sh"
fi

if [ ${__create_subdir} -eq 1 ]; then
  if [ ${__job_type} -eq 0 -a ${__job_type} -eq 3 ]; then
    #normal or interactive
    newdir="${__jobid_to_filename}"
    mkdir ${newdir}
    cd ${newdir}
  if [ ${__job_type} -eq 1 -a ${__job_type} -eq 2 ]; then
    #MPI (LSF or PBS)
    newdir="${__jobid_to_filename}"
    mkdir -p .mpi/${newdir}
    if [ $? != 0 ]; then
      log_error "Cannot create .mpi/${newdir} directory"
    fi
    cd .mpi/${newdir}
  fi
fi

if [ ! -w . ]; then
  log_error "Working directory not writable"
fi
workdir="`pwd`"

if [ -n "${__brokerinfo}" ]; then
  export GLITE_WMS_RB_BROKERINFO="`pwd`/${__brokerinfo}"
fi

maradona="${__jobid_to_filename}.output"
touch "${maradona}"

if [ -z "${GLOBUS_LOCATION}" ]; then
  log_error "GLOBUS_LOCATION undefined"
elif [ -r "${GLOBUS_LOCATION}/etc/globus-user-env.sh" ]; then
  . ${GLOBUS_LOCATION}/etc/globus-user-env.sh
else
  log_error "${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable"
fi

for env in ${__environment[@]}
do
  eval export $env
done

umask 022

for f in ${__input_file[@]}
do
  if [ ${__wmp_support} -eq 0 ]; then
    globus_url_retry_copy "${__input_base_url}${f}" "file://${workdir}/${f}"
    if [ $? != 0 ]; then
      log_error "Cannot download ${f} from ${__input_base_url}"
    fi
  else #WMP support
    file=`basename $f`
    if [ "${f:0:9}" == "gsiftp://" ]; then
      globus-url-copy "${f}" "file://${workdir}/${file}"
    elif [ "${f:0:8}" == "https://" ]; then
      htcp "${f}" "file://${workdir}/${file}"
    fi
    if [ $? != 0 ]; then
      log_error "Cannot download ${f} from ${__input_base_url}"
    fi
  fi
done

if [ -f "${__job}" ]; then
  chmod +x "${__job}"
else
  log_error "${__job} not found or unreadable"
fi

#user prescript
if [ -x "${workdir}/pre_script.sh" ]; then
  ${workdir}/pre_script.sh
  if [ $? -ne 0]
    echo "User prescript returned with an error"
    doExit $?
  fi
fi

if [ ${__job_type} -eq 3 ]; then
  #interactive job
  for f in  "glite-wms-pipe-input" "glite-wms-pipe-output" "glite-wms-job-agent" ; do
    globus_url_retry_copy "${__input_base_url}opt/glite/bin/${f} file://${workdir}/${f}"
    chmod +x ${workdir}/${f}
  done
  globus_url_retry_copy "${__input_base_url}opt/glite/lib/libglite-wms-grid-console-agent.so.0 file://${workdir}/libglite-wms-grid-console-agent.so.0"
fi

host=`hostname -f`
log_event "Running"

if [ ${__perusal_support} -eq 1 ]; then
  send_partial_file ${__perusal_listfileuri} ${__perusal_filesdesturi} ${__perusal_timeinterval} & send_pid=$!
fi

if [ ${__token_support} -eq 1 ]; then
  value=`$GLITE_WMS_LOCATION/bin/glite-gridftp-rm ${__token_file}`
  result=$?
  if [ $result -eq 0 ]; then
    log_event "ReallyRunning"
    echo "Token ${GLITE_WMS_SEQUENCE_CODE} taken"
  else
    log_error "Cannot take token!"
  fi
fi

if [ ${__job_type} -eq 1 ]; then
  HOSTFILE=host$$
  touch ${HOSTFILE}
  for host in ${LSB_HOSTS}
    do echo $host >> ${HOSTFILE}
  done
elif [ ${__job_type} -eq 2 ]; then
  HOSTFILE=${PBS_NODEFILE}
fi
if [ ${__job_type} -eq 1 -a ${__job_type} -eq 2 ]; then #MPI LSF, PBS
  for i in `cat $HOSTFILE`; do
    ssh ${i} mkdir -p `pwd`
    /usr/bin/scp -rp ./* "${i}:`pwd`"
    ssh ${i} chmod 755 `pwd`/${__job}
  done
fi

if [ ${__job_type} -eq 0 ]; then #normal
  cmd_line="${__job} ${__arguments} $*"
elif [ ${__job_type} -eq 1 -a ${__job_type} -eq 2 ]; then #MPI LSF, PBS
  cmd_line="mpirun -np ${__nodes} -machinefile ${HOSTFILE} ${__job} ${__arguments} $*"
elif [ ${__job_type} -eq 3 ]; then #interactive
  cmd_line="./glite-wms-job-agent $BYPASS_SHADOW_HOST $BYPASS_SHADOW_PORT '${__job} ${__arguments} $*'"
fi

if [ ${__job_type} -ne 3 ]; then #all kind of but interactive
  if [ -n "${__standard_input}" ]; then
    cmd_line="$cmd_line < ${__standard_input}"
  fi
  if [ -n "${__standard_output}" ]; then
    cmd_line="$cmd_line > ${__standard_output}"
  else
    cmd_line="$cmd_line > /dev/null "
  fi
  if [ -n "${__standard_error}" ]; then
    if [ -n "${__standard_output}" ]; then
      if [ "${__standard_error}" = "${__standard_output}" ]; then
        cmd_line="$cmd_line 2>&1"
      else
        cmd_line="$cmd_line 2>${__standard_error}"
      fi
    fi
  else
    cmd_line="$cmd_line 2> /dev/null"
  fi
fi

(
  perl -e '
    unless (defined($ENV{"EDG_WL_NOSETPGRP"})) {
      $SIG{"TTIN"} = "IGNORE";
      $SIG{"TTOU"} = "IGNORE";
      setpgrp(0, 0);
    }
    exec(@ARGV);
    warn "could not exec $ARGV[0]: $!\n";
    exit(127);
  ' "$cmd_line" &

  user_job=$!

  exec 2> /dev/null

  perl -e '
    while (1) {
      $time_left = `grid-proxy-info -timeleft 2> /dev/null` || 0;
      last if ($time_left <= 0);
      sleep($time_left);
    }
    kill(defined($ENV{"EDG_WL_NOSETPGRP"}) ? 9 : -9, '"$user_job"');
    my $err_msg = "Job killed because of user proxy expiry\n";
    my $maradona = "'$maradona'";
    my $logger = "'$lb_logevent'";
    print STDERR $err_msg;
    if (open(DAT,">> ".$maradona)) {
      print DAT $err_msg;
      close(DAT);
    }
    if (open(CMD, $logger.
                  " --jobid=\"".$ENV{GLITE_WMS_JOBID}."\"".
                  " --source=LRMS".
                  " --sequence=".$ENV{GLITE_WMS_SEQUENCE_CODE}.
                  " --event=\"Done\"".
                  " --reason=\"".$err_msg."\"".
                  " --status_code=FAILED"
                  " --exit_code=0" 2>/dev/null |")) {
      chomp(my $value = <CMD>);
      close(CMD);
      my $result = $?;
      my $exit_value = $result >> 8;
      $ENV{GLITE_WMS_SEQUENCE_CODE} = $value if ($exit_value == 0);
    }
    exit(1);
	  ' &

  watchdog=$!

  wait $user_job
  status=$?
  kill -9 $watchdog $user_job -$user_job
  exit $status
)

status=$?

#customization point #2
if [ -f "${GLITE_LOCAL_CUSTOMIZATION_DIR}/cp_2.sh" ]; then
  . "${GLITE_LOCAL_CUSTOMIZATION_DIR}/cp_2.sh"
fi

if [ ${__perusal_support} -eq 1 ]; then
  kill -USR2 $send_pid
  wait $send_pid 
fi

if [ ${__output_data} -eq 1 ]; then
  return_value=0
  if [ $status -eq 0 ]; then
    local=`doDSUploadTmp`
    status=$?
    return_value=$status
    local_cnt=0
    for outputfile in ${__output_file[@]}
    do
      local=`doCheckReplicaFile ${__output_file}`
      status=$?
      if [ $status -ne 0 ]; then
        return_value=1
      else
        if [ -z "${__output_lfn}" -a -z "${__output_se}"] ; then
	       local=`doReplicaFile $outputfile`
        elif [ -n "${__output_lfn}" -a -z "${__output_se}"] ; then
	       local=`doReplicaFilewithLFN $outputfile ${__output_lfn[$local_cnt]}`
        elif [ -z "${__output_lfn}" -a -n "${__output_se}"] ; then
          local=`doReplicaFilewithSE $outputfile ${__output_se[$local_cnt]}`
        else
	       local=`doReplicaFilewithLFNAndSE $outputfile ${__output_lfn[$local_cnt]} ${__output_se[$local_cnt]}`
        fi
        status=$?
      fi
      local_cnt=`expr $local_cnt + 1`
    done
    local=`doDSUpload`
    status=$?
  fi
fi

echo "job exit status = ${status}"
echo "job exit status = ${status}" >> "${maradona}"

file_size_acc=0
total_files=${#__output_file[@]}
current_file=0
# comment this one below if the osb order list originally 
# specified may be of some relevance to the user
sort_output_by_size
for f in ${__output_file[@]} 
do
  if [ ${__wmp_support} -eq 0 ]; then
    if [ -r "${f}" ]; then
      output=`dirname $f`
      if [ "x${output}" = "x." ]; then
        ff=$f
      else
       ff=${f##*/}
      fi

      if [ ${__max_osb_size} -ge 0 ]; then
        #todo
        #if hostname=wms
          file_size=`stat -t $f | awk '{print $2}'`
          file_size_acc=`expr $file_size_acc + $file_size`
        #fi
        if [ $file_size_acc -le ${__max_osb_size} ]; then
          globus_url_retry_copy "file://${workdir}/${f}" "${__output_base_url}${ff}"
        else
          echo "OSB quota exceeded for file://${workdir}/${f}, truncating needed"
          # $current_file is zero-based (being used even
          # below as an array index), + 1 again because of the 
          # difference between $total and $current (i.e. 20-19=2 more files)
          remaining_files=`expr $total_files \- $current_file + 2`
          remaining_space=`expr $__max_osb_size \- $file_size_acc`
          trunc_len=`expr $remaining_space / $remaining_files`
          if [ $trunc_len -lt 50 ]; then
            #at least the first 50 bytes
            echo "Not enough room for a significant truncation on file ${f}"
          else
            truncate "file://${workdir}/${f}" $trunc_len
            if [ $? != 0 ]; then
              echo "Could not truncate output sandbox file ${f}"
            else
              globus_url_retry_copy "file://${workdir}/${f}" "${__output_base_url}${ff}.trunc"
            fi
          fi
        fi
      else
        globus_url_retry_copy "file://${workdir}/${f}" "${__output_base_url}${ff}"
      fi
      if [ $? != 0 ]; then
        log_error "Cannot upload ${f} into ${__output_base_url}" "Done"
      fi
    fi
  else #WMP support
    file=`basename $f`
    s="file://${workdir}/${__wmp_output_file[$current_file]}"
    if [ ${__osb_wildcards_support} -eq 0 ]; then
      d=${f}
    else
      d=${__output_sandbox_base_dest_uri}/${file}
    fi
    if [ ${__max_osb_size} -ge 0 ]; then
      #todo
      #if hostname=wms
        file_size=`stat -t $f | awk '{print $2}'`
        file_size_acc=`expr $file_size_acc + $file_size`
      #fi
      if [ $file_size_acc -le ${__max_osb_size} ]; then
        if [ "${f:0:9}" == "gsiftp://" ]; then
          globus-url-copy $s $d
        elif [ "${f:0:8}" == "https://" ]; then
          htcp $s $d
        fi
      else
        echo "OSB quota exceeded for $s, truncating needed"
        remaining_files=`expr $total_files \- $current_file + 2`
        remaining_space=`expr $__max_osb_size \- $file_size_acc`
        trunc_len=`expr $remaining_space / $remaining_files`
          if [ $trunc_len -lt 50 ]; then
            #at least the first 50 bytes
            echo "Not enough room for a significant truncation on file ${f}"
          else
          truncate "file://${workdir}/${f}" $trunc_len
          if [ $? != 0 ]; then
            echo "Could not truncate output sandbox file ${f}"
          else
            if [ "${f:0:9}" == "gsiftp://" ]; then
              globus-url-copy $s "$d.trunc"
            elif [ "${f:0:8}" == "https://" ]; then
              htcp $s "$d.trunc"
            fi
          fi
        fi
      fi
    else #unlimited osb
      if [ "${f:0:9}" == "gsiftp://" ]; then
        globus-url-copy $s $d
      elif [ "${f:0:8}" == "https://" ]; then
        htcp $s $d
      fi
    fi
    if [ $? != 0 ]; then
      log_error "Cannot upload ${file} into ${f}" "Done"
    fi
  fi
  current_file=`expr $current_file + 1`
done

log_event "Done"

if [ -n "${LSB_JOBID}" ]; then
  cat "${X509_USER_PROXY}" | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L lsf_${LSB_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H "$HLR_LOCATION"
  if [ $? != 0 ]; then
  echo "Error transferring gianduia with command: cat ${X509_USER_PROXY} | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L lsf_${LSB_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H $HLR_LOCATION"
  fi
fi

if [ -n "${PBS_JOBID}" ]; then
  cat ${X509_USER_PROXY} | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L pbs_${PBS_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H "$HLR_LOCATION"
  if [ $? != 0 ]; then
  echo "Error transferring gianduia with command: cat ${X509_USER_PROXY} | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L pbs_${PBS_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H $HLR_LOCATION"
  fi
fi

#customization point #3
if [ -f "${GLITE_LOCAL_CUSTOMIZATION_DIR}/cp_3.sh" ]; then
  . "${GLITE_LOCAL_CUSTOMIZATION_DIR}/cp_3.sh"
fi

doExit 0
