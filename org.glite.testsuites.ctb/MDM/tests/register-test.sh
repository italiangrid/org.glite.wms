#!/bin/bash
#
# Tests the glite-mdm-register command
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

TEMP_FILE=`mktemp`

###############################
###### Test for man pages #####
###############################

if [ "$MAN_PAGE_CHECK" -ne 0 ] ; then
  check_man glite-mdm-register
  check_man glite-mdm-del
  echo "Man pages found"
fi

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $TEMP_FILE ] ; then
    rm -f $TEMP_FILE
  fi

  glite-mdm-del -f $TEST_IMAGE_1 >/dev/null 2>&1

  if [ $? -ne 0 ] ; then
    echo "There was an error removing image $TEST_IMAGE_1"
    my_exit $ERROR
  fi
}

###############################
####### Normal register #######
###############################


glite-mdm-register $TEST_IMAGE_1 >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error registering the image $TEST_IMAGE_1"
  rm $TEMP_FILE
  my_exit $ERROR
fi

echo "Dicom image succesfully registered"

###############################
## Check that reg. succeeded ##
###############################

# Check that image info exists in amga
GUID=`mdcli find /mdm/DICOM/ \''SERinsUID = "'$TEST_IMAGE_1_SERIES'" and STUinsUID = "'$TEST_IMAGE_1_STUDY'" and SOPinsUID = "'$TEST_IMAGE_1_SOP'"'\'`

if [ $? -ne 0 ] ; then
  echo "There was an error retrieving AMGA metadata for registered image $TEST_IMAGE_1"
  cleanup
  my_exit $ERROR
fi

echo $GUID |grep "^guid" > /dev/null

if [ $? -ne 0 ] ; then
  echo "No AMGA information found for registered image $TEST_IMAGE_1"
  cleanup
  my_exit $ERROR
fi

echo "AMGA Metadata succesfully found for $TEST_IMAGE_1"

# Check that image key info exist in hydra
glite-eds-getacl "/"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "No Hydra key found for image $TEST_IMAGE_1"
  cleanup
  my_exit $ERROR
fi

echo "Hydra key found for $TEST_IMAGE_1"

#Check that the file is registered in LFC and that it's accessible
lfc-ls $LFN  >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "No LFC link found for image $TEST_IMAGE_1"
  cleanup
  my_exit $ERROR
fi

echo "LFC link found for $TEST_IMAGE_1"

#Try to copy the file
lcg-cp lfn:$LFN file:$TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Could not copy file from storage."
  cleanup
  my_exit $ERROR
fi

rm $TEMP_FILE

echo "File succesfully copied from storage"

###############################
## Test re-registering image ##
###############################

glite-mdm-register $TEST_IMAGE_1 >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "No error in registering already registered file"
  cleanup
  my_exit $ERROR
fi

echo "Registering an already registered file gives an error"

###############################
##### Test removing image #####
###############################

glite-mdm-del -f $TEST_IMAGE_1 >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Error removing image from MDM"
  my_exit $ERROR
fi

echo "Image succesfully removed"



###############################
## Check that rem. succeeded ##
###############################

# Check that image info was removed from amga
GUID=`mdcli find /mdm/DICOM/ \''SERinsUID = "'$TEST_IMAGE_1_SERIES'" and STUinsUID = "'$TEST_IMAGE_1_STUDY'" and SOPinsUID = "'$TEST_IMAGE_1_SOP'"'\'`

if [ $? -eq 0 ] ; then
  echo "The image $TEST_IMAGE_1 found in AMGA, even if it was removed from MDM"
  my_exit $ERROR
fi

echo "AMGA Metadata was succesfully removed for $TEST_IMAGE_1"

# Check that image key info was removed from hydra
glite-eds-getacl "/"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "Hydra key found for image $TEST_IMAGE_1, even if it was removed from MDM"
  my_exit $ERROR
fi

echo "Hydra key was succesfully removed for $TEST_IMAGE_1"

#Check that the file is not registered in LFC
lfc-ls $LFN  >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "LFC link found for image $TEST_IMAGE_1, even if it was removed from MDM"
  my_exit $ERROR
fi

echo "LFC link was succesfully removed for $TEST_IMAGE_1"

#Try to copy the file
dpns-ls $DPMN  >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "Found $TEST_IMAGE_1 in the DPM, even if it was removed from MDM"
  my_exit $ERROR
fi


echo "$TEST_IMAGE_1 was succesfully removed from DPM"



###############################
##### Test removing image #####
###############################

glite-mdm-del -f $TEST_IMAGE_1 >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "Removing an already removed image doesn't give an error"
  my_exit $ERROR
fi

echo "Trying to remove an already removed image correctly gives an error"


###############################
############ Done #############
###############################

echo "Tests completed"
my_exit $SUCCESS
