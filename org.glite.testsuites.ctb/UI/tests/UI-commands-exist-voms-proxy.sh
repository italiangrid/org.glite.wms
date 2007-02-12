# This is a "multishell" script.

# Test whether the VOMS proxy commands can be found by the OS

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo "    === Existence test of voms-proxy-* ===    "

source `dirname $0`/command-exist.sh voms-proxy-init    || exit 1
source `dirname $0`/command-exist.sh voms-proxy-destroy || exit 1
source `dirname $0`/command-exist.sh voms-proxy-info    || exit 1
source `dirname $0`/command-exist.sh voms-proxy-list    || exit 1
source `dirname $0`/command-exist.sh voms-proxy-fake    || exit 1

echo "    ===  Ok  ===    "
