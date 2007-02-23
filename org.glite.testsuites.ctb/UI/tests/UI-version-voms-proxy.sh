# This is a "multishell" script.

# Try VOMS proxy commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of voms-proxy-* ==="
echo ""

source `dirname $0`/command-version.sh voms-proxy-init	  --version || exit $?
source `dirname $0`/command-version.sh voms-proxy-destroy --version || exit $?
source `dirname $0`/command-version.sh voms-proxy-info	  --version || exit $?
source `dirname $0`/command-version.sh voms-proxy-list	  --version || exit $?
source `dirname $0`/command-version.sh voms-proxy-fake	  --version || exit $?

echo " == all Ok == "
exit 0
