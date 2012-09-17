# command-version: multishell script
#
# Try to run '$@' where $1 is a command name and $2 is an option and check error code.
#
# Example usage:
# ./command-version.sh ls
# source command-version.sh voms-proxy-init --version
# bash command-version.sh grid-proxy-init -version
# tcsh command-version.sh glite-transfer-list -V
#
# 0 is returned if $1 $2 returns zero, 1 otherwise
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "testing $1 $2 ..."

$1 $2 && (echo ""; true) || (echo "... Error! Problem getting version of $1"; false)
