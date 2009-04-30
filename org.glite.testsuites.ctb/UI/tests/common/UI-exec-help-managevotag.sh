# This is a "multishell" script.

# Try lcg-ManageVOTag  with --help

echo "    == Help test of lcg-ManageVOTag === "
echo ""

source `dirname $0`/command-help.sh lcg-ManageVOTag	  --help || exit $?

echo " == all Ok == "
exit 0

