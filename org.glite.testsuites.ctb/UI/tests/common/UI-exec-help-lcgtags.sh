# This is a "multishell" script.

# Try lcg-tags with --help

echo "    == Help test of lcg-tags === "
echo ""

source `dirname $0`/command-help.sh lcg-tags --help || exit $?

echo " == all Ok == "
exit 0

