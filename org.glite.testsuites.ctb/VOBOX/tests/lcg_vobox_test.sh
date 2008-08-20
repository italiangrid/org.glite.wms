#!/bin/sh
#
# Wrapper script that calls the perl script lcg_vobox_test.pl
# that esecutes the following tests
# 1 check that the proxy renewal service is running
# 2 check the proxy registration
# 3 check that the machine is properly registered in the myproxy server
# 4 check that the machine is running its proxy
# 5 check the duration of the delegation proxy
# 6 check that the access to the software area is propery defined


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

