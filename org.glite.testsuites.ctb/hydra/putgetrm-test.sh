#!/bin/bash
#
# Tests glite-eds-put, glite-eds-get and glite-eds-rm commands
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
###### Test for man pages #####
###############################

if [ "$MAN_PAGE_CHECK" -ne 0 ] ; then
  check_man glite-eds-put
  check_man glite-eds-get
  check_man glite-eds-rm
fi

###############################
###### Prepare testfiles ######
###############################

ORIG_FILE=`mktemp`
RETR_FILE=`mktemp`
ENC_FILE=`mktemp`

cat >> $ORIG_FILE <<EOF
A quick brown fox
jumps over the
lazy dog
EOF

lfc-ls $LFC_TEST_DIR

if [ $? -eq 0 ] ; then
  echo "LFC path already exists. Please choose another path"
  rm $ORIG_FILE $RETR_FILE $ENC_FILE
  my_exit $ERROR
fi

lfc-mkdir $LFC_TEST_DIR
if [ $? -eq 0 ] ; then
  echo "Could not create lfc path"
  rm $ORIG_FILE $RETR_FILE $ENC_FILE
  my_exit $ERROR
fi

LFC_HOME=$LFC_TEST_DIR

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $ORIG_FILE ] ; then
    rm -f $ORIG_FILE 
  fi

  if [ -e $RETR_FILE ] ; then
    rm -f $RETR_FILE
  fi

  if [ -e $ENC_FILE ] ; then
    rm -f $ENC_FILE
  fi
 
  lfc-rm -r $LFC_TEST_DIR
}

###############################
##### Write and read file #####
###############################

glite-eds-put $ORIG_FILE lfc:testfile1

if [ $? -ne 0 ] ; then
  echo "Error in creating file"
  cleanup
  my_exit $ERROR
fi

glite-eds-get lfc:testfile1 $RETR_FILE
if [ $? -ne 0 ] ; then
  echo "Error retrieving file"
  glite-eds-rm lfc:testfile1
  cleanup
  my_exit $ERROR
fi

diff --brief $RETR_FILE $ORIG_FILE
if [ $? -ne 0 ] ; then
  echo "Original and retrieved files don't match"
  cleanup
  my_exit $ERROR
fi

###############################
### Retrieve encrypted file ###
###############################

lcg-cp lfc:testfile1 file:$ENC_FILE
if [ $? -ne 0 ] ; then
  echo "Unable to retrieve (lfc-cp) file"
  glite-eds-rm lfc:testfile1
  cleanup
  my_exit $ERROR
fi

diff --brief $ENC_FILE $ORIG_FILE
if [ $? -eq 0 ] ; then
  echo "Original file the same as the encrypted file"
  glite-eds-rm lfc:testfile1
  cleanup
  my_exit $ERROR
fi

echo "" > $RETR_FILE
glite-eds-decrypt $ENC_FILE $RETR_FILE

if [ $? -eq 0 ] ; then
  echo "Unable to decrypt file"
  cleanup
  glite-eds-rm lfc:testfile1
  my_exit $ERROR
fi

diff --brief $RETR_FILE $ORIG_FILE
if [ $? -ne 0 ] ; then
  echo "Original and retrieved files don't match"
  glite-eds-rm lfc:testfile1
  cleanup
  my_exit $ERROR
fi

glite-eds-rm lfc:testfile1
if [ $? -ne 0 ] ; then
  echo "Error removing file and key"
  cleanup
  my_exit $ERROR
fi


###############################
############ Done #############
###############################

cleanup
echo "Tests completed"
my_exit $SUCCESS
