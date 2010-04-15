#!/bin/bash
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
# AUTHOR Liudmila Stepanova, SINP MSU
#
##############################################################################
showUsage ()
{
cat <<EOF
Usage: check_commands [--options]  
  Options :
    -help                           displays usage
    -h                              displays usage
    --help                          displays usage
EOF
}
if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 1
fi
ERR=0
. myproxy-functions.sh
which myproxy-store 2>/dev/null
if [ $? -ne 0 ]
then
   myecho "myproxy-store not in PATH"
ERR=1
fi
which myproxy-init 2>/dev/null
if [ $? -ne 0 ]
then 
   myecho "myproxy-init not in PATH"
ERR=1
fi
which myproxy-retrieve 2>/dev/null
if [ $? -ne 0 ]
then 
   myecho "myproxy-retrieve not in PATH"
ERR=1
fi
which myproxy-destroy 2>/dev/null
if [ $? -ne 0 ]
then
  myecho "myproxy-destroy not in PATH"
ERR=1
fi
which myproxy-logon 2>/dev/null
if [ $? -ne 0 ]
then
   myecho "myproxy-logon not in PATH"
ERR=1
fi
which myproxy-change-pass-phrase 2>/dev/null
if [ $? -ne 0 ]
then
   myecho  "myproxy-change-pass-phrase not in PATH"
   ERR=1
fi
if [ "$ERR" = "1" ]; then
  myecho "end time: " `date`
  echo "*** -TEST FAILED- ***"
  exit 2
fi
  myecho "end time: " `date`
  echo "=== test PASSED ==="
exit 0 
