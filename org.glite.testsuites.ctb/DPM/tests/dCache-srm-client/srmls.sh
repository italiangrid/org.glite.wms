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
# AUTHORS: Gianni Pucciani, CERN
#
##############################################################################

usage ()
{
cat <<EOF
Usage:  srmls.sh [--dpm <DPM HOST>] 
  <DPM HOST> = specify a DPM SE, defaults to the CTB one
EOF

}

exitFailure ()
{
echo "------------------------------------------------"
echo "END `date`"
echo "-TEST FAILED-"
exit -1
}

#######################
#Parsing the arguments#
#######################
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 2
fi

#Parse arguments
while [ $# -ne 0 ]; do
  case "$1" in
    --dpm)
      shift
      DPM_HOST_ARG=$1
      shift
      ;;
    *|'')
      echo "Unknown option '$1'"
      exit
      ;;
  esac
done

if [ -n "$DPM_HOST_ARG" ]; then
  sehost=$DPM_HOST_ARG
  echo "DPM HOST: $DPM_HOST_ARG"
elif [ -n "$DPM_HOST" ]; then
  sehost=$DPM_HOST
  echo "DPM HOST: $DPM_HOST"
else
  echo "no DPM host selected"
  exit 2
fi

source lcg-tests-functions.sh
SRM2PORT=8446
DPNS_BASEDIR=/dpm/cern.ch/home
run_command srmls -2 srm://$sehost:${SRM2PORT}${DPNS_BASEDIR}

myexit 0


