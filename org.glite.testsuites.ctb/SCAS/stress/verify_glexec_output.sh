#!/bin/sh

function usage() {

  echo "Usage: ./verify_glexec_output.sh <mapped_user> <output file>"
  echo "    <mapped_user> Unix user that should be returned by the glexec test."
  echo "                  In case of pool accounts one can specify the prefix (without numbers)."
  echo "    <output file> The file containing the output of the glexec test"
}

if [ $# -ne 2 -o "x$1" == "--help" ]; then
  usage
  exit 1
fi

user=$1
file=$2

if [ ! -f $file ]; then
  echo "Error, the second argument is not a file"
  usage
  exit 1
fi

#output=`grep -e '[^$user]' $file`
output=`egrep "[^$user]" $file`

if [ "x$output" != "xSTART" ]; then
  echo $output >> out.txt
  echo "Undesirable output has been detected, look at it in out.txt"
  echo "TEST FAILED"
else
  echo "glexec output file is correct"
fi

exit 0
 
