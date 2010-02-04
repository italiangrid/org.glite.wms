#!/bin/bash

#Assumtpions: The PDP is running with a correct configuration file
#Note: Each single test has this assumption

export PDP_HOME=/opt/argus/pdp
echo `date`
echo "---Test-PDP-Configuration---"

conffile=$PDP_HOME/conf/pdp.ini
failed="no"

#################################################################
echo "1) testing pdp status"

/etc/rc.d/init.d/pdp status | grep -q 'service: pdp'
if [ $? -eq 0 ]; then
  echo "OK"
else
  failed="yes"
  echo "Failed"
fi

if [ $failed == "yes" ]; then
  echo "---Test-PDP-Configuration: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-PDP-Configuration: TEST PASSED---"
  echo `date`
  exit 0
fi

