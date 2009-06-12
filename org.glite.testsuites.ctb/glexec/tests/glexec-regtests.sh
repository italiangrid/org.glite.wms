#!/bin/bash

# Test PRE-requisite:
#  run this script as root
#  a proxy should be available on /tmp/x509up_u0
#  config file is /opt/glite/etc/glexec.conf
#  a user certificate has to be present in /root/.globus/usercert.pem
#  glexec should be callable by root, in glexec.conf root has to be in the white list

. ./helper.sh
globresult=0

echo "Testing glexec: `date`"
echo

# Root has to run these test
whorunsme=`/usr/bin/whoami`
if [  "x${whorunsme}" = "xroot" ]; then
 echo_success; 
else
 echo_failure;globresult=1;
fi
echo "   Is this test running as 'root' ?"

# Is glexec installed ?
if [ -f /opt/glite/sbin/glexec ]; then
 echo_success; else echo_failure; globresult=1; fi
echo "   Is /opt/glite/sbin/glexec installed? "

# Are the permissions OK ?
output=`stat --format=%a /opt/glite/sbin/glexec`
if [ $output -eq 6555 ]; then
 echo_success; else echo_failure; globresult=1; fi
echo "   Is the permissions on the executble is OK (6555)? "

# Can root run -V ?
output=`/opt/glite/sbin/glexec -V 2>/dev/null`
ret=$?
if [ $ret -eq 0 ]; then
 echo_success; else echo_failure; globresult=1; fi
echo "   Can root run glexec with -V (should fail due to BUG #50646)?"

# Check the default setting glexec compiled with
defconfigfile=`/opt/glite/sbin/glexec -V  2>&1 | grep GLEXEC_CONFIG_FILE | cut -d "=" -f 2 | sed -e 's/"//g'`
output=`stat --format=%a $defconfigfile`
if [ "x$output" = "x640" ]; then
 echo_success; else echo_failure ; globresult=1;
fi
echo "   Are the permissions on the default config file OK ?"

# Check the owner of the default config file
output=`stat --format=%u $defconfigfile`
if [ "x$output" = "x0" ]; then
 echo_success; else echo_failure; globresult=1;
fi
echo "   Is the ownership of the default config file  OK ?"

# Check for strings in the binary
output=`strings /opt/glite/sbin/glexec | grep condor`
if [ "x$output" = "x" ]; then
 echo_success; else echo_failure; globresult=1;
fi
echo "   Is glexec contains dangerous RPATHs ? "

echo
echo "[  Backing up original configuration...  ]"
mv /opt/glite/etc/glexec.conf /opt/glite/etc/glexec.conf.original
echo "[  Writing custom test configuration... ]"
echo
cat <<EOF > /opt/glite/etc/glexec.conf
#
#  Glexec configuration file
#
[glexec]
silent_logging               = no
log_level                    = 5
user_white_list              = root
preserve_env_variables       = yes
linger                       = yes

lcmaps_db_file               = /opt/glite/etc/lcmaps/lcmaps-glexec.db
lcmaps_log_file              = /var/log/glexec/lcas_lcmaps.log
lcmaps_debug_level           = 5

lcas_db_file                 = /opt/glite/etc/lcas/lcas-glexec.db
lcas_log_file                = /var/log/glexec/lcas_lcmaps.log
lcas_debug_level             = 5

EOF

#
#

export GLEXEC_CLIENT_CERT=/tmp/x509up_u0
export GLEXEC_SOURCE_PROXY=/tmp/x509up_u0
export GLEXEC_TARGET_PROXY=/tmp/myfile

chmod o+rw $GLEXEC_CLIENT_CERT
rm -f ${GLEXEC_TARGET_PROXY}

/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   World writeable proxies should not be accepted.. "
chmod 400 $GLEXEC_CLIENT_CERT
#
#

chown daemon.daemon ${GLEXEC_SOURCE_PROXY}
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Source proxies owned by others should not be accepted.  "
chown root.root ${GLEXEC_SOURCE_PROXY}


#
#


chown daemon.daemon ${GLEXEC_CLIENT_CERT}
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Client certificates owned by others should not be accepted.  "
chown root.root ${GLEXEC_SOURCE_PROXY}

#
#

touch ${GLEXEC_TARGET_PROXY}
chown daemon.daemon ${GLEXEC_TARGET_PROXY}
chmod 400 ${GLEXEC_TARGET_PROXY}
echo "test" > ${GLEXEC_TARGET_PROXY}

