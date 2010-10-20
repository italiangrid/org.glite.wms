#!/bin/sh

script_name=`basename $0`
failed="no"
policyfile=policyfile.txt
obligationfile=obligationfile.txt

echo "Running: ${script_name}"
echo `date`

if [ -z $PAP_HOME ]
then
    if [ -d /opt/argus/pap ]
    then
        export PAP_HOME=/opt/argus/pap
    else
        echo "${script_name}: PAP_HOME cannot be found. Exiting"
        exit 0;
    fi
fi

/etc/rc.d/init.d/pepd status > /dev/null
if [ $? -ne 0 ]; then
  echo "PEPd is not running. Starting one."
  /etc/rc.d/init.d/pepd start
  sleep 5
else
  echo "${script_name}: Stopping PEPd."
  /etc/rc.d/init.d/pepd stop > /dev/null
  sleep 5
  echo "${script_name}: Starting PEPd."
  /etc/rc.d/init.d/pepd start > /dev/null
  sleep 15
fi

/etc/rc.d/init.d/pdp status > /dev/null
if [ $? -ne 0 ]; then
  echo "PDP is not running. Starting one."
  /etc/rc.d/init.d/pdp start
  sleep 15
fi

# use a PAP to enter a policy and an obligation?

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  /etc/rc.d/init.d/pap-standalone start;
  sleep 15;
fi

# Remove all policies defined for the default pap
$PAP_HOME/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: $PAP_HOME/bin/pap-admin rap"
  exit 1
fi

# Get my cert DN for usage later
declare subj_string;
foo=`openssl x509 -in /etc/grid-security/hostcert.pem -subject -noout`;
IFS=" "
subj_string=( $foo )

RESOURCE="resource_1"
ACTION="do_not_test"
RULE="permit"

# Store initial policy
cat <<EOF > $policyfile
resource "${RESOURCE}" {
    action "${ACTION}" {
        rule ${RULE} { subject="${subj_string[1]}" }
    }
}
EOF

# $PAP_HOME/bin/pap-admin apf $policyfile
if [ $? -ne 0 ]; then
  echo "Error preparing the test environment"
  echo "Failed command: $PAP_HOME/bin/pap-admin apf $policyfile"
  exit 1
fi

# Now should add the obligation?

OPTS=" -v "
OPTS=" "

$PAP_HOME/bin/pap-admin $OPTS ap --resource resource_1 \
             --action testwerfer \
             --obligation \
http://glite.org/xacml/obligation/local-environment-map ${RULE} subject="${subj_string[1]}"

###############################################################

$PAP_HOME/bin/pap-admin lp -srai
/etc/rc.d/init.d/pdp reloadpolicy

###############################################################

export LD_LIBRARY_PATH=/opt/glite/lib64:${LD_LIBRARY_PATH}
OPTS=" -v "
OPTS=" "

/opt/glite/bin/pepcli $OPTS -p https://`hostname`:8154/authz \
       -c /etc/grid-security/hostcert.pem \
       --capath /etc/grid-security/certificates/ \
       --key /etc/grid-security/hostkey.pem \
       --cert /etc/grid-security/hostcert.pem \
       -r "resource_1" \
       -a "testwerfer" > /tmp/${script_name}.out
result=$?

if [ $result -eq 0 ]
then
    grep -q $RESOURCE /tmp/${script_name}.out;
    if [ $? -ne 0 ]
    then
        echo "${script_name}: Did not find expected resource: $RESOURCE"
        failed="yes"
    fi
    grep -qi $RULE /tmp/${script_name}.out;
    if [ $? -ne 0 ]
    then
	echo "${script_name}: Did not find expected rule: $RULE"
	failed="yes"
    fi

    declare groups;

    foo=`grep Username: /tmp/${script_name}.out`
    IFS=" "
    groups=( $foo )
    if [ ! -z ${groups[1]} ]
    then
        sleep 0;
    else
	echo "${script_name}: No user account mapped."
        failed="yes"
    fi
    foo=`grep Group: /tmp/${script_name}.out`
    IFS=" "
    groups=( $foo )
    if [ ! -z ${groups[1]} ]
    then
        sleep 0;
    else
        echo "${script_name}: No user group mapped."
        failed="yes"
    fi
    foo=`grep "Secondary Groups:" /tmp/${script_name}.out`
    IFS=" "
    groups=( $foo )
    if [ ! -z ${groups[1]} ]
    then
        sleep 0;
    else
        echo "${script_name}: No user secondary group mapped."
        failed="yes"
    fi
fi

###############################################################
#clean up

clean_up=0
# clean_up=1

if [ $failed == "yes" ]; then
  echo "---${script_name}: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---${script_name}: TEST PASSED---"
  echo `date`
  exit 0
fi

