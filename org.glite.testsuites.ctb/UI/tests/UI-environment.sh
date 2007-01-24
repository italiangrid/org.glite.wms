#!/bin/sh

# This test helps to reveal common problems with shell environment,
# such as wrong entries in $PATH and "similar" variables.
# A failure is returned if a "single-entry" variable pointing to non-existing directory is found.
# For colon-separated lists, such as $PATH, only a warning will be shown.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$


echo " === general shell environment test === "

function testpath() {
  echo " --- testing $1 --- "
  if [ -n "$2" ]; then
    if [ -d "$2" ]; then
    echo "info   : $1 is set to $2"
    else
      echo "ERROR  : $1 is set to non-existing location $2"
      echo " === test NOT passed === "
      exit 1
    fi
  else
    echo "info    : $1 is not defined"
  fi
}

function testmultipath() {
  echo " --- testing $1 --- "
  for DIR in `echo $2 | sed 's/:/\n/g' | sort` ; do
    if [ -d "$DIR" ]; then
      echo "info   : $1 has $DIR"
    else
      echo "WARNING: $1 lists a non-existing location $DIR"
    fi
  done
}

testmultipath PATH $PATH
testmultipath LD_LIBRARY_PATH $LD_LIBRARY_PATH
testmultipath PYTHONPATH $PYTHONPATH
testmultipath PERLLIB $PERLLIB
testmultipath PERL5LIB $PERL5LIB
testmultipath MANPATH $MANPATH

testpath JAVA_HOME $JAVA_HOME
testpath JAVA_INSTALL_PATH $JAVA_INSTALL_PATH

echo " === test PASSED (but check warnings!) === "
exit 0
