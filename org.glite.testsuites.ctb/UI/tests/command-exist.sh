# command_exist: multishell version
#
# Test whether $1 is a valid command name
#
# Example usage:
# ./command-exist.sh ls
# source command-exist.sh voms-proxy-init
# bash command-exist.sh myproxy-info
# tcsh command-exist.sh glite-job-submit
#
# 0 is returned if $1 is a valid command, 1 otherwise
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$


function usage() {
  echo "Usage $0 <command>"
  echo "  <command> a command to look for"
}


if [ $# -lt 1 ]; then
  echo "Wrong arguments"
  usage
  exit 1
fi

echo "testing if $1 exists in the system ..."

/usr/bin/which "$1" && (echo "... Yes"; true) || (echo "... Error! $1 not found!"; false)
