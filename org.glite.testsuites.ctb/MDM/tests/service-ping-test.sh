#!/bin/bash
#
# Tests the that the required services for MDM are found
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

. $(dirname $0)/functions.sh

if [ $# -ne 0 ] ; then
  usage $0
  if [ $1 = "--help" ] ; then
    my_exit $SUCCESS
  else
    my_exit $ERROR
  fi
fi

setup 

###############################
##### Check if AMGA works #####
###############################

mdcli dir >/dev/null 2>&1 

if [ $? -ne 0 ] ; then
  echo "There were problems with contacting the AMGA server"
  my_exit $ERROR
fi

echo "AMGA server found"

###############################
##### Check if hydra works ####
###############################

KN=`mktemp`
rm $KN

glite-eds-key-register $KN >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "There were problems with contacting the Hydra server"
  my_exit $ERROR
fi

glite-eds-key-unregister $KN >/dev/null 2>&1

echo "Hydra server found" 

###############################
###### Check if LFC works #####
###############################

lfc-ls / >/dev/null 2>&1 

if [ $? -ne 0 ] ; then
  echo "There were problems with contacting the LFC server"
  my_exit $ERROR
fi

echo "LFC server found" 

###############################
###### Check if DPM works #####
###############################

dpns-ls / >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There were problems with contacting the DPM server"
  my_exit $ERROR
fi

echo "DPM server found" 


###############################
############ Done #############
###############################

echo "Tests completed"
my_exit $SUCCESS
