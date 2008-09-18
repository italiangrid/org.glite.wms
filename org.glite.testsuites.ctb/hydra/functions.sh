#!/bin/bash
#
# Shared functions for the Hydra functionality tests. 
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

###############################
###### General functions ######
###############################


function usage() {
  echo "Hyrdra server and client tests" 
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
 
  # No setup for the server test
  if [ "$1" == server ] ; then
    return
  fi 

  GLITE_SD_PLUGIN=${GLITE_SD_PLUGIN:-bdii}
  GLITE_LFC_HOST=${GLITE_LFC_HOST:-lxb7607.cern.ch}
  LFC_TEST_DIR=${LFC_TEST_DIR:-/grid/dteam/hydratest}
  MAN_PAGE_CHECK=${MAN_PAGE_CHECK:-1}
  TOMCAT_CFG_DIR=${TOMCAT_CFG_DIR:-/etc/tomcat5/Catalina/localhost/}
  STRESS_TEST_PROCS=${STRESS_TEST_PROCS:-10}
  STRESS_TEST_KEYS_PER_PROC=${STRESS_TEST_KEYS_PER_PROC:-100}


  export GLITE_SD_PLUGIN
  export GLITE_LFC_HOST
  export LFC_TEST_DIR
  export MAN_PAGE_CHECK
  export TOMCAT_CFG_FIR
  export STRESS_TEST_PROCS
  export STRESS_TEST_KEYS_PER_PROC

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
 
  export USE_2_PROXIES=0
  # Test for a second proxy certificate
  if [ -e "$X509_USER_PROXY_2" ] ; then
    voms-proxy-info -file $X509_USER_PROXY_2 >/dev/null 2>&1 
    
    if [ $? -ne 0 ] ; then
      echo "Could not read second proxy certificate, using one proxy certificate"
    else
      DN_ONE=`voms-proxy-info -identity`
      DN_TWO=`voms-proxy-info -file $X509_USER_PROXY_2 -identity`

      if [ "$DN_ONE" = "$DN_TWO" ] ; then
        echo "Both proxy certificates have same DN, using only one"
      else
        echo "Second user proxy certificate found"
        export USE_2_PROXIES=1
      fi
    fi
  fi

}


function check_man {
  man -w $1 >/dev/null
  if [ $? -ne 0 ] ; then
    echo "Man page for $1 not found"
    my_exit $ERROR
  fi

}

###############################
### Common hydra functions ####
###############################

function register_key {
  # Test registering a key
  glite-eds-key-register $1 >/dev/null

  if [ $? -ne 0 ] ; then
    echo "Error registering key"
    my_exit $ERROR
  fi
}

function unregister_key {
  # Test unregistering a key
  glite-eds-key-unregister $1 >/dev/null

  if [ $? -ne 0 ] ; then
    echo "Error unregistering key"
    my_exit $ERROR
  fi
}

function set_acl {
  # Set acl
  glite-eds-setacl $@

  if [ $? -ne 0 ] ; then
    echo "Error setting acl"
    cleanup
    my_exit $ERROR
  fi
}

function get_acl {
  # Echo the ACL of a key into a file
  glite-eds-getacl $1 > $2

  if [ $? -ne 0 ] ; then
    echo "Error getting acl"
    cleanup
    my_exit $ERROR
  fi
}

