# This is a "multishell" script.

# Test whether the grid-proxy-* and grid-cert-info commands exist in the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of grid-proxy-* and grid-cert-info ===    "

source `dirname $0`/command-exist.sh grid-proxy-init    || exit $?
source `dirname $0`/command-exist.sh grid-proxy-info    || exit $?
source `dirname $0`/command-exist.sh grid-proxy-destroy || exit $?
source `dirname $0`/command-exist.sh grid-cert-info     || exit $?

echo "    ===  Ok  ===    "
