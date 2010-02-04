#!/bin/sh

failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-Update-PAP---"
###############################################################
echo "1) testing upap with non existing alias"
$PAP_HOME/bin/pap-admin upap Dummy
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-Update-PAP: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Update-PAP: TEST PASSED---"
  echo `date`
  exit 0
fi

