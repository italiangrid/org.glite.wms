##############################################################################
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the copyright
# holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################
#
# AUTHORS: Maarten Litmaath, Laurence Field, Andreas Unterkircher CERN
#
##############################################################################

function regtest_timeout()
{
 (
  TMP_FILE=${TMPDIR-/tmp}/timeout.$$

  perl -e '
   $SIG{"TTIN"} = "IGNORE";
   $SIG{"TTOU"} = "IGNORE";
   setpgrp(0, 0);
   exec(@ARGV);
   warn "could not exec $ARGV[0]: $!\n";
   exit(127);
  ' "$@" &

  user_job=$!
  exec 2> /dev/null
  
  perl -e '
   sleep('"$REGTEST_TIMEOUT"');
   kill(-9, '"$user_job"');
   open(LOCAL_FILE,">'"$TMP_FILE"'");
   close LOCAL_FILE;
   exit(222);
  ' &

  watchdog=$!

  wait $user_job
  status=$?

  /bin/kill -9 $watchdog -$user_job

  if [ -e $TMP_FILE ]; then
   status=$REGTEST_TIMEOUT_ERROR
   rm -f $TMP_FILE
  fi

  exit $status
 )
}


function run_command_timeout() {
 command=$1
 message=$2
 echo -n "${message} ... "
 
 output=$(eval regtest_timeout "$command" 2>&1)

 retval=$?

 if [ $retval -eq $REGTEST_TIMEOUT_ERROR ]; then
  echo
  echo "TIMEOUT after $REGTEST_TIMEOUT sec. of command"
  echo $command
  return $REGTEST_TIMEOUT_ERROR
 fi
 if [ $retval -gt 0 ]; then
  echo "Failed"
  echo "${command}" >&2
  export OUTPUT=${output}
  return ${REGTEST_FAIL}
 fi

 echo "OK"
 export OUTPUT=${output}
 return ${REGTEST_OK}
}

function unique_name() {
uni_pid=$$
uni_time=`date +%s`
name="$uni_pid$uni_time"
}

function run_command() {
 command=$1
 message=$2
 echo -n "${message} ... "
 
 output=$(eval "${command}" 2>&1)

 if [ $? -gt 0 ]; then
  echo "Failed"
  echo "${command}" >&2
  echo "${output}"
  return ${REGTEST_FAIL}
 fi
 
 echo "OK"
 export OUTPUT=${output}
 return ${REGTEST_OK}
}

