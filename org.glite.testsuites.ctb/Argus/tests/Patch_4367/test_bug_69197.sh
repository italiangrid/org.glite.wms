#!/bin/sh

script_name=`basename $0`
failed="no"
policyfile=policyfile.txt
obligationfile=obligationfile.txt

## This is the needed bit to make EGEE/EMI compatible tests
if [ -z $PAP_HOME ]
then
    if [ -d /usr/share/argus/pap ]
    then
        PAP_HOME=/usr/share/argus/pap
    else
        if [ -d /opt/argus/pap ]
        then
            PAP_HOME=/opt/argus/pap
        else
            echo "PAP_HOME not set, not found at standard locations. Exiting."
            exit 2;
        fi
    fi
fi
PEP_CTRL=argus-pepd
if [ -f /etc/rc.d/init.d/pep ];then;PEP_CTRL=pep;fi
echo "PEP_CTRL set to: /etc/rc.d/init.d/$PEP_CTRL"
PDP_CTRL=argus-pdp
if [ -f /etc/rc.d/init.d/pdp ];then;PDP_CTRL=pdp;fi
echo "PDP_CTRL set to: /etc/rc.d/init.d/$PDP_CTRL"
PAP_CTRL=argus-pap
if [ -f /etc/rc.d/init.d/pap-standalone ];then
    PAP_CTRL=pap-standalone
fi
echo "PAP_CTRL set to: /etc/rc.d/init.d/$PAP_CTRL"
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  /etc/rc.d/init.d/$PAP_CTRL start
  sleep 10
fi

pep_config="/etc/argus/pepd/conf/pepd.ini"
if [ -f /opt/argus/pepd/conf/pepd.ini ]
then
    pep_config=/opt/argus/pepd/conf/pepd.ini
fi
pep_config_saved="/tmp/pepd.ini.saved"

PEPCLI=pepcli
if [ -f $PEPCLI ]
then
    PEPCLI=$PEPCLI
fi

## To here for EGEE/EMI compatible tests

USERCERT=/etc/grid-security/hostcert.pem
USERKEY=/etc/grid-security/hostkey.pem
# USERCERT=$HOME/user_certificates/test_user_0_cert.pem
# USERKEY=$HOME/user_certificates/test_user_0_key.pem

X509_USER_CERT=$USERCERT
X509_USER_KEY=$USERKEY

echo "Running: ${script_name}"
echo `date`

# Get my cert DN for usage later

declare subj_string;
foo=`openssl x509 -in $USERCERT -subject -noout`;
IFS=" "
subj_string=( $foo )

