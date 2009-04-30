# This is a "multishell" script.

# Try gLite uberftp with -v

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of uberftp  ===     "
echo ""

source `dirname $0`/command-version.sh uberftp -v || exit $?

echo "    ===    seems Ok    ===    "
exit 0
