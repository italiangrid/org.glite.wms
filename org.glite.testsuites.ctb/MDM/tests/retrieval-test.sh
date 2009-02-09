#!/bin/bash
#
# Tests the glite-mdm-get command
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
TEMP_FILE_2=`mktemp`

###############################
###### Test for man pages #####
###############################

if [ "$MAN_PAGE_CHECK" -ne 0 ] ; then
  check_man glite-mdm-get
  echo "Man pages found"
fi

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $TEMP_FILE ] ; then
    rm -f $TEMP_FILE
  fi

  if [ -e $TEMP_FILE_2 ] ; then
    rm -f $TEMP_FILE_2
  fi

  glite-mdm-del -f $TEST_IMAGE_1 >/dev/null 2>&1

  if [ $? -ne 0 ] ; then
    echo "There was an error removing image $TEST_IMAGE_1"
    my_exit $ERROR
  fi
}

###############################
#### Register test image ######
###############################

# Re-register the image for the tests
glite-mdm-register $TEST_IMAGE_1 >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "There was an error registering the image $TEST_IMAGE_1" 
  rm $TEMP_FILE
  my_exit $ERROR
fi

###############################
# Retrieve img with se/st/sop #
###############################

# Retrieve the image
glite-mdm-get $TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP $TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error retrieving the image "$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP
  cleanup
  my_exit $ERROR
fi

echo "Dicom image succesfully retrieved with glite-mdm-get study/series/sop"

# Check that registered and retrieved files are identical
diff --brief $TEST_IMAGE_1 $TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Registered file and retrieved file differ"
  cleanup
  my_exit $ERROR
fi

echo "Registered and retrieved files match"

rm $TEMP_FILE

###############################
#### Retrieve img with LFN ####
###############################

# Retrieve the image
glite-mdm-get -l lfn:$LFN $TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error retrieving the image lfn:/grid/dteam/mdm/"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP
  cleanup
  my_exit $ERROR
fi

echo "Dicom image succesfully retrieved with glite-mdm-get lfn"

# Check that registered and retrieved files are identical
diff --brief $TEST_IMAGE_1 $TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Registered file and retrieved file differ"
  cleanup
  my_exit $ERROR
fi

echo "Registered and retrieved files match"

rm $TEMP_FILE

###############################
### Retrieve img with guid ####
###############################

GUID=`lcg-lg lfn:$LFN`

# Retrieve the image
glite-mdm-get -g $GUID  $TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "There was an error retrieving the image $GUID"
  cleanup
  my_exit $ERROR
fi

echo "Dicom image succesfully retrieved with glite-mdm-get guid"

# Check that registered and retrieved files are identical
diff --brief $TEST_IMAGE_1 $TEMP_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Registered file and retrieved file differ"
  cleanup
  my_exit $ERROR
fi

echo "Registered and retrieved files match"

rm $TEMP_FILE

###############################
# Retrieve img with std tools #
###############################

lcg-cp lfn:$LFN file:$TEMP_FILE>/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Failed to retrieve image lfn:$LFN with standard gLite tools"
  cleanup
  my_exit $ERROR
fi

echo "Succesfully retrieved image with standard gLite tools"

# Check that registered and retrieved files differ, since it should be encrypted
diff --brief $TEST_IMAGE_1 $TEMP_FILE >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "Registered file and retrieved file do not differ, even if retrieved file should be encrypted"
  cleanup
  my_exit $ERROR
fi

echo "Registered and retrieved files correctly differ, retrieved file should be encrypted"

# Decrypt the image

glite-eds-decrypt /"$TEST_IMAGE_1_STUDY"/"$TEST_IMAGE_1_SERIES"/"$TEST_IMAGE_1_SOP" $TEMP_FILE $TEMP_FILE_2 
if [ $? -ne 0 ] ; then
  echo "Failed to decrypt encrypted image"
  cleanup
  my_exit $ERROR
fi

echo "Successfully decrypted retrieved image"

# Check that registered and decrypted files are identical
diff --brief $TEST_IMAGE_1 $TEMP_FILE_2 >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Registered file and decrypted file differ"
  cleanup
  my_exit $ERROR
fi

echo "Registered and decrypted files are identical"

 
###############################
############ Done #############
###############################

cleanup
echo "Tests completed"
my_exit $SUCCESS
