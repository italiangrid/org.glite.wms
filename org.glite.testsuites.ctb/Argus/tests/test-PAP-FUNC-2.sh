#!/bin/bash

#Assumtpions: The PAP is running with a correct configuration file
#Note: Each single test has this assumption

echo `date`
echo "---Test-PAP-FUNC-2---"

conffile=$PAP_HOME/conf/pap_configuration.ini
bkpfile=$PAP_HOME/conf/pap_configuration.bkp
argusconffile=$PAP_HOME/conf/pap_authorization.ini
argusbkpfile=$PAP_HOME/conf/pap_authorization.bkp
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

#################################################################
echo "1) testing missing configuration file"

mv $conffile $bkpfile
/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  mv -f $bkpfile $conffile
  echo "FAILED"
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
  sleep 40
  echo "OK"
fi

#################################################################
echo "2) testing missing argus file"
mv -f $argusconffile  $argusbkpfile

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $argusbkpfile $argusconffile
else
  mv -f $argusbkpfile $argusconffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
  sleep 40
  echo "OK"
fi

#################################################################
#start/restart the server
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP not running'
if [ $? -eq 0 ]; then
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
else
  /etc/rc.d/init.d/pap-standalone restart >>/dev/null
fi
sleep 10

if [ $failed == "yes" ]; then
  echo "---Test-PAP-FUNC-2: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-PAP-FUNC-2: TEST PASSED---"
  echo `date`
  exit 0
fi

