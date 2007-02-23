# This is a "multishell" script.

# Try MyProxy commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of myproxy-* === "
echo ""

source `dirname $0`/command-version.sh myproxy-init		  --version || exit $?
source `dirname $0`/command-version.sh myproxy-info		  --version || exit $?
source `dirname $0`/command-version.sh myproxy-destroy		  --version || exit $?
source `dirname $0`/command-version.sh myproxy-server		  --version || exit $?
source `dirname $0`/command-version.sh myproxy-get-delegation	  --version || exit $?
source `dirname $0`/command-version.sh myproxy-change-pass-phrase --version || exit $?

echo " == all Ok == "
exit 0
