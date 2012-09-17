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
# AUTHORS: Andreas Unterkircher, CERN
#
##############################################################################

usage() {
 echo
 echo "This script creates a tarball WNtests.tgz and a jdl file WNtest.jdl. Submitting this jdl sends WNtests.tgz to some WN and executes the tests contained therein. The output of the job contains the test results. WNtests.tgz contains the following UI tests: UI--data-lcg-*, UI-data-lfc-*, UI-inf-lcg-*. To use this script you also have to check out the UI tests."
 echo "This script accepts the following options (with the same meaning as for the UI-test-driver.sh script):  --sehost|-sehost <SE hostname> --lfchost|-lfchost <LFC hostname> --lfcdir|-lfcdir <LFC directory> --vo|-vo <VO name>"
 echo "Have a look at WNtest.jdl and adapt it to your needs."
 echo
}

while [ $# -gt 0 ]
do
 case $1 in
 --sehost | -sehost ) sehost=$2
  shift
  ;;
 --lfchost | -lfchost ) lfchost=$2
  shift
  ;;
 --lfcdir | -lfcdir ) lfcdir=$2
  shift
  ;;
 --vo | -vo ) voname=$2
  shift
  ;;
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done


workdir=`pwd`

if [ ! -d ../UI/tests ]; then
 echo "Also check out org.glite.testsuites.ctb/UI/tests"
 exit 1
fi

cd ${workdir}/../UI/tests
tar cvfz ${workdir}/WNtests.tgz UI-data-lcg-* UI-data-lfc-* UI-inf-lcg-info* lcg-tests-common.sh lfc-tests-common.sh UI-test-driver.sh
cd $workdir

cat > WNtest.jdl << EOF
Executable = "/bin/sh";
Arguments = "-c 'tar xzf WNtests.tgz ; ./UI-test-driver.sh ${sehost:+--sehost $sehost} ${lfchost:+--lfchost $lfchost} ${lfcdir:+--lfcdir $lfcdir} ${voname:+--vo $voname} UI-data-lcg UI-data-lfc UI-inf-lcg 2>&1'";
StdOutput = "WNtests.out";
StdError = "WNtests.out";
InputSandbox = {"WNtests.tgz"};
OutputSandbox = {"WNtests.out"};
EOF

