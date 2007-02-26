#!/bin/sh
#
# Common functions for the UI LCG data management tests.
#
# The startup procedure defined here is common for all the UI-data-lcg- tests and defines
# the accepted command line parameters.
#
# UI-data-lcg-<name>.sh [--vo <VO>] [-d <SE>]
#
#  --vo <VO> : VO to be fed to lcg- commands. If not given, and LCG_GFAL_VO not defined
#       an attempt is made to determine VO name using voms-proxy-info
#
#  -d <SE> : Storage Element host name. This option is mandatory inless VO_<VO>_DEFAULT_SE is defined
#
# Example usage:
# UI-data-lcg-cr-lr-lg-gt-del.sh --vo dteam -d lxb0724.cern.ch
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

# ... prefixed echo

function myecho() {
  echo " # ui lcg test # $@"
}

# ... Remove temporary file if needed

function cleanup() {

  if [ -f "$LOCAL_FILE" ]; then
    #echo ""
    #myecho "Removing $LOCAL_FILE"
    rm -f $LOCAL_FILE
  fi

}

# ... Remove the GRID file if needed (GUID must point to the file to be removed!)

function cleanup_remote() {

  if [ -n "$GUID" ] ; then

    echo ""; myecho "lcg-del $VO_OPTIONS -s $SE_HOST $GUID"; echo ""
    lcg-del $VO_OPTIONS -s $SE_HOST $GUID

    if [ $? -ne 0 ]; then
      myecho "lcg-del failed"
      return 1
    fi
  fi

  return 0
}

# ... Exit with cleanup

function myexit() {

  cleanup

  cleanup_remote
  
  if [ $? -ne 0 ] || [ "$1" != "0" ]; then

    echo ""
    echo " *** Make sure you have a valid [voms] proxy and correct SE is given *** "
    echo " *** test NOT passed *** "
    exit 1
  fi
 
  echo "    === test PASSED === "
  exit 0
}

function lcg_test_startup() {

  VO_OPTIONS=""
  SE_HOST=""
  GUID=""
  
  # ... parse options

  if [ "$1" == "--vo" ] && [ -n "$2" ]; then
    myecho "VO given in command line: $2"
    VO_OPTIONS="--vo $2"
    myecho "Will use VO Options:" $VO_OPTIONS
    shift
    shift
  fi

  if [ "$1" == "-d" ] && [ -n "$2" ]; then
    myecho "SE host given in command line: $2"
    SE_HOST=$2
    shift
    shift
  fi
  
  # ... define VO options

  if [ -z "$VO_OPTIONS" ]; then
    if [ -n "$LCG_GFAL_VO" ]; then
      myecho "VO defined by LCG_GFAL_VO: $LCG_GFAL_VO"
    else
      myecho "Trying to get user VO using voms-proxy-info -vo"
      VO=`voms-proxy-info -vo`
      if [ $? -ne 0 ] || [ -z "$VO" ]; then
        echo $VO
        myecho "Could not get VO name - have to abandon the test"
        myexit 1
      fi
      myecho "Will use VO: $VO"
      VO_OPTIONS="--vo $VO"
      myecho "Will use VO Options:" $VO_OPTIONS
    fi
  fi

  # ... define SE host

  if [ -z "$SE_HOST" ]; then
    DEFAULT_SE=`printenv VO_${VO}_DEFAULT_SE`
    if [ -n "$DEFAULT_SE" ]; then
      myecho "SE defined in VO_${VO}_DEFAULT_SE: $DEFAULT_SE"
    else
      myecho "Please supply SE hostname with -d option"
      myexit 1
    fi
  fi
  
  myecho "[for information]: LFC_HOST=$LFC_HOST"

  # ... create temporary test file

  LOCAL_FILE=/tmp/testfile_`id -u`_$$
  LOCAL_FILE_URI=file:$LOCAL_FILE

  echo "ui lcg test file created `date`" > $LOCAL_FILE || myexit 1
  echo ""
  echo " ---    $LOCAL_FILE     --- "
  cat $LOCAL_FILE || myexit 1
  LOCAL_FILE_CREATED="yes"
  echo " --- end of $LOCAL_FILE --- "
}
