#!/bin/bash

# This test helps to reveal common problems with environment variables,
# such as wrong entries in $PATH and "similar" variables.
# A failure is returned if a "single-entry" variable pointing to non-existing directory is found.
# For colon-separated lists, such as $PATH, only a warning will be shown.
#
# Usage: 
# UI-environment.sh - test environment seen from a bash script (~/.bashrc is executed)
# or
# source UI-environment.sh - test current shell environment (from bash only)
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo " === bash environment test === "

# ... Part I: colon-separated lists

for VAR_NAME_TEST in PATH LD_LIBRARY_PATH PYTHONPATH PERLLIB PERL5LIB MANPATH ; do

  echo " --- testing $VAR_NAME_TEST --- "

  # ... note that printenv is executed in a subshell, so variables not marked for export will not be seen
  # ... and this is actually what we want since external (gLite) commands will not see such variables either

  for DIR_NAME_TO_TEST in `printenv $VAR_NAME_TEST | sed 's/:/\n/g' | sort` ; do

    if [ -d "$DIR_NAME_TO_TEST" ]; then
      echo "info   : $VAR_NAME_TEST has $DIR_NAME_TEST"
    else
      echo "WARNING: $VAR_NAME_TEST lists a non-existing location $DIR_NAME_TEST"
    fi

  done

done


# .. error flag

ENVIRONMENT_ERRORS=0

# ... Part II: simple directory names

for VAR_NAME_TEST in JAVA_HOME JAVA_INSTALL_PATH ; do

  echo " --- testing $VAR_NAME_TEST --- "

  DIR_NAME_TEST=`printenv $VAR_NAME_TEST`

  if [ -z "$DIR_NAME_TEST" ]; then
    echo "info    : $VAR_NAME_TEST is not defined or empty"
  elif [ -d "$DIR_NAME_TEST" ]; then
    echo "info   : $VAR_NAME_TEST is set to $DIR_NAME_TEST"
  else
    echo "ERROR  : $VAR_NAME_TEST is set to non-existing location $DIR_NAME_TEST"
    ENVIRONMENT_ERRORS=1
    break
  fi

done


# ... just a little clean up (makes sence when this script is sourced)

unset DIR_NAME_TEST
unset VAR_NAME_TEST


# ... the following enables return codes while allowing to "source" this file
# ... (the use of "exit" would terminate the parent shell)

if [ $ENVIRONMENT_ERRORS -ne 0 ]; then
  echo " === test NOT passed === "
  false
else
  echo " === test PASSED (but check warnings!) === "
  true
fi
