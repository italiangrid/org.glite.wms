#!/bin/bash

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
# AUTHORS: Gianni Pucciani, CERN
#
##############################################################################

#Script for testing FTS file transfer submission using checksum option
# 1st scenario

showUsage()
{
 echo "Usage: FTS-submission --fts <fts hostname> --channel <channel name>"
 echo "Options: "
 echo "          --fts  <fts hostname> "
 echo "                   The name of the FTS host."
 echo "          --channel  <channel name> "
 echo "                   The channel to use for submitting a file transfer"
 echo "          --bdii  <bdii hostname> "
 echo "                   The top-level BDII hostname"
 echo "          --vo  <VO name> "
 echo "                   VO name"
 echo "          --timeout  <timeout> "
 echo "                   timeout in seconds"
}

if [ $# -ne 10 ];then
  showUsage
  echo
  echo "-TEST FAILED-"
  exit 2
fi

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
     --channel)
           if [ -z "$2" ]; then
                shift 1
           else
                CHANNEL=$2
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
          *)
           showUsage
           exit 2
    ;;
  esac
done

#source functions definitions
source FTS-common
#source ../FTS-certconfig

#0.1) Check for valid proxy
ProxyExist=`voms-proxy-info 2>/dev/null | grep timeleft | wc -l`
ProxyExpired=`voms-proxy-info 2>/dev/null | grep  "timeleft  : 0:00:00" | wc -l`

if [ $ProxyExist -gt 0 -a $ProxyExpired -eq 0 ]; then
         echo ""
         echo "Using proxy from: $X509_USER_PROXY"
else
         echo ""
         echo ">>> NOTE: Valid proxy is needed for this test!"
         echo ""
         if [ $ProxyExpired -gt 0 ]; then
            echo ">>> NOTE: Proxy credential expired!"
         fi
         echo "-TEST FAILED-"
         exit 1
fi

#1) Test if channel exists

#Get Channels list
echo
echo "Retrieving channels list..."
get_channels

found=0
for Channel in $Channel_List
do
  if [ "$Channel" == "$CHANNEL" ];then 
    echo "Channel exists"
    found=1
    break
  fi
done

if [ $found -eq 0 ];then
  echo "Channel $CHANNEL does not exist"
  echo "-TEST FAILED-"
  exit 1
fi

#2) Check that the channel is active
echo
echo "Checking that $CHANNEL is active..."
is_channel_active $CHANNEL
if [ $? -eq 0 ];then
  echo "Channel $CHANNEL is Active"
else
  echo "Channel $CHANNEL is not Active"
  echo "-TEST FAILED-"
  exit 1
fi


#3) Retrieve SEs from source and destination site
echo
echo "Retrieving SEs info from source and destination sites in $CHANNEL..."

#3.a) Retrieve source and destination sites
get_source_dest_from_channel $CHANNEL
if [ $? -eq 1 ];then
  echo "Failing retrieving source and destination sites"
  echo "-TEST FAILED-"
  exit 1
fi

echo "Source: $Source_Site"
echo "Dest: $Destin_Site"

#3.b) Retrieve good SEs from source and destination sites
#get_good_se_from_site $Source_Site
get_def_se_from_site $Source_Site
if [ $? -eq 1 ];then
  echo "Failing retrieving good SE from site $Source_Site"
  echo "-TEST FAILED-"
  exit 1
fi
SE_SOURCE=$SE_HOST
echo "SE_source: $SE_SOURCE"

#get_good_se_from_site $Destin_Site
get_def_se_from_site $Destin_Site
if [ $? -eq 1 ];then
  echo "Failing retrieving good SE from site $Destin_Site"
  echo "-TEST FAILED-"
  exit 1
fi
SE_DEST=$SE_HOST
echo "SE_dest: $SE_DEST"

#3.c) Retrieve SEs path to use for file transfer
get_se_path $SE_SOURCE
if [ $? -eq 1 ];then
  echo "Failing retrieving SAPATH from SE $SE_SOURCE"
  echo "-TEST FAILED-"
  exit 1
fi
SOURCE_SAPATH=$SE_SRM_LOC
echo "SAPATH_source: $SOURCE_SAPATH"

get_se_path $SE_DEST
if [ $? -eq 1 ];then
  echo "Failing retrieving SAPATH from SE $SE_DEST"
  echo "-TEST FAILED-"
  exit 1
fi
DEST_SAPATH=$SE_SRM_LOC
echo "SAPATH_destination: $DEST_SAPATH"

#4) Submitting a file transfer using all the checksum algorithms
if [ "$CHANNEL" == "CERN-CERN" ]; then
  #USING DPM SHA1 not supported
  ALGOS="CRC32 ADLER32 MD5"
elif [ "$CHANNEL" == "CERN-DESY" ]; then
  #USING DCACHE
  ALGOS="ADLER32"
else
  echo "Don't know which checksum algorithm can be used on $CHANNEL"
  exit 1
fi

for ALGO in $ALGOS 
do
  echo 
  echo "Submitting a file transfer job using $ALGO"
  file_transfer_with_checksum $SOURCE_SAPATH $DEST_SAPATH $ALGO

  if [ $? -eq 1 ];then
    echo "Failing submitting a job"
    echo "-TEST FAILED-"
    exit 1
  fi

  #5) Poll for results with timeout
  echo "Retrieving the job status..."
  #Hardcoded timeout
  #TIMEOUT=100
  poll_status_with_timeout $SUBID $TIMEOUT
  return=$?
  
  if [ "$return" -eq 1 ];then
    echo "Failing retrieving the job status"
    echo "-TEST FAILED-"
    exit 1
  elif [ "$return" -eq 2 ];then
    echo "Failing retrieving the job status"
    echo "-TEST FAILED-"
    exit 1
  elif [ "$return" -eq 3 ];then
    echo "Job failed"
    echo "-TEST FAILED-"
    exit 1
  elif [ "$return" -eq 4 ];then
    echo "Timeout exceeded"
    echo "-TEST FAILED-"
    exit 1
  elif [ "$return" -gt 4 ];then
    echo "return code unknown $?"
    echo "-TEST FAILED-"
    exit 1
  fi

  echo "$ALGO OK!"
done

echo "-TEST PASSED-"
exit 0
