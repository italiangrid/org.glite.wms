# This is a "multishell" script.

# Try lcg-info with --help

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    == Help test of lcg-info === "
echo ""

source `dirname $0`/command-help.sh lcg-info	  --help || exit $?

echo " == all Ok == "
exit 0

