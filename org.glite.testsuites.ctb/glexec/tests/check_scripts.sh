#!/bin/bash

#This script has to be run in a wroking glexec environment, with setuid enabled



printenv="printenv.sh"

if [ ! -f $printenv ]; then
  echo "Error: printenv.sh script has to be in the current directory"
  exit 1
fi

unset MYVAR1 MYVAR2 MYVAR3 MYVAR4 MYVAR5 MYVAR6 MYVAR7 MYVAR8 MYVAR9

#TEST1
#Basic execution without exceptions
export MYVAR1=value1
export GLEXEC_ENV=`/opt/glite/sbin/glexec_wrapenv.pl`

userenvfile=`/opt/glite/sbin/glexec /opt/glite/sbin/glexec_unwrapenv.pl -- /tmp/printenv.sh`

grep MYVAR1 $userenvfile >>/dev/null
if [ $? -ne 0 ]; then
  echo "TEST1 failed: variable not correctly passed"
  exit 1
else
  echo "TEST1 OK: variable correctly passed"
fi


#TEST2
#exclude a variable from wrapping script
export MYVAR2=value2
export GLEXEC_ENV=`/opt/glite/sbin/glexec_wrapenv.pl --exclude=MYVAR2`

userenvfile=`/opt/glite/sbin/glexec /opt/glite/sbin/glexec_unwrapenv.pl -- /tmp/printenv.sh`
grep MYVAR2 $userenvfile >>/dev/null
if [ $? -ne 1 ]; then
  echo "TEST2 failed: variable not excluded from wrapping"
  exit 1
else
  echo "TEST2 OK: variable correctly excluded from wrapping"
fi

#TEST3
#exclude a variable from unwrapping script
export MYVAR3=value3
export GLEXEC_ENV=`/opt/glite/sbin/glexec_wrapenv.pl`

userenvfile=`/opt/glite/sbin/glexec /opt/glite/sbin/glexec_unwrapenv.pl --exclude=MYVAR3 /tmp/printenv.sh`
grep MYVAR3 $userenvfile >>/dev/null
if [ $? -ne 1 ]; then
  echo "TEST3 failed: variable not excluded from unwrapping"
  exit 1
else
  echo "TEST3 OK: variable correctly excluded from unwrapping"
fi

#TEST4
#mixed tests
export MYVAR4=value4
export MYVAR5=value5
export MYVAR6=value6
export MYVAR7=value7
export MYVAR8=value8
export MYVAR9=value9

export GLEXEC_ENV=`/opt/glite/sbin/glexec_wrapenv.pl --exclude=MYVAR4,MYVAR5`

userenvfile=`/opt/glite/sbin/glexec /opt/glite/sbin/glexec_unwrapenv.pl --exclude=MYVAR6,MYVAR7 /tmp/printenv.sh`
egrep 'MYVAR4|MYVAR5|MYVAR6|MYVAR7' $userenvfile >>/dev/null
if [ $? -ne 1 ]; then
  echo "TEST4 failed: found a non wanted variable"
  exit 1
fi

egrep 'MYVAR8|MYVAR9' $userenvfile
if [ $? -ne 0 ]; then
  echo "TEST4 failed: wanted variable not found"
  exit 1
fi

echo "TEST4 OK: passed only the wanted variable"

exit 0

