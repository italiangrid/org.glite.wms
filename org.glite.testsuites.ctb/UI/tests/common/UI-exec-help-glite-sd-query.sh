# This is a "multishell" script.

# Try glite-sd-query with -h

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    == Help test of glite-sd-query === "
echo ""

source `dirname $0`/command-help.sh glite-sd-query	  -h || exit $?

echo " == all Ok == "
exit 0

