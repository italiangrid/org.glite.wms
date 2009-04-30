# This is a "multishell" script.

# Try grid-proxy-init/info/destroy and grid-cert-info with -version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    == Version test of grid-proxy-* and grid-cert-info === "
echo ""

source `dirname $0`/command-version.sh grid-proxy-init	  -version || exit $?
source `dirname $0`/command-version.sh grid-proxy-info	  -version || exit $?
source `dirname $0`/command-version.sh grid-proxy-destroy -version || exit $?
source `dirname $0`/command-version.sh grid-cert-info	  -version || exit $?

echo " == all Ok == "
exit 0

