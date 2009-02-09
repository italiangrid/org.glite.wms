#!/bin/bash
#
# Shared functions for the MDM functionality tests. 
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

###############################
###### General functions ######
###############################


function usage() {
  echo "Medical Data Management server and client tests" 
  echo "Usage:" 
  echo "$0 [--help]" 
  echo "" 
  echo "   --help   Distplay this help" 
  echo "" 
  echo "If no commands are given, the test is run." 

  echo "" 
}

function my_exit() {
  exit $1
}

function setup() {
  . config.sh

  # Define errors
  export ERROR=0
  export SUCCESS=1
 
  GLITE_SD_PLUGIN=${GLITE_SD_PLUGIN:-bdii}
  LFC_HOST=${LFC_HOST:-lxb7608v3.cern.ch}
  LFC_TEST_DIR=${LFC_TEST_DIR:-/grid/dteam/mdm}
  MAN_PAGE_CHECK=${MAN_PAGE_CHECK:-1}
  TOMCAT_CFG_DIR=${TOMCAT_CFG_DIR:-/etc/tomcat5/Catalina/localhost/}
  MDM_CONFIG_FILE=${MDM_CONFIG_FILE:-/etc/MedicalDataManager.conf}
  TEST_IMAGE_1=${TEST_IMAGE_1:-images/test1.dcm}

  export GLITE_SD_PLUGIN
  export GLITE_LFC_HOST
  export LFC_TEST_DIR
  export MAN_PAGE_CHECK
  export TOMCAT_CFG_FIR
  export MDM_CONFIG_FILE
  export TEST_IMAGE_1

  source $MDM_CONFIG_FILE
  # Test for proxy certificate
  voms-proxy-info >/dev/null 2>&1 

  if [ $? -ne 0 ] ; then
    cat << EOF
Could not find a valid voms proxy certificate. 
Please check that the proxy certificate is in 
the default location or that X509_USER_PROXY
is set.

Aborting.
EOF
    my_exit $ERROR
  fi
  
  # Get the file info ready for scripts

  RES=0
  # Get info from files
  TEST_IMAGE_1_STUDY=`$MDM_GetUiDFile  $TEST_IMAGE_1 STUDY_INSTANCE_UID`
  RES=$(($RES+$?))
  TEST_IMAGE_1_SERIES=`$MDM_GetUiDFile  $TEST_IMAGE_1 SERIES_INSTANCE_UID`
  RES=$(($RES+$?))
  TEST_IMAGE_1_SOP=`$MDM_GetUiDFile  $TEST_IMAGE_1 SOP_INSTANCE_UID`
  RES=$(($RES+$?))

  if [ $RES -ne 0 ] ; then
    echo "Failed getting info from test files, are these DICOM files?"
    my_exit $ERROR
  fi

  LFN=$MDM_LfcPath/$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP
  DPMN=$MDM_DpmPath/$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP
}


function check_man {
  man -w $1 >/dev/null
  if [ $? -ne 0 ] ; then
    echo "Man page for $1 not found"
    my_exit $ERROR
  fi

}

