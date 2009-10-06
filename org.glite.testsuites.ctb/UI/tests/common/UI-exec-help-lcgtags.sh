# This is a "multishell" script.

# Try lcg-tags with --help

echo "    == Help test of lcg-tags === "
echo ""

#Put back this line when bug is fixed
#source `dirname $0`/command-help.sh lcg-tags --help || exit $?
source `dirname $0`/command-help.sh lcg-tags --help
if [ $? -eq 1 ]; then
  echo " == Expected failure: bug 53411 == "
  exit 0
else
  echo "==Unexpected success: bug 53411, update the test! =="
  exit 1
fi
