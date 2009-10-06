#!/bin/sh
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

showUsage ()
{
 echo "                                           "
 echo "Usage:  UI-certtest.sh [-f <conf.file>]    "
 echo "  <conf.file> Configuration file, default is UI-certconfig"
 echo "                                           "
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
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ] || [ $# -gt 2 ]; then
  showUsage
  exit 2
fi

if [ "$1" = "-f" ]; then
  conffile=$2
else
  conffile="./UI-certconfig"
fi

###################################
# Check for environment variables #
###################################

if [ -e $conffile ]; then
  echo "Using $conffile"
  source $conffile
else
  echo "Config File $conffile does not exist"
  exitFailure
fi

#if [ -z "$PAP_HOME" ]; then
#  echo "You need to set PAP_HOME in order to run this script"
#  exitFailure
#fi

#########
# START #
#########

echo "START `date` "
echo "------------------------------------------------"

#get pass from the user
echo "Enter the test user pass:"
read -s PASS
export PASS

####################################
# Create a directory for log files #
####################################

id=`date +%y%m%d%H%M%S`
if [ -z "$LOGSLOCATION" ]; then
  cp=`pwd`
  loglocation=$cp/logs_$id
  mkdir -p $loglocation
else
  loglocation=$LOGSLOCATION/logs_$id
  mkdir -p $loglocation
fi

if [ ! -d $loglocation ];then
  echo   "Error while creating log directory $loglocation"
  exitFailure
else
  echo "Log files will be stored in $loglocation"
fi

########################
# Launch all the tests #
########################

declare -a tests_failed
failed=no

#######
# SL4 #
#######

if [ "x${SL4}" = "xyes" ]; then

  echo "*Running SL4 tests"
  pushd ./tests/gl31-SLC4-i386 >> /dev/null
  declare -a tests_list
#  tests_list=("${tests_list[@]}" "UI-inf-rgma-client.sh")
#  tests_list=("${tests_list[@]}" "UI-security-voms-proxy-init-userconf.sh")
#  tests_list=("${tests_list[@]}" "UI-workload-edg-submit-wait-output.sh") #look at the test!
#  tests_list=("${tests_list[@]}" "UI-workload-edg-job-list-match.sh")
  tests_list=("${tests_list[@]}" "UI-libraries-exist.sh")
  tests_list=("${tests_list[@]}" "UI-security-myproxy-init-info-destroy.sh")
  tests_list=("${tests_list[@]}" "UI-security-voms-proxy-init-info-destroy.sh")
 
  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt
    ./$item  > $loglocation/${item}_result.txt 2>&1
    if [ $? -ne 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED" 
    fi
  done
  popd >> /dev/null
else
  echo "*SL4 tests skipped"
fi

#######
# SL5 #
#######
unset tests_list

if [ "x${SL5}" = "xyes" ]; then

  echo "*Running SL5 tests"
  pushd ./tests/gl32-SLC5-x86_64 >> /dev/null
  declare -a tests_list
  tests_list=("${tests_list[@]}" "UI-libraries-exist.sh")
  tests_list=("${tests_list[@]}" "UI-security-myproxy-init-info-destroy.sh")
  tests_list=("${tests_list[@]}" "UI-security-voms-proxy-init-info-destroy.sh")
#  tests_list=("${tests_list[@]}" "UI-security-voms-proxy-init-userconf.sh")

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt
    ./$item  > $loglocation/${item}_result.txt 2>&1
    if [ $? -ne 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED" 
    fi
  done
  popd >> /dev/null
else
  echo "*SL5 tests skipped"
fi

##########
# COMMON #
##########
unset tests_list

if [ "x${COMMON}" = "xyes" ]; then

  echo "*Running COMMON tests"
  pushd ./tests/common >> /dev/null
  declare -a tests_list
#  tests_list=("${tests_list[@]}" "command-exist.sh")
#  tests_list=("${tests_list[@]}" "command-help.sh")
#  tests_list=("${tests_list[@]}" "command-version.sh")
#  tests_list=("${tests_list[@]}" "lcg-tests-common.sh")
#  tests_list=("${tests_list[@]}" "lfc-tests-common.sh")
#  tests_list=("${tests_list[@]}" "test-lcg-utils.sh")
##  tests_list=("${tests_list[@]}" "UI-commands-exist-all.sh") TODO
#  tests_list=("${tests_list[@]}" "UI-data-lcg-alias.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lcg-cp.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lcg-cr.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lcg-errors.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lcg-list.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lfc-acl.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lfc-comment.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lfc-errors.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lfc-ln.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lfc-ls.sh")
#  tests_list=("${tests_list[@]}" "UI-data-lfc-mkdir.sh")
  tests_list=("${tests_list[@]}" "UI-environment.csh")
  tests_list=("${tests_list[@]}" "UI-environment.sh")
  tests_list=("${tests_list[@]}" "UI-exec-help-glite-delegation.sh")
  tests_list=("${tests_list[@]}" "UI-exec-help-glite-sd-query.sh")
  tests_list=("${tests_list[@]}" "UI-exec-help-lcg-info.sh")
  tests_list=("${tests_list[@]}" "UI-exec-help-lcgtags.sh") #bug 56603 #Expected failure
  tests_list=("${tests_list[@]}" "UI-exec-help-managevotag.sh") #bug 53516 #Expected failures on SL5
  tests_list=("${tests_list[@]}" "UI-exec-version-glite-transfer.sh")
  tests_list=("${tests_list[@]}" "UI-exec-version-glite-wms-job.sh")
  tests_list=("${tests_list[@]}" "UI-exec-version-grid-proxy.sh")
  tests_list=("${tests_list[@]}" "UI-exec-version-lcg.sh")
  tests_list=("${tests_list[@]}" "UI-exec-version-myproxy.sh")
  tests_list=("${tests_list[@]}" "UI-exec-version-uberftp.sh")
  tests_list=("${tests_list[@]}" "UI-exec-version-voms-proxy.sh")
  tests_list=("${tests_list[@]}" "UI-glite-environment.csh")
  tests_list=("${tests_list[@]}" "UI-glite-environment.sh")
  tests_list=("${tests_list[@]}" "UI-inf-lcg-info-ce.sh")
  tests_list=("${tests_list[@]}" "UI-inf-lcg-info-se.sh")
  tests_list=("${tests_list[@]}" "UI-inf-lcg-infosites.sh") #bug 53411 #Expected failures
  tests_list=("${tests_list[@]}" "UI-inf-ldapsearch.sh")
##  tests_list=("${tests_list[@]}" "UI-manpage-exist-all.sh") TODO
  tests_list=("${tests_list[@]}" "UI-ntp-is-running.sh") #bug? ntp is stopped WARNING
  tests_list=("${tests_list[@]}" "UI-security-grid-cert-info.sh")
  tests_list=("${tests_list[@]}" "UI-security-grid-proxy-info.sh")
  tests_list=("${tests_list[@]}" "UI-security-grid-proxy-init-info-destroy.sh")
  tests_list=("${tests_list[@]}" "UI-security-myproxy-change-pass-phrase.sh")
  tests_list=("${tests_list[@]}" "UI-security-voms-proxy-info.sh") #bug 49614 #Expected failures
  tests_list=("${tests_list[@]}" "UI-tags-lcgtags.sh")
  tests_list=("${tests_list[@]}" "UI-tags-managevotag.sh") #bug 53516 #Expected failures on SL5
  tests_list=("${tests_list[@]}" "UI-workload-glite-submit-wait-output.sh")
  tests_list=("${tests_list[@]}" "UI-workload-glite-wms-deleg-submit-wait-output.sh")
  tests_list=("${tests_list[@]}" "UI-workload-glite-wms-errors.sh")
  tests_list=("${tests_list[@]}" "UI-workload-glite-wms-job-list-match.sh")
  tests_list=("${tests_list[@]}" "UI-workload-glite-wms-submit-wait-output.sh")

  for item in ${tests_list[*]}
  do
    rm -rf ${item}_result.txt
    ./$item  > $loglocation/${item}_result.txt 2>&1
    if [ $? -ne 0 ]; then
      echo "$item FAILED"
      failed=yes
      tests_failed=( "${tests_failed[@]}" "$item" )
    else
      echo "$item PASSED" 
    fi
  done
  popd >> /dev/null
else
  echo "*COMMON tests skipped"
fi

#########################
# Analyse tests outcome #
#########################

if [ $failed = "yes" ]; then

  echo "TEST_FAILED"
  failures=${#tests_failed[*]}
  echo "$failures tests failed:"
  for item in ${tests_failed[*]}
  do
    echo "$item: results in $loglocation/${item}_result.txt"
  done
  exit 1
else
    echo "TEST_PASSED"
  exit 0
fi

