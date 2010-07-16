# This is a "multishell" script.
# hmm - this script had the wrong bug hard coded in (was 53411)

# Try lcg-tags with --help

echo "    == Help test of lcg-tags === "
echo ""

#Put back this line when bug is fixed
#source `dirname $0`/command-help.sh lcg-tags --help || exit $?
source `dirname $0`/command-help.sh lcg-tags --help
if [ $? -eq 1 ]; then
  echo " == Expected failure: bug 56603 == "
  exit 0
else
  echo "==Unexpected success: bug 56603, update the test! =="
  exit 1
fi

