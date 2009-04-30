# This is a "multishell" script.

# Try gLite lcg-* with -v

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of lcg--*  ===     "
echo ""

source `dirname $0`/command-version.sh lcg-cp --version || exit $?
source `dirname $0`/command-version.sh lcg-cr --version || exit $?
source `dirname $0`/command-version.sh lcg-del --version || exit $?
source `dirname $0`/command-version.sh lcg-rep --version || exit $?
source `dirname $0`/command-version.sh lcg-gt --version || exit $?
source `dirname $0`/command-version.sh lcg-sd --version || exit $?

source `dirname $0`/command-version.sh lcg-aa --version || exit $?
source `dirname $0`/command-version.sh lcg-ra --version || exit $?
source `dirname $0`/command-version.sh lcg-rf --version || exit $?
source `dirname $0`/command-version.sh lcg-uf --version || exit $?
source `dirname $0`/command-version.sh lcg-la --version || exit $?
source `dirname $0`/command-version.sh lcg-lg --version || exit $?
source `dirname $0`/command-version.sh lcg-lr --version || exit $?

echo "    ===    seems Ok    ===    "
exit 0
