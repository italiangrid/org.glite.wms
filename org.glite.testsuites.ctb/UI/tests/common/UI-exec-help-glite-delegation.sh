# This is a "multishell" script.

# Try glite-delegation with -h

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    == Help test of glite-delegation-* === "
echo ""

source `dirname $0`/command-help.sh glite-delegation-info	  -h || exit $?
source `dirname $0`/command-help.sh glite-delegation-init	  -h || exit $?
source `dirname $0`/command-help.sh glite-delegation-destroy	  -h || exit $?

echo " == all Ok == "
exit 0

