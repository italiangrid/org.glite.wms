# This is a "multishell" script.

# Try gLite FTS commands with -V

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of glite-transfer-*  ===     "
echo ""

source `dirname $0`/command-version.sh glite-transfer-submit -V || exit $?
source `dirname $0`/command-version.sh glite-transfer-status -V || exit $?
source `dirname $0`/command-version.sh glite-transfer-list   -V || exit $?
source `dirname $0`/command-version.sh glite-transfer-cancel -V || exit $?

source `dirname $0`/command-version.sh glite-transfer-channel-add    -V || exit $?
source `dirname $0`/command-version.sh glite-transfer-channel-list   -V || exit $?
source `dirname $0`/command-version.sh glite-transfer-channel-set    -V || exit $?
source `dirname $0`/command-version.sh glite-transfer-channel-signal -V || exit $?

echo "    ===    seems Ok    ===    "
exit 0
