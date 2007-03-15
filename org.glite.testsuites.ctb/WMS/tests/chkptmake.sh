#!/bin/bash

echo "==================== Try to make C++ job"
make -f makeWN 2>&1
ret=$?

if [ $ret != 0 ]; then
 echo "================== Warning: can not make C++ job. hostname: `hostname` :"
 echo "make exit with exit code= $ret"
 echo "================== Run job from InputSandbox"
 chmod 755 ./chkjob1
 ./chkjob1
else
 echo "make C++ job OK"
 echo "================== Run job"
 ./chkjob
fi

exit 0

#if [ -e chkjob ]; then
# chmod 755 ./chkjob
# ./chkjob
# exit 0
#else
# echo "No file chkjob"
# exit 1
#fi

