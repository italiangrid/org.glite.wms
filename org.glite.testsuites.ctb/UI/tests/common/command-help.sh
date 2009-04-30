# command-help: multishell script
# based on the command-version script
#
# Try to run '$@' where $1 is a command name and $2 is an option and check error code.
#
# Example usage:
# ./command-help.sh ls
# source command-help.sh lcg-info  --help
#
# 0 is returned if $1 $2 returns zero, 1 otherwise
#
# Author: Kalle Happonen <kalle.happonen@cern.ch>
# Version info: $Id$

echo "testing $1 $2 ..."

IFS=""
out=`$1 $2`  && (echo $out 2>/dev/null |head -n2 ; echo ""; unset IFS;true) || (echo "... Error! Problem getting help for $1"; unset IFS; false)
