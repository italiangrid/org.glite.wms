#!/bin/sh

failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-PAP-Ping---"
###############################################################
echo "1) test PAP ping"
$PAP_HOME/bin/pap-admin ping
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-PAP-Ping: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-PAP-Ping: TEST PASSED---"
  echo `date`
  exit 0
fi

