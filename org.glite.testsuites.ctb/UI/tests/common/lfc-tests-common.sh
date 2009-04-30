#!/bin/sh
#
# Common functions for the UI LFC tests.
#
# The startup procedure defined here is common for all the UI-lfc- tests and defines
# the accepted command line parameters.
#
# UI-data-lfc-<name>.sh [--lfchost <lfchost>] [--lfcdir <lfcdir>]
#       <lfchost> LFC host name. When given it overwrites LFC_HOST.
#       <lfcdir> Working directory in LFC, the directory should exist (see Note 3).
#
# Note 1. lfc- commands require the hostname of the LFC catalog to be defined in LFC_HOST.
# You can supply your value for LFC_HOST using --lfchost (the new value will be assigned to LFC_HOST).
# If niether LFC_HOST is set nor --lfchost option is given the tests will try to obtain
# LFC hostname from the informatin system.
#
# Note 2. LFC commands recognize relative path if LFC_HOME is set (then LFC_HOME is used as prefix).
# The LFC test script do not modify LFC_HOME. However if LFC_HOME is not empty the tests will
# attempt to use relative pathes. Thus one could test both relative and absolute pathes by
# setting and unsetting LFC_HOME before running the tests.
#
# Note 3. The test working directory is the LFC directory to be listed by the lfc-ls test and
# to be used by other tests as parent directory for newly created directories.
# You can supply your preferred name for the working directory in command line using --lfcdir.
# If not supplied it will be set to LFC_HOME.
# If LFC_HOME is empty an attempt will be made to define the working directory in the form /grid/<vo>
# Note though that currently when LFC_HOME is set beforehand the --lfcdir option is actually
# ignored by most of the tests (because they use relative path in that case).
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

# ... define LFC_HOME (used by lfc- commands), LFC_DIR (used by the lfc-ls test) and TEST_DIR (used by the other lfc tests)

# ... prefixed echo

function myecho() {
  echo " # ui lfc test # $@"
}

# ... failure exit with clean-up

function exit_failure() {

  if [ "$TEST_DIR_CREATED" == "yes" ]; then
    echo ""
    myecho "Removing $TEST_DIR"
    lfc-rm -r $TEST_DIR
  fi

  echo ""
  echo " *** Make sure you have a valid [voms] proxy, correct LFC hostname is defined in LFC_HOST"
  echo "     or supplied with --lfchost, the working directory $LFC_DIR exists"
  echo "     and, if using relative path for the working directory, LFC_HOME is also defined *** "
  echo " *** test NOT passed *** "
  exit 1

}

# ... good exit

function exit_success() {

 echo ""
 echo "    === test PASSED === "
 exit 0

}

function lfc_test_startup() {

  # ... Parse options
  #     Quotation is disabled to allow constructions like "--vo $2" to work correctly
  #     But beware you can not pass a vo name containing spaces or other special characters because of that

  OPTS=`getopt --unquoted --longoptions "lfchost:,lfcdir:" --options "" -- "$@"`

  if [ $? -ne 0 ]; then
    echo "Usage: $0 [--lfchost <lfchost>] [--lfcdir <lfcdir>]"
    exit_failure
  fi

  set -- $OPTS
  
  while [ ! -z "$1" ]
  do
    case "$1" in

      --lfchost) myecho "LFC host given in command line: $2"
		 export LFC_HOST=$2
		 shift
		 shift
		 ;;
      --lfcdir) myecho "LFC directory given in command line: $2"
		LFC_DIR=$2
		shift
		shift
		;;
       *) break;;
    esac
  done

  if [ -z "$LFC_DIR" ] && [ -n "$LFC_HOME" ]; then
    myecho "Will use LFC_HOME as working directory"
    LFC_DIR=$LFC_HOME
  fi
  
  if [ -z "$LFC_HOST" ] || [ -z "$LFC_DIR" ]; then
    myecho "Either LFC host or LFC directory is not defined - will try to guess"
    myecho "Trying to get user VO using voms-proxy-info -vo"
    VO=`voms-proxy-info -vo` 
    if [ $? -ne 0 ] || [ -z "$VO" ]; then
      echo $VO
      myecho "Could not get VO name - have to abandon the test"
      exit_failure
    fi
    myecho "User VO according to voms-proxy-info: $VO"
  fi

  if [ -z "$LFC_HOST" ]; then
    myecho "LFC hostname is not defined neither by LFC_HOST nor supplied with --lfchost"
    myecho "Trying to get LFC hostname using lcg-infosites --vo $VO lfc"
    LFC_HOSTS=`lcg-infosites --vo $VO lfc`
    if [ $? -ne 0 ]; then
      echo $LFC_HOSTS
      myecho "Could not get LFC host name lcg-infosites says $LFC_HOST"
      myecho "Please define \$LFC_HOST or supply LFC host name with --lfchost"
      #myecho "Usually you can find out where LFC is using lcg-infosites --vo <vo> lfc"
      exit_failure
    fi
    echo "$LFC_HOSTS"
    export LFC_HOST=`head -n 1 <<<"$LFC_HOSTS"`
    myecho "Got LFC hostname: $LFC_HOST"
  fi

  if [ -z "$LFC_DIR" ]; then
    export LFC_DIR=/grid/$VO
    myecho "LFC working directory is not defined - will use /grid/<vo>=$LFC_DIR"
  fi

  export LFC_CONNTIMEOUT=10 # connect timeout in seconds 
  export LFC_CONRETRY=5     # number of retries
  export LFC_CONRETRYINT=3  # retry interval in seconds

  echo ""
  myecho "LFC environment  : LFC_HOST=$LFC_HOST"
  myecho "LFC environment  : LFC_HOME=$LFC_HOME"
  myecho "Working directory: LFC_DIR =$LFC_DIR"


  # ... define the name of the directory to create

  RELATIVE_NAME=UI-data-lfc-mkdir-ls-rm-test-`date +%y-%m-%d-%H-%M-%S`-$$
  FULL_NAME=$LFC_DIR/$RELATIVE_NAME

  # ... will test relative path if LFC_HOME is defined

  if [ -z "$LFC_HOME" ]; then
    TEST_DIR=$FULL_NAME
  else
    TEST_DIR=$RELATIVE_NAME
  fi

}
