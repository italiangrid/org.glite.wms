#!/bin/tcsh

# This test helps to reveal common problems with environment variables,
# such as wrong entries in $PATH and "similar" variables.
# A failure is returned if a "single-entry" variable pointing to non-existing directory is found.
# For colon-separated lists, such as $PATH, only a warning will be shown.
#
# Usage: 
# UI-environment.csh - test environment seen from a tcsh script (/etc/csh.cshrc and ~/.tcshrc or ~/.cshrc are executed)
# or
# source UI-environment.csh - test current shell environment (from csh/tcsh only)
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo " === tcsh environment test === "

# ... Part I: colon-separated lists

foreach VAR_NAME_TEST ( PATH LD_LIBRARY_PATH PYTHONPATH PERLLIB PERL5LIB MANPATH )

  echo " --- testing $VAR_NAME_TEST --- "

  foreach DIR_NAME_TEST ( `printenv $VAR_NAME_TEST | sed 's/:/\n/g' | sort` )

    if ( -d "$DIR_NAME_TEST" ) then
      echo "info   : $VAR_NAME_TEST has $DIR_NAME_TEST"
    else
      echo "WARNING: $VAR_NAME_TEST lists a non-existing location $DIR_NAME_TEST"
    endif

  end
end 

# ... Part II: simple directory names

foreach VAR_NAME_TEST ( JAVA_HOME JAVA_INSTALL_PATH )

  echo " --- testing $VAR_NAME_TEST --- "

  set DIR_NAME_TEST=`printenv $VAR_NAME_TEST`
  
  if ( -z "$DIR_NAME_TEST" ) then
    echo "info    : $VAR_NAME_TEST is not defined"
  else if ( -d "$DIR_NAME_TEST" ) then
    echo "info   : $VAR_NAME_TEST is set to $DIR_NAME_TEST"
  else
      echo "ERROR  : $VAR_NAME_TEST is set to non-existing location $DIR_NAME_TEST"
      echo " === test NOT passed === "
      exit 1
  endif
end

# ... clean up

unset VAR_NAME_TEST
unset DIR_NAME_TEST

echo " === test PASSED (but check warnings!) === "
exit 0
