#!/bin/bash

#Assumtpions: The PAP is running with a correct configuration file
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

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
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

location = /opt/argus/pap/repository
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

location = /opt/argus/pap/repository
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

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
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

location = /opt/argus/pap/repository
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

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
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

location = /opt/argus/pap/repository
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

/etc/rc.d/init.d/pap-standalone restart >>/dev/null
sleep 10
/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -eq 0 ]; then
  failed="yes"
  echo "FAILED"
  mv -f $bkpfile $conffile
else
  mv -f $bkpfile $conffile
  /etc/rc.d/init.d/pap-standalone start >>/dev/null
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
  sleep 10
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
  echo "---Test-PAP-FUNC-1: TEST FAILED---"
  echo `date`
  exit 1
else
  echo "---Test-PAP-FUNC-1: TEST PASSED---"
  echo `date`
  exit 0
fi