grep ${subj_string[1]} /etc/grid-security/grid-mapfile
#
# Here should check that the
# /etc/grid-security/voms-grid-mapfile
#                    grid-mapfile
#                    groupmapfile
#
# Make sure that all references to the credential DN are not present
# in the above files
#
xxx_tmp=${subj_string[1]}; echo ${xxx_tmp}
searchstring=${xxx_tmp//\//\\\/} ;echo ${searchstring}; # echo ${searchstring}
target_file=/etc/grid-security/voms-grid-mapfile
sed -i 's/'${searchstring}'/# Ignore/g' ${target_file}

target_file=/etc/grid-security/grid-mapfile
sed -i 's/'${searchstring}'/# Ignore/g' ${target_file}

target_file=/etc/grid-security/groupmapfile
sed -i 's/'${searchstring}'/# Ignore/g' ${target_file}

# So now there aree no DNs present in any of those files.

# Next remove all the "leases" from the /etc/grid-security/gridmapdir/

rm /etc/grid-security/gridmapdir/*`hostname`*

# Now probably should start the services and test whether I can get an account.

# exit 0

function pep_start {
/etc/rc.d/init.d/$PEP_CTRL status > /dev/null
if [ $? -ne 0 ]; then
  echo "PEPd is not running. Starting one."
  /etc/rc.d/init.d/$PEP_CTRL start
  sleep 10
else
  echo "${script_name}: Stopping PEPd."
  /etc/rc.d/init.d/$PEP_CTRL stop > /dev/null
  sleep 6
  echo "${script_name}: Starting PEPd."
  /etc/rc.d/init.d/$PEP_CTRL start > /dev/null
  sleep 10
fi
}

pep_start

function pdp_start {
/etc/rc.d/init.d/$PDP_CTRL status > /dev/null
if [ $? -ne 0 ]; then
  echo "PDP is not running. Starting one."
  /etc/rc.d/init.d/$PDP_CTRL start
  sleep 10
fi
}

pdp_start

# use a PAP to enter a policy and an obligation?

function pap_start {
/etc/rc.d/init.d/$PAP_CTRL status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  /etc/rc.d/init.d/$PAP_CTRL start;
  sleep 10;
fi 
}

pap_start

# Remove all policies defined for the default pap
$PAP_HOME/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: $PAP_HOME/bin/pap-admin rap"
  exit 1
fi

RESOURCE="resource_1"
ACTION="do_not_test"
RULE="permit"

# Now should add the obligation?

OPTS=" -v "
OPTS=" "

$PAP_HOME/bin/pap-admin $OPTS ap --resource resource_1 \
             --action testwerfer \
             --obligation \
http://glite.org/xacml/obligation/local-environment-map ${RULE} subject="${subj_string[1]}"

###############################################################

# $PAP_HOME/bin/pap-admin lp -srai
/etc/rc.d/init.d/$PDP_CTRL reloadpolicy

###############################################################

export LD_LIBRARY_PATH=/opt/glite/lib64
OPTS=" -v "
OPTS=" "

$PEPCLI $OPTS -p https://`hostname`:8154/authz \
       -c $USERCERT \
       --capath /etc/grid-security/certificates/ \
       --key $USERKEY \
       --cert $USERCERT \
       -r "resource_1" \
       -a "testwerfer" > /tmp/${script_name}.out
result=$?; echo $result
#
# At this stage, we should not get a user account mapping
# as the mapfiles do not contain a DN and the leases are deleted
#
if [ $result -eq 0 ]
then
    grep -q $RESOURCE /tmp/${script_name}.out;
    if [ $? -ne 0 ]
    then
        echo "${script_name}: Did not find expected resource: $RESOURCE"
        # failed="yes"
    fi
    grep -qi $RULE /tmp/${script_name}.out;
    if [ $? -ne 0 ]
    then
	echo "${script_name}: Did not find expected rule: $RULE"
	# failed="yes"
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
        # failed="yes"
    fi
    foo=`grep Group: /tmp/${script_name}.out`
    IFS=" "
    groups=( $foo )
    if [ ! -z ${groups[1]} ]
    then
        sleep 0;
    else
        echo "${script_name}: No user group mapped."
        # failed="yes"
    fi
    foo=`grep "Secondary Groups:" /tmp/${script_name}.out`
    IFS=" "
    groups=( $foo )
    if [ ! -z ${groups[1]} ]
    then
        sleep 0;
    else
        echo "${script_name}: No user secondary group mapped."
        # failed="yes"
    fi
fi

#
# Now should modify the mapfiles to have a DN present.
# 
target_file=/etc/grid-security/voms-grid-mapfile
grep -q '# Ignore' ${target_file}
if [ $? -ne 0 ]
then
    echo \"${xxx_tmp}\" .dteam >>${target_file}
else
    sed -i 's/# Ignore/'${searchstring}'/g' ${target_file};# echo $?
fi

target_file=/etc/grid-security/grid-mapfile
grep -q '# Ignore' ${target_file}
if [ $? -ne 0 ]
then
    echo \"${xxx_tmp}\" .dteam >>${target_file}
else
    sed -i 's/# Ignore/'${searchstring}'/g' ${target_file};# echo $?
fi

target_file=/etc/grid-security/groupmapfile
grep -q '# Ignore' ${target_file}
if [ $? -ne 0 ]
then
    echo \"${xxx_tmp}\" dteam >>${target_file}
else
    sed -i 's/# Ignore/'${searchstring}'/g' ${target_file};# echo $?
fi

/etc/rc.d/init.d/$PAP_CTRL restart
pdp_start
#
# Now we should modify the /opt/argus/pep/conf/pepd.ini to take the correct
#
# preferDNForLoginName = true
# preferDNForPrimaryGroupName = true
# noPrimaryGroupNameIsError = false
#
# Needs to go AFTER 
# parserClass = org.glite.authz.pep.obligation.dfpmap.DFPMObligationHandlerConfigurationParser
# 
# In the case of testing... this does actually go at the 
# end of the pepd.ini file but may not be guaranteed.
#
# pep_config="/opt/argus/pepd/conf/pepd.ini"
grep -q 'org.glite.authz.pep.obligation.dfpmap.DFPMObligationHandlerConfigurationParser' ${pep_config}
if [ $? -ne 0 ]
then
    echo "${script_name}: Obligation handler not defined"
    failed="yes"
    exit 1;
fi
rm /etc/grid-security/gridmapdir/*`hostname`*
echo "preferDNForLoginName = false" >> ${pep_config}
echo "noPrimaryGroupNameIsError = true" >> ${pep_config}
echo "preferDNForPrimaryGroupName = false" >> ${pep_config}

pep_start

$PEPCLI $OPTS -p https://`hostname`:8154/authz \
       -c $USERCERT \
       --capath /etc/grid-security/certificates/ \
       --key $USERKEY \
       --cert $USERCERT \
       -r "resource_1" \
       -a "testwerfer" > /tmp/${script_name}.out
result=$?; echo $result

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
