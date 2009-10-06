# This is a "multishell" script.

# Try lcg-ManageVOTag  with --help

echo "    == Help test of lcg-ManageVOTag === "
echo ""

#original test line
#source `dirname $0`/command-help.sh lcg-ManageVOTag	  --help || exit $?

SL=`cat /etc/redhat-release |  awk -F'release ' '{print $2}' | cut -c 1`
if [ $SL == "5" ]; then
  source `dirname $0`/command-help.sh lcg-ManageVOTag --help
  if [ $? -eq 1 ]; then
    echo " == Expected failure: bug 53516 == "
    exit 0
  else
    echo "==Unexpected success: bug 53516, update the test! =="
    exit 1
  fi
else
  source `dirname $0`/command-help.sh lcg-ManageVOTag --help || exit $?
  echo "== TEST PASSED =="
fi



