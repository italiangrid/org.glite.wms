#!/bin/bash

##############################################################################
# Copyright (c) Members of the EGEE Collaboration. 2010.
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
# AUTHORS:
# Gianni Pucciani
#
##############################################################################

shortinfo="Test CLI glite-transfer-cancel"

#source functions definitions
source FTS-common

showUsage()
{
 echo << EOF
 Usage: $0 [--fts <fts hostname>] [--src <source SE host>] [--dest <destination SE host>] [--bdii  <bdii hostname>] [--vo  <VO name>] [--timeout  <timeout>] [--gsiftp] [--shortinfo] [--longinfo]
 Options: 
   <fts hostname> The name of the FTS host
   <source SE host> Hostname of the source SE. Default to \$SE_SOURCE.
   <destination SE host> Hostname of the destination SE. Default to \$SE_DEST.
   <bdii hostname> The top-level BDII hostname
   <VO name> VO name. Default to dteam
   <timeout> timeout in seconds
   --gsiftp   [yes|no] If set to 'yes', GSIFTP URLs will be used (Optional. Default value: No)
   --shortinfo = print a short test description
   --longinfo = print a long test description

EOF
}

# Special tests for glite-transfer-cancel operation
# Check command return code only 
#
function test_transfer-cancel-parameters()
{
  echo "Check parameters: -s -q -v --verbose "

  echo "Check -q parameter" 
  run_command_status glite-transfer-cancel -q -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/FileTransfer 1 || return 2
  if [ "x$OUTPUT" != "x" ]; then
    echo "Error: the command returned this output '$OUTPUT'"
    return 1
  fi
 
  echo "Check -v parameter" 
  run_command_status glite-transfer-cancel -v -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/FileTransfer 1 || return 2
  echo $OUTPUT | grep -q "Service version:"
  if [ $? -ne 0 ]; then
    echo "Error: Service version not printed as expected"
    return 1
  fi

  echo "Check without -s parameter" 
  run_command_status glite-transfer-cancel -v 0 || return 2

  return 0
}

#
# Cancel transfer job.
# FTS_HOST must be set.
# The job Id is stored in SUBID
#
# Added by Victor Galaktionov 18/11/209
function transfer_job_cancel()
{
    echo "transfer_job_cancel()"

    echo "Check failure without SUBID argument"
    run_command_status glite-transfer-cancel -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/FileTransfer 2>message 1 || return 1

    echo "Check real cancel command" 
    run_command_status glite-transfer-cancel --verbose -s https://$FTS_HOST:8443/glite-data-transfer-fts/services/FileTransfer $SUBID 0 || return 1

    return 0
}

# Add by Victor Galaktionov 18/11/2009
#
# get SUBID and TIMEOUT and poll the status until it is done (0), failed (3)
# or failed for timeout exceeded (4). 1 ir returned on error and 2 for bad
# input parameters
#
function poll_canceled_status_with_timeout()
{
  if [ -z $1 ];then
    echo "A submission Id must be provided to poll_status_with_timeout()"
    return 2
  else
    local SUBID=$1
  fi

  if [ -z $2 ];then
    echo "A timeout must be provided to poll_status_with_timeout()"
    return 2
  else
    local Timeout=$2
  fi

  echo "Checking status for job ID: $SUBID" 

  previous_status="None"
  t=0
  while test $t -lt $Timeout
  do
    get_status_result $SUBID
    status=$?

    if [ "$status" -eq 1 ]; then
      echo "error retrieving the status with get_status_result()"
      return 1
    fi
    if [ "$status" -eq 6 ]; then
      echo "Job canceled"
      export STATUS=6
      return 0
    fi
    if [ "$status" -eq 3 ]; then
      echo "Job failed"
      export STATUS=3
      return 0
    fi
    #if [ "$status" -eq 4 ]; then
        echo "Waiting for status changes" 
        sleep 10
        t=`expr $t + 10`
        echo "Time is $t timeout is $Timeout"  
    #fi

    if [ "$t" -eq "$Timeout" ]; then
      echo "Exceded timeout of $Timeout seconds"
      export STATUS=4
      return 0
    else
      continue
    fi
  done
}


