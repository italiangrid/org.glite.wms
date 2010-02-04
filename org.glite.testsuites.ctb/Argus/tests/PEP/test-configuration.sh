#!/bin/bash

#Assumtpions: The PEP is running with a correct configuration file
#Note: Each single test has this assumption

export PEP_HOME=/opt/argus/pepd
echo `date`
echo "---Test-PEP-Configuration---"

conffile=$PEP_HOME/conf/pepd.ini
bkpconffile=$PEP_HOME/conf/pepd.bkp
failed="no"

#################################################################
echo "1) testing pep status"

/etc/rc.d/init.d/pepd status | grep -q 'service: pep'
if [ $? -eq 0 ]; then
  echo "OK"
else
  failed="yes"
  echo "Failed"
fi

#################################################################
#echo "2) testing pep with SSL"
#/etc/rc.d/init.d/pep stop
#sleep 5
#mv $conffile $bkpconffile
#Insert SSL option
#sed '/SERVICE/a\enableSSL = true' $bkpconffile > $conffile

#/etc/rc.d/init.d/pep start
#if [ $? -eq 0 ]; then
#  echo "OK"
#else
#  failed="yes"
#  echo "Failed"
#fi

#################################################################
#echo "2) testing pep with no config file"
#/etc/rc.d/init.d/pep stop
#mv $conffile $bkpconffile

#/etc/rc.d/init.d/pep start
#if [ $? -ne 0 ]; then
#  echo "OK"
#else
#  failed="yes"
#  echo "Failed"
#fi



#################################################################
#/etc/rc.d/init.d/pep stop >>/dev/null
#mv $bkpconffile $conffile
#/etc/rc.d/init.d/pep start >>/dev/null

if [ $failed == "yes" ]; then
  echo "---Test-PEP-Configuration: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-PEP-Configuration: TEST PASSED---"
  echo `date`
  exit 0
fi

