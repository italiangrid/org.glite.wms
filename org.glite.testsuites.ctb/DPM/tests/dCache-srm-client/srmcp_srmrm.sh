#!/bin/sh
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
# AUTHOR: Liudmila Stepanova, SINP MSU 
#
##############################################################################

showUsage ()
{
cat <<EOF
Usage:  srmcp_srmrm.sh [ options]  [--options "srmcp command line options" ] <SE_HOST>
  <SE_HOST> = specify the DPM SE host
  Options :
    -help                           displays usage
    -h                              displays usage
    --help                          displays usage
example: srmcp_srmrm.sh --options "-streams_num=1" <SE_HOST>
EOF

}

#######################
#Parsing the arguments#
#######################
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 1
fi
source DPM-certconfig
source srm-client-functions.sh
#Parse arguments
if test -z "$1"
then
 showUsage
 exit 2 
fi
if [ $# -gt 3 ];then
showUsage
 exit 2
fi
  if [ "$1" = "--options" ]; then
      shift
      check_options "$1" srmcp   
      if test -z "$2"
           then
             showUsage
             exit 2
         fi
         sehost=$2
   else
       sehost=$1
       options="--streams_num=1"
   fi
      out=(`tr "." " " <<<$sehost`)
      if [ ${#out[*]} -le 2 ];then
   echo SE_HOST=$sehost
   showUsage
             exit 2
   fi
get_VO
DT=`date +%s` 
#MD5SUM=`md5sum /proc/cpuinfo | awk '{print $1}' `
if [ "$VER" = "" ]; then
run_command lcg-cp --vo $VO  -v -D srmv2   file:////proc/cpuinfo srm://$sehost:$PORT/$SAPATH/cpuinfo.$DT
run_command srmcp  ${options[@]} -2 srm://$sehost:$PORT/$SAPATH/cpuinfo.$DT file:////tmp/cpuinfo.$DT
run_command srmrm  -2 srm://$sehost:$PORT/$SAPATH/cpuinfo.$DT
rm -f /tmp/cpuinfo.$DT
else
run_command srmcp -version -2  srm://$sehost:$PORT/$SAPATH
fi
myexit 0


