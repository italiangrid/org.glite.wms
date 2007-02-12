# This is a "multishell" script.

# Test whether MyProxy commands can be found by the OS

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of myproxy-* ===    "

source `dirname $0`/command-exist.sh myproxy-init		|| exit $?
source `dirname $0`/command-exist.sh myproxy-info		|| exit $?
source `dirname $0`/command-exist.sh myproxy-destroy		|| exit $?
source `dirname $0`/command-exist.sh myproxy-server		|| exit $?
source `dirname $0`/command-exist.sh myproxy-get-delegation	|| exit $?
source `dirname $0`/command-exist.sh myproxy-change-pass-phrase || exit $?

echo "    ===  Ok  ===    "
