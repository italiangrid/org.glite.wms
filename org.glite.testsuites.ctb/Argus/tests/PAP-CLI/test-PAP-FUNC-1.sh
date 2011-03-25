#!/bin/bash

#Assumptions: The PAP is running with a correct configuration file
#Note: Each single test has this assumption

if [ -z $PAP_HOME ]; then
  echo "Please set PAP_HOME variable"
  exit 1
fi

echo `date`
echo "---Test-PAP-FUNC-1---"

conffile=$PAP_HOME/conf/pap_configuration.ini
bkpfile=$PAP_HOME/conf/pap_configuration.bkp
argusconffile=$PAP_HOME/conf/pap_authorization.ini
argusbkpfile=$PAP_HOME/conf/pap_authorization.bkp
failed="no"

PAP_CTRL=argus-pap

if [ ! -f /etc/rc.d/init.d/pap-standalone ]
then
    PAP_CTRL=pap-standalone    
fi

echo "PAP_CTRL set to: /etc/rc.d/init.d/$PAP_CTRL"

/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'

if [ $? -ne 0 ]; then
  echo "PAP is not running"
  /etc/rc.d/init.d/$PAP_CTRL start; result=$?
  sleep 5;
  if [ $result -ne 0 ]
  then 
      echo "PAP is not running: A start was attempted."
      echo "Failed"
      exit 1
  else
      echo "PAP started. Proceeding."
  fi
fi

#################################################################
echo "1) testing required security section"
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

poll_interval = 14400
ordering = default

[repository]

location = $PAP_HOME/repository
consistency_check = false
consistency_check.repair = false

[standalone-service]

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

#certificate = /etc/grid-security/hostcert.pem
#private_key = /etc/grid-security/hostkey.pem

EOF
#
# What should happen here?
# The re-start of the PAP should fail as there are no
# credentials present! At least they have been commented
# out of the configuration file.
#
/etc/rc.d/init.d/$PAP_CTRL restart
sleep 10
# cat $conffile
# /etc/rc.d/init.d/$PAP_CTRL status
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  echo "The PAP is running... should NOT be running."
  failed="yes"
  mv -f $bkpfile $conffile
  echo "FAILED"
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
echo "2) testing required poll_interval "
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

#poll_interval = 14400
ordering = default

[repository]

location = $PAP_HOME/repository
consistency_check = false
consistency_check.repair = false

[standalone-service]

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

certificate = /etc/grid-security/hostcert.pem
private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null 2>&1
sleep 10
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
echo "3) testing syntax error: missing ']'"
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

poll_interval = 14400
ordering = default

[repository]

location = $PAP_HOME/repository
consistency_check = false
consistency_check.repair = false

[standalone-service

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

certificate = /etc/grid-security/hostcert.pem
private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null 2>&1
sleep 10
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
echo "4) testing syntax error: missing '='"
mv -f $conffile  $bkpfile
cat <<EOF > $conffile
[paps]
## Trusted PAPs will be listed here

[paps:properties]

poll_interval = 14400
ordering  default

[repository]

location = $PAP_HOME/repository
consistency_check = false
consistency_check.repair = false

[standalone-service]

hostname = 127.0.0.1
port = 8150
shutdown_port = 8151

[security]

certificate = /etc/grid-security/hostcert.pem
private_key = /etc/grid-security/hostkey.pem

EOF

/etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null 2>&1
sleep 10
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
echo "5) testing argus syntax error: missing ']'"
mv -f $argusconffile $argusbkpfile
cat <<EOF > $argusconffile
[dn


"/C=CH/O=CERN/OU=GD/CN=Test user 300" : ALL
"/DC=ch/DC=cern/OU=computers/CN=vtb-generic-54.cern.ch" : POLICY_READ_LOCAL|POLICY_READ_REMOTE


[fqan]

EOF

/etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null 2>&1
sleep 10
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $argusbkpfile $argusconffile
else
  mv -f $argusbkpfile $argusconffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
echo "6) testing argus syntax error: missing ':'"
mv -f $argusconffile $argusbkpfile
cat <<EOF > $argusconffile
[dn]


"/C=CH/O=CERN/OU=GD/CN=Test user 300"  ALL
"/DC=ch/DC=cern/OU=computers/CN=vtb-generic-54.cern.ch" : POLICY_READ_LOCAL|POLICY_READ_REMOTE


[fqan]

EOF

/etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null 2>&1
sleep 10
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $argusbkpfile $argusconffile
else
  mv -f $argusbkpfile $argusconffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
echo "7) testing argus syntax error: missing 'permission'"
mv -f $argusconffile $argusbkpfile
cat <<EOF > $argusconffile
[dn]


"/C=CH/O=CERN/OU=GD/CN=Test user 300"
"/DC=ch/DC=cern/OU=computers/CN=vtb-generic-54.cern.ch" : POLICY_READ_LOCAL|POLICY_READ_REMOTE


[fqan]

EOF

cat $argusconffile

/etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null 2>&1
sleep 10
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $argusbkpfile $argusconffile
else
  mv -f $argusbkpfile $argusconffile
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
  sleep 10
  echo "OK"
fi

#################################################################
#start/restart the server
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP not running'
if [ $? -eq 0 ]; then
  /etc/rc.d/init.d/$PAP_CTRL start >>/dev/null
else
  /etc/rc.d/init.d/$PAP_CTRL restart >>/dev/null
fi
sleep 10

if [ $failed == "yes" ]; then
  echo "---Test-PAP-FUNC-1: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-PAP-FUNC-1: TEST PASSED---"
  echo `date`
  exit 0
fi

