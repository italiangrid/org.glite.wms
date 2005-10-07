#!/bin/sh

doExit() {
  stat=$1

  echo "jw exit status = ${stat}"
  echo "jw exit status = ${stat}" >> "${maradona}"

  globus-url-copy "file://${workdir}/${maradona}" "${__maradonaprotocol}"

  cd ..
  rm -rf ${newdir}

  exit $stat
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
  export GLITE_WMS_LOCATION="${__edg_location}"
fi

if [ $__create_subdir -eq 1 ]; then
  newdir="${__jobid_to_filename}"
  mkdir -p ".mpi/${newdir}"
  if [ $? != 0 ]; then
    echo "Cannot create .mpi/${newdir} directory"

    GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
      --jobid="$GLITE_WMS_JOBID" \
      --source=LRMS \
      --sequence="$GLITE_WMS_SEQUENCE_CODE"\
      --event="Done"\
      --reason="Cannot create ".mpi/"${newdir} directory"\
      --status_code=FAILED\
      --exit_code=0\
      || echo $GLITE_WMS_SEQUENCE_CODE`
    export GLITE_WMS_SEQUENCE_CODE

  exit 1
fi
cd ".mpi/${newdir}"

if [ ! -w . ]; then
  echo "Working directory not writable"

 export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
  --jobid="$GLITE_WMS_JOBID" \
  --source=LRMS \
  --sequence="$GLITE_WMS_SEQUENCE_CODE"\
  --event="Done"\
  --reason="Working directory not writable!"\
  --status_code=FAILED\
  --exit_code=0\
  || echo $GLITE_WMS_SEQUENCE_CODE`

  exit 1
fi

if [ -n "${__brokerinfo}" ]; then
  export GLITE_WMS_RB_BROKERINFO="`pwd`/${__brokerinfo}"
fi

maradona="${__jobid_to_filename}.output"
touch "${maradona}"

if [ -z "${GLOBUS_LOCATION}" ]; then
  echo "GLOBUS_LOCATION undefined"
  echo "GLOBUS_LOCATION undefined" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
   --jobid="$GLITE_WMS_JOBID" \
   --source=LRMS \
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="GLOBUS_LOCATION undefined"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
elif [ -r "${GLOBUS_LOCATION}/etc/globus-user-env.sh" ]; then
  . ${GLOBUS_LOCATION}/etc/globus-user-env.sh
else
  echo "${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable"
  echo "${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
   --jobid="$GLITE_WMS_JOBID" \
   --source=LRMS \
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
fi

for env in ${__environment[@]}
do
  eval export $env
done

umask 022

if [ $__wmp_support -eq 0 ]; then
  for f in ${__input_file[@]}
  do
    globus-url-copy "${__input_base_url}${f}" "file://${workdir}/${f}"
    if [ $? != 0 ]; then
      echo "Cannot download ${f} from ${__input_base_url}"
      echo "Cannot download ${f} from ${__input_base_url}" >> "${maradona}"

      export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
       --jobid="$GLITE_WMS_JOBID" \
       --source=LRMS \
       --sequence="$GLITE_WMS_SEQUENCE_CODE"\
       --event="Done"\
       --reason="Cannot download ${f} from ${__input_base_url}"\
       --status_code=FAILED\
       --exit_code=0\
       || echo $GLITE_WMS_SEQUENCE_CODE`
      doExit 1
    fi
  done
else
  #WMP support
  for f in ${__wmp_input_base_file[@]}
  do
    file=`basename $f`
    globus-url-copy "${f}" "file://${workdir}/${file}"
    if [ $? != 0 ]; then
      echo "Cannot download ${file} from ${f}"
      echo "Cannot download ${file} from ${f}" >> "${maradona}"

      export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
       --jobid="$GLITE_WMS_JOBID" \
       --source=LRMS \
       --sequence="$GLITE_WMS_SEQUENCE_CODE"\
       --event="Done"\
       --reason="Cannot download ${file} from ${f}"\
       --status_code=FAILED\
       --exit_code=0\
       || echo $GLITE_WMS_SEQUENCE_CODE`
      doExit 1
    fi
  done
fi

if [ -e "${__jobid}" ]; then
  chmod +x "${__jobid}" 2> /dev/null
else
  echo "${__jobid} not found or unreadable"
  echo "${__jobid} not found or unreadable" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
   --jobid="$GLITE_WMS_JOBID" \
   --source=LRMS \
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="/bin/echo not found or unreadable!"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
fi

host=`hostname -f`
export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGENVENT \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="Running"\
 --node=$host\
 || echo $GLITE_WMS_SEQUENCE_CODE`

value=`$GLITE_WMS_LOCATION/bin/glite-gridftp-rm $__token_file`
result=$?
if [ $result -eq 0 ]; then
  GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="ReallyRunning"\
 --status_code=\
 --exit_code=\
 || echo $GLITE_WMS_SEQUENCE_CODE`
export GLITE_WMS_SEQUENCE_CODE

  echo "Take token: ${GLITE_WMS_SEQUENCE_CODE}"
else
  echo "Cannot take token!"
  echo "Cannot take token!" >> "${maradona}"

  GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="Done"\
 --reason="Cannot take token!"\
 --status_code=FAILED\
 --exit_code=0\
 || echo $GLITE_WMS_SEQUENCE_CODE`
export GLITE_WMS_SEQUENCE_CODE

  doExit 1
fi

HOSTFILE=host$$
touch ${HOSTFILE}
for host in ${LSB_HOSTS}
  do echo $host >> ${HOSTFILE}
done

for i in `cat $HOSTFILE`; do
  ssh "${i} mkdir -p `pwd`"
  /usr/bin/scp -rp "./* ${i}:`pwd`"
  ssh "${i} chmod 755 `pwd`/${__job}"
done

cmd_line="mpirun -np ${__nodes} -machinefile ${HOSTFILE} ${__job} ${__arguments} $*"
if [ -n "${__standard_input}" ]; then
  cmd_line="$cmd_line < ${__standard_input}"
fi
if [ -n "${__standard_output}" ]; then
  cmd_line="$cmd_line > ${__standard_output}"
else
  cmd_line="$cmd_line > /dev/null"
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
  cmd_line="$cmd_line 2 > /dev/null"
fi

status=$?
echo "job exit status = ${status}"
echo "job exit status = ${status}" >> "${maradona}"

error=0
if [ $__wmp_support -eq 0 ]; then
  for f in ${__output_file[@]} 
  do
    if [ -r "${f}" ]; then
      output=`dirname $f`
      if [ "x${output}" = "x." ]; then
        ff=$f
      else
       ff=${f##*/}
      fi
      globus-url-copy "file://${workdir}/${f}" "${__output_base_url}${ff}"
      if [ $? != 0 ]; then
        echo "Cannot upload ${f} into ${__output_base_url}"
        echo "Cannot upload ${f} into ${__output_base_url}" >> "${maradona}"

        export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
         --jobid="$GLITE_WMS_JOBID" \
         --source=LRMS \
         --sequence="$GLITE_WMS_SEQUENCE_CODE"\
         --event="Done"\
         --reason="Cannot upload ${f} into ${__output_base_url}"\
         --status_code=FAILED\
         --exit_code=0\
         || echo $GLITE_WMS_SEQUENCE_CODE`
        doExit 1
      fi
    fi
  done
else
  #WMP support
  for f in ${__wmp_output_dest_file[@]} 
  do
    file=`basename $f`
    if [ -r "${file}" ]; then
      output=`dirname $f`
      if [ "x${output}" = "x." ]; then
        ff=$f
      else
       ff=${f##*/}
      fi
      globus-url-copy "file://${workdir}/${file}" "${f}"
      if [ $? != 0 ]; then
        echo "Cannot upload ${file} into ${f}"
        echo "Cannot upload ${file} into ${f}" >> "${maradona}"

        export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
         --jobid="$GLITE_WMS_JOBID" \
         --source=LRMS \
         --sequence="$GLITE_WMS_SEQUENCE_CODE"\
         --event="Done"\
         --reason="Cannot upload ${file} into ${f}"\
         --status_code=FAILED\
         --exit_code=0\
         || echo $GLITE_WMS_SEQUENCE_CODE`
        doExit 1
      fi
    fi
  done
fi

export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="Done"\
 --status_code=OK\
 --exit_code=$status\
 || echo $GLITE_WMS_SEQUENCE_CODE`

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

doExit 0

