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
Usage:  srm_space.sh [ options]  [--options "srm-get-space-tokens  command line options" ]  <SE_HOST>
  <SE_HOST> = specify the DPM SE or dCache SE
  Options :
    -help                           displays usage
    -h                              displays usage
    --help                          displays usage
examples: srm_space.sh --options "-debug" <SE_HOST>
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
        check_options "$1" srm-get-request-tokens 
        if test -z "$2"
           then
             showUsage
             exit 2
         fi
         sehost=$2
   else
       sehost=$1
       options=""
   fi
   out=(`tr "." " " <<<$sehost`)
   if [ ${#out[*]} -le 2 ];then
   echo SE_HOST=$sehost
   showUsage
             exit 2
   fi
get_VO
if [ "$VER" = "" ]; then
RESERV=`run_command srm-reserve-space  -desired_size=2000 -2 -retention_policy=REPLICA -access_latency=ONLINE -guaranteed_size=1024 -lifetime=3600 srm://$sehost:$PORT/$SAPATH/`
TOKEN=`awk -F token  '{ print $2}' <<< $RESERV  |sed s/=//g |sed s/" "//g`
run_command  srm-get-space-tokens ${options[@]} -2 srm://$sehost:$PORT/$SAPATH/
run_command srm-get-space-metadata ${options[@]} -2 -space_tokens=$TOKEN srm://$sehost:$PORT/$SAPATH/
run_command srm-release-space  -space_token=$TOKEN -2 srm://$sehost:$PORT/$SAPATH/
else
run_command srm-reserve-space -version srm://$sehost:$PORT/$SAPATH/
run_command srm-get-space-tokens -version srm://$sehost:$PORT/$SAPATH/
run_command srm-get-space-metadata -version srm://$sehost:$PORT/$SAPATH/
run_command srm-release-space -version srm://$sehost:$PORT/$SAPATH/ 
fi 
myexit 0
