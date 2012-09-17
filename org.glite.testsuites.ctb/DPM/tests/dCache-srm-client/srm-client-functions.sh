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
# AUTHOR Liudmila Stepanova, SINP MSU
#
##############################################################################

LCG_GFAL_INFOSYS_default="lcg-bdii.cern.ch:2170"

function myecho() {
  echo " ===> $@"
}

# ... Run command succussfuly or exit with cleanup
function myexit() {
  echo ""
  myecho "end time: " `date`
  echo ""

  if [ $? -ne 0 ] || [ "$1" != "0" ]; then

    echo " *** -TEST FAILED- *** "
    if [ "$1" != "0" ]; then
      echo " *** failed command: $2 *** "
    fi
    exit 2
  fi

  echo "    === test PASSED === "
  exit 0
}

function run_command() {
  echo ""
  echo " $" $@

  OUTPUT=$(eval $@ 2>&1)
  if [ $? -gt 0 ]; then
   echo "${OUTPUT}"
    myecho "$1 failed"
    echo ""
    myexit 2 $1
  fi

  echo "${OUTPUT}"

  return 0
}
function get_VO() {
timeleft=`voms-proxy-info -timeleft`
if [ "$?" != 0 ];then
echo " *** -TEST FAILED- *** "
exit 2
fi
if [ "$timeleft" -lt 60 ]
then
   echo
   myecho "failed: proxy is expired "
   echo " *** -TEST FAILED- *** "
   exit 2
fi
VO=`voms-proxy-info -vo`
return 0
}

function get_srm_URL {
SEx_HOST=$1   
    if [ -z "$SEx_HOST" ]; then
      myecho "failed: SE_HOST not defined"
      echo " *** -TEST FAILED- *** "  
      exit 2
    fi
if [ "$LCG_GFAL_INFOSYS" = "" ]; then
   LCG_GFAL_INFOSYS=$LCG_GFAL_INFOSYS_default
fi
get_VO
OUTPUT=`ldapsearch -z none -x -H ldap://$LCG_GFAL_INFOSYS -b "mds-vo-name=local,o=grid" "(&(GlueChunkKey=GlueSEUniqueID=${SEx_HOST})(|(GlueSAAccessControlBaseRule=$VO)(GlueSAAccessControlBaseRule=VO:$VO)))"|grep GlueSAAccessControlBaseRule 2>&1`
OutputVO=`awk -F"GlueSAAccessControlBaseRule:" ' {print $2} ' <<< $OUTPUT`
if [ "$OutputVO" = "" ]; then
myecho "$SEx_HOST VO:$VO is not suppotted"
myexit 1 "ldapsearch $OUTPUT" 
fi
OUTPUT=`ldapsearch  -x -LLL -H ldap://$LCG_GFAL_INFOSYS -b "mds-vo-name=local, o=grid" "(&(objectclass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID=$SEx_HOST))" | grep GlueVOInfoPath | grep $VO 2>&1`
URL=(`awk -F: '{print $2}' <<<"$OUTPUT"`)
 if [ "$URL" = "" ]; then
    OUTPUT=`ldapsearch  -x -LLL -H ldap://$LCG_GFAL_INFOSYS -b "mds-vo-name=local, o=grid" "(&(objectclass=GlueSA)(GlueChunkKey=GlueSEUniqueID=$SEx_HOST))" | grep GlueSAPath | grep $VO 2>&1`
URL=(`awk -F: '{print $2}' <<<"$OUTPUT"`)
 fi
OK="0"
for URLX in ${URL[@]}
do
#echo $URLX
ARR=(`echo $URLX |tr "/" " "`)
for i in ${ARR[@]}
do
if [ "$i" = "$VO" ]; then
OK="1"
fi
done
if [ "$OK" = "1" ]; then
break
fi
done 
  URL=(`awk -F/ '{print $2" "$3}'<<<  $URLX`)

if [ "${URL[0]}" = "dpm" ];then
PORT="8446"
else
if [ "${URL[0]}" = "pnfs" ];then
PORT="8443"
else
myexit 1 "SE does not support DPM or dCache for $VO"
fi
fi
echo SE=$SEx_HOST URL=$URLX PORT=$PORT 
DT=`date +%s`     
 return 0
}
function check_options {
options=($1)
OPTIONS=(-debug -srmcphome -help -h  -gsissl -mapfile -webservice_path  -webservice_protocol -use_proxy -x509_user_proxy -x509_user_cert -x509_user_trusted_certificates -globus_tcp_port_range -gss_expected_name -conf   -save_conf -wsdl_url -retry_timeout -retry_num -connect_to_wsdl -delegate -full_delegation -version -srm_protocol_version  -1  -2)
SRMCP=(-protocols -pushmode buffer_size -tcp_buffer_size -streams_num -array_of_client_networks -retention_policy -access_latency -access_pattern -connection_type -space_token -copyjobfile -use_urlcopy_script-report -server_mode -storagetype -request_lifetime  -priority -overwrite_mode -send_cksm -cksm_type -cksm_value)
SRMLS=(-l -recursion_depth -offset  -count)
BRING=(-protocols -array_of_client_networks -retention_policy -access_latency -access_pattern -lifetime -space_token -report -storagetype -request_lifetime -priority )
SRM_GET_REQUEST_STATUS=(-request_id)
SRM_GET_REQUECT_TOKENS=(-request_desc)
case "$2" in
"srmcp" )                  OPTIONS=(${OPTIONS[@]} ${SRMCP[@]});;
"srmls" )                  OPTIONS=(${OPTIONS[@]} ${SRMLS[@]});;
"srm-get-request-status ") OPTIONS=(${OPTIONS[@]} ${SRM_GET_REQUEST_STATUS[@]});;
"srm-get-request-tokens" ) OPTIONS=(${OPTIONS[@]} ${SRM_GET_REQUECT_TOKENS[@]});;
"srm-bring-online" )       OPTIONS=(${OPTIONS[@]} ${BRING[@]});;      
 esac
i=0
for a in ${options[@]}
do
OK=0
VER=""
arg=`awk -F= '{print $1}' <<< $a`
if [ "$arg" = "-version" ];then
VER=1
fi
for opt in  ${OPTIONS[@]}
     do
if [ "$arg" = "$opt" ];then
OK=1
fi
done
#echo $arg
if [ "$OK" = "0" ];then
echo  Option $a is not correct
options[$i]=""
fi

let i=$i+1
done
 return 0
}
