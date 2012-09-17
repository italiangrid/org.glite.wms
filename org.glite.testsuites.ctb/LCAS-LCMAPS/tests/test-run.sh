#!/bin/bash
#definable variables

#Different certificates
TESTCERT_OK="test_user_100_cert.pem"
TESTKEY_OK="test_user_100_key.pem"
TESTCERT_BAN="test_user_101_cert.pem"
TESTKEY_BAN="test_user_101_key.pem"

#voms
TESTVOMS=dteam

#jdl file
JDL=testJob.jdl

PHASE=0

function myexit() {

  if [ $1 -ne 0 ]; then
    echo " *** something went wrong *** "
    echo " *** test NOT passed *** "
    exit $1
  else
    echo ""
    echo "    === phase $PHASE of the test PASSED === "
    if [ $PHASE -eq 3 ] ; then
     echo "    === ALL TESTS PASSED === "
    fi
  fi
   
  exit 0
}

function myecho()
{
  echo "#LCAS client tests# $1"
}


usage() {
 echo
 echo "Script for running the LCAS client tests"
 echo "The tests are run in three phases, please initialize each phase on the server before running it"
 echo "test-run.sh --phase 1|2|3"
 echo
}


while [ $# -gt 0 ]
do
 case $1 in
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --phase | -phase | --p | -p ) PHASE=$2
  shift
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done

if [ $PHASE -lt 1 ]  ||  [ $PHASE -gt 3 ] ; then
 usage
 exit 0
fi

if [ $PHASE -eq 1 ] ; then
 export X509_USER_CERT=$TESTCERT_OK
 export X509_USER_KEY=$TESTKEY_OK
 voms-proxy-init --voms $TESTVOMS
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a normal job"
 glite-wms-job-submit -a $JDL
 if [ $? -ne 0 ] ; then
  myecho "Error, could not submit job with proxy from $TESTCERT_OK even if this should be possible"
  myexit 1
 fi
 myecho "Job submission succeeded, trying with a non-VOMS proxy"

 voms-proxy-init 
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a non-voms job"
 glite-wms-job-submit -a $JDL
 if [ $? -eq 0 ] ; then
  myecho "Error, job submission succeeded proxy from $TESTCERT_OK even if it didn't have voms information"
  myexit 1
 fi
 myecho "Test succeeded, trying with a bad VOMS proxy"

 voms-proxy-fake -hostcert $TESTCERT_OK -hostkey $TESTCERT_OK -voms $TESTVOMS
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a fake-voms job"
 glite-wms-job-submit -a $JDL
 if [ $? -eq 0 ] ; then
  myecho "Error, job submission succeeded proxy from $TESTCERT_OK even if it had fake voms information"
  myexit 1
 fi
 myecho "Test succeeded, trying with a banned user"

 export X509_USER_CERT=$TESTCERT_BAN
 export X509_USER_KEY=$TESTKEY_BAN
 voms-proxy-init --voms $TESTVOMS
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_BAN"
  myexit 1
 fi
 glite-wms-job-submit -a $JDL
 if [ $? -eq 0 ] ; then
  myecho "Error, job submission succeeded with $TESTCERT_BAN even if this user should be banned"
  myexit 1
 fi

 myecho "Phase 1 complete, please setup phase 2 on the server, and run the phase 2 of this script"

elif [ $PHASE -eq 2 ] ; then
 export X509_USER_CERT=$TESTCERT_OK
 export X509_USER_KEY=$TESTKEY_OK
 voms-proxy-init --voms $TESTVOMS
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a normal allowed job against GACL"
 glite-wms-job-submit -a $JDL
 if [ $? -ne 0 ] ; then
  myecho "Error, could not submit job with proxy from $TESTCERT_OK even if this should be possible"
  myexit 1
 fi
 myecho "Job submission succeeded, trying with a non-VOMS proxy agaist GACL"

 voms-proxy-init 
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a non-voms job"
 glite-wms-job-submit -a $JDL
 if [ $? -eq 0 ] ; then
  myecho "Error, job submission succeeded proxy from $TESTCERT_OK even if it didn't have voms information"
  myexit 1
 fi
 myecho "Test succeeded, trying with a bad VOMS proxy against GACL"

 voms-proxy-fake -hostcert $TESTCERT_OK -hostkey $TESTCERT_OK -voms $TESTVOMS
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a fake-voms job"
 glite-wms-job-submit -a $JDL
 if [ $? -eq 0 ] ; then
  myecho "Error, job submission succeeded proxy from $TESTCERT_OK even if it had fake voms information"
  myexit 1
 fi
 myecho "Test succeeded"

 myecho "Phase 2 complete, please setup phase 3 on the server, and run the phase 2 of this script"

else
 #PHASE = 3
 export X509_USER_CERT=$TESTCERT_OK
 export X509_USER_KEY=$TESTKEY_OK
 voms-proxy-init --voms $TESTVOMS
 if [ $? -ne 0 ] ; then
  myecho "Error getting voms proxy for $TESTCERT_OK"
  myexit 1
 fi

 myecho "Trying to submit a normal job but timeslot should be denied"
 glite-wms-job-submit -a $JDL
 if [ $? -eq 0 ] ; then
  myecho "Error, job submission with proxy from $TESTCERT_OK even if this should be denied"
  myexit 1
 fi
fi

myexit 0