/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
out=`cat ${GLEXEC_TARGET_PROXY}`
if [ ! "x$out" = "xtest" ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Target proxies owned by others should not be overwritten. "
rm -f ${GLEXEC_TARGET_PROXY}


#
#

chmod o+wx /etc/grid-security/certificates
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Glexec should refuse to run when CA directory is world writeable. "
chmod 755 /etc/grid-security/certificates

#
#

chmod o+w /opt/glite/etc/glexec.conf
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Glexec should refuse to run when it's default config file world writeable. "
chmod 755 /etc/grid-security/certificates
chmod o-w /opt/glite/etc/glexec.conf

#
#

mv ${GLEXEC_CLIENT_CERT} ${GLEXEC_CLIENT_CERT}.original
cp /root/.globus/usercert.pem ${GLEXEC_CLIENT_CERT}
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Glexec should not accept certificates only proxies ! "
mv ${GLEXEC_CLIENT_CERT}.original ${GLEXEC_CLIENT_CERT}


#
#

chmod o+w /opt/glite/etc/lcas/lcas-glexec.db
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ ${ret} -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Glexec should refuse to run when LCAS config file is world writeable. "
chmod o-w /opt/glite/etc/lcas/lcas-glexec.db

#
#

chmod o+w /opt/glite/etc/lcmaps/lcmaps-glexec.db
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
ret=$?
if [ $ret -eq 0 ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Glexec should refuse to run when LCMAPS config file is world writeable. "
chmod o-w /opt/glite/etc/lcmaps/lcmaps-glexec.db

#
#

unset LD_DEBUG
rm -f /tmp/glexectest
touch /tmp/glexectest
export LD_DEBUG=all
export LD_DEBUG_OUTPUT=/tmp/glexectest
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
unset LD_DEBUG
searchpath=`cat /tmp/glexectest | grep condor`
if [ ! "x${searchpath}" = "x" ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Linker should not look for libraries in condor directories. "
rm -f /tmp/glexectest

#
#

rm -f /var/log/glexec/glexec_log
export GLEXEC_ID="This is
a-new-line"
/opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
newline=`cat /var/log/glexec/glexec_log | awk '{print $1}' | grep "a-new-line"`
if [ ! "x${newline}" = "x" ]; then
 echo_failure; globresult=1; else echo_success;
fi
echo "   Glexec should not log newline characters. "


#
#

mv /opt/glite/etc/lcmaps/lcmaps-glexec.db /opt/glite/etc/lcmaps/lcmaps-glexec.db.original
cat <<EOF > /opt/glite/etc/lcmaps/lcmaps-glexec.db
#
# LCMAPS config file generated for regresson tets
#

# where to look for modules
path = /opt/glite/lib/modules

verifyproxy = "lcmaps_verify_proxy.mod"
" -certdir /etc/grid-security/certificates"
"--max-voms-ttl 2d-13:37"
"--max-proxy-level-ttl=L 1d-13:37"
"--max-proxy-level-ttl=0 7d-13:37"

good = "lcmaps_dummy_good.mod"

posix_enf = "lcmaps_posix_enf.mod"
            " -maxuid 1"
            " -maxpgid 1"
            " -maxsgid 32"

scasclient = "lcmaps_scas_client.mod"
             " -capath /etc/grid-security/certificates/"
             " -endpoint https://vtb-generic-83.cern.ch:8443"
             " -resourcetype wn"
             " -actiontype execute-now"
# policies
glexec_get_account:
verify_proxy -> scasclient
scasclient -> posix_enf

EOF
keyfile="/root/.globus/userkey.pem"
certfile="/root/.globus/usercert.pem"
proxy="proxy-novoms"
if [ ! -f $keyfile ] || [ ! -f $certfile ]; then
  echo "user certificate r key not found"
  globresult=1
fi

echo "test" | /opt/globus/bin/grid-proxy-init -cert $certfile -key $keyfile -out $proxy -valid 48:00 -pwstdin 2>/dev/null 1>/dev/null
if [ $? -ne 0 ]; then
  echo "Error creating the proxy" 
  globresult=1
else
 /opt/glite/sbin/glexec /usr/bin/whoami 2>/dev/null 1>/dev/null
 if [ $? -ne 202 ]; then
  echo_failure; globresult=1; else echo_success;
 fi
 echo "   Glexec should refuse proxieswith lifetime > 24h if told to do so. "
fi

mv /opt/glite/etc/lcmaps/lcmaps-glexec.db.original /opt/glite/etc/lcmaps/lcmaps-glexec.db

#
# Restoring
#

mv /opt/glite/etc/glexec.conf.original /opt/glite/etc/glexec.conf
echo
echo "[  Restoring original configuration... ]"
echo
if [ $globresult -eq 0 ]; then
 echo_success; else echo_failure;
fi
echo "   Overall test result:"

exit $globresult
