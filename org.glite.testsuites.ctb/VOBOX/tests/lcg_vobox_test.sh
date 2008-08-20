#!/bin/sh
#
# Wrapper script that calls the perl script lcg_vobox_test.pl
# that esecutes the following tests
# - check that the proxy renewal service is running
# - check the proxy registration
# - check that the machine is properly registered in the myproxy server
# - check that the machine is running its proxy
# - check the duration of the delegation proxy
# - check that the access to the software area is propery defined
# - check the WMS status
# - check the access to the local BDII sites and the SEs published there


showUsage ()
{
 echo "                                           "
 echo "Usage:  lcg_vobox_test.sh --node <margument1>        "
 echo "Arguments:                                                                           "
 echo "  mandatory:                                      "
 echo "                                           "
 echo "          --node   <nodename>                          "
 echo "              node name to label the tests.                      "
 echo "                                           "
}

#######################
#Parsing the arguments#
#######################
if [ -z "$1" ] || [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "--help" ]; then
  showUsage
  exit 2
fi

until [ -z "$1" ]
do
  case "$1" in
     --node)
           if [ -z "$2" ]; then
                shift 1
       else
                NODE=$2
                shift 2
           fi
    ;;

    *)
        showUsage
        exit 2
    ;;
  esac
done

#####################
# Exporting variables
##################### 



###########################
# Calling the perl script #
###########################

rm -rf err.txt
rm -rf /tmp/$NODE/*.result


if [ ! -e "lcg_vobox_test.pl" ]; then
  echo "The perl script lcg_vobox_test.pl must be in the current directory"
  echo "Test FAILED"
  exit 1
fi

perl lcg_vobox_test.pl -node $NODE 2>err.txt 1>/dev/null

if [ -s err.txt ]; then
  echo "Test FAILED"
  cat out.txt
  echo 
  grep --no-filename Failed /tmp/$NODE/*.result
  exit 1
fi

echo "Test PASSED"
exit 0

