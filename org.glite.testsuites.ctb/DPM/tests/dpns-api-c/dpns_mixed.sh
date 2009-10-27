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

. ./helper.sh

node=$CFG_DPM_HOST
vo=$CFG_VO

echo "Target node is $node. `date`"
echo "Launching DPNS C API tests:"
echo "Compiling dpns_mixed.c"
rm -f dpns_mixed
gcc -I /opt/lcg/include/dpm/ -L/opt/lcg/lib/ -ldpm -lgfal dpns_mixed.c -o dpns_mixed

if [ $? -eq 0 ] && [ -f dpns_mixed ]; then
   echo_success
else
   echo_failure
   exit 1
fi
echo "  Compilation of dpns_mixed.c program:"
echo

mydomain=`echo $node | cut -d "." -f 2-`
mypath=/dpm/$CFG_DPNS_DOMAIN/$CFG_DPNS_BASEDIR/$CFG_VO/dpnscapitest$RANDOM

export DPM_HOST=$node
export DPNS_HOST=$node

./dpns_mixed $mypath