until [ -z "$1" ] 
do
  case "$1" in
     --fts)
           if [ -z "$2" ]; then
                shift 1
           else
                FTS_HOST=$2
                shift 2
           fi
     ;;
     --src)
           if [ -z "$2" ]; then
                shift 1
           else
                SE_SOURCE=$2
                shift 2
           fi
     ;;
     --dest)
           if [ -z "$2" ]; then
                shift 1
           else
                SE_DEST=$2
                shift 2
           fi
     ;;
     --bdii)
           if [ -z "$2" ]; then
                shift 1
           else
                BDII_HOST=$2
                shift 2
           fi
     ;;
     --vo)
           if [ -z "$2" ]; then
                shift 1
           else
                VO_NAME=$2
                shift 2
           fi
     ;;
     --timeout)
           if [ -z "$2" ]; then
                shift 1
           else
                TIMEOUT=$2
                shift 2
           fi
     ;;
     --gsiftp)
           if [ -z "$2" ]; then
                shift 1
           else
                GSIFTP_URL=$2
                shift 2
           fi
     ;;
     --shortinfo )
      echo $shortinfo
      exit 0
    ;;
    --longinfo )
      echo "$shortinfo: "
      echo "  - submit a file transfer"
      echo "  - cancel the job with glite-transfer-cancel"
      echo "  - test glite-transfer-cancel parameters"
      exit 0
    ;;
          *)
           showUsage
           exit 2
    ;;
  esac
done

if [ -z "$GSIFTP_URL" ]; then
    GSIFTP_URL="no"
fi

function pretest() {

run_function check_valid_proxy || return 1

if [ -z $SE_SOURCE ]; then
  echo "SE_SOURCE is not defined"
  return 1
fi
if [ -z $SE_DEST ]; then
  echo "SE_DEST is not defined"
  return 1
fi
if [ -z $VO_NAME ]; then
  VO_NAME=dteam
fi



#3.c) Retrieve SEs path to use for file transfer
echo "Retrieve SEs path to use for file transfer, $SE_SOURCE"
run_function get_se_path $SE_SOURCE || return 1

if [ "x$SE_SRM_LOC" != "x" ]; then
  SOURCE_SAPATH=$SE_SRM_LOC
  echo "SAPATH_source: $SOURCE_SAPATH"
else
  echo "Failing retrieving SAPATH from SE $SE_SOURCE"
  echo "-TEST FAILED-"
  exit 1
fi

run_function get_se_path $SE_DEST || return 1
if [ "x$SE_SRM_LOC" != "x" ]; then
  DEST_SAPATH=$SE_SRM_LOC
  echo "SAPATH_destination: $DEST_SAPATH"
else
  echo "Failing retrieving SAPATH from SE $SE_SOURCE"
  echo "-TEST FAILED-"
  exit 1
fi

#4) Submitting a file transfer
echo "Submitting a file transfer job..."
run_function file_transfer $SOURCE_SAPATH $DEST_SAPATH || return 1

echo " pretest OK"
return 0

}

function test_transfer-cancel-basic()
{
  echo "Testing basic usage of glite-transfer-cancel"

  echo "Cancel transfer job"
  run_function transfer_job_cancel || return 1

  echo "Poll for cancel result with timeout"
  echo "Retrieving the job status..."

  run_function poll_canceled_status_with_timeout $SUBID $TIMEOUT
  if [ -z $STATUS ];then
    echo "Job staus not correctly retrieved"
    return 1
  fi
  if [ "$STATUS" -eq 1 ];then
    echo "Failing retrieving the job status"
    echo "-TEST FAILED-"
    return 1
  elif [ "$STATUS" -eq 2 ];then
    echo "Failing retrieving the job status"
    echo "-TEST FAILED-"
    return 1
  elif [ "$STATUS" -eq 3 ];then
    echo "Job failed"
    echo "-TEST FAILED-"
    return 1
  elif [ "$STATUS" -eq 4 ];then
    echo "Timeout exceeded"
    echo "-TEST FAILED-"
    return 1
  elif [ "$STATUS" -eq 6 ];then
    echo "Job canceled"
    return 0
  elif [ "$STATUS" -gt 4 ];then
    echo "return code unknown $STATUS"
    return 1
  else
    echo "return code unknown $STATUS"
    return 1
  fi

}

run_function pretest || exit 1

failed=0

run_test test_transfer-cancel-basic $PASSED || failed=1

run_test test_transfer-cancel-parameters $PASSED || failed=1

if [ $failed -eq 1 ]; then
  echo ""
  echo "$shortinfo: Failed"
  exit 1
else
  echo ""
  echo "$shortinfo: Passed"
  exit 0
fi

