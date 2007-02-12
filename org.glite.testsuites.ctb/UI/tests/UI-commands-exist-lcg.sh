# This is a "multishell" script.

# Test whether the LCG data management tools can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of lcg-* ===    "

source `dirname $0`/command-exist.sh lcg-cp  || exit $?
source `dirname $0`/command-exist.sh lcg-cr  || exit $?
source `dirname $0`/command-exist.sh lcg-del || exit $?
source `dirname $0`/command-exist.sh lcg-rep || exit $?
source `dirname $0`/command-exist.sh lcg-gt  || exit $?
source `dirname $0`/command-exist.sh lcg-sd  || exit $?

source `dirname $0`/command-exist.sh lcg-aa  || exit $?
source `dirname $0`/command-exist.sh lcg-ra  || exit $?
source `dirname $0`/command-exist.sh lcg-rf  || exit $?
source `dirname $0`/command-exist.sh lcg-uf  || exit $?
source `dirname $0`/command-exist.sh lcg-la  || exit $?
source `dirname $0`/command-exist.sh lcg-lg  || exit $?
source `dirname $0`/command-exist.sh lcg-lr  || exit $?

#source `dirname $0`/command-exist.sh  lcg-fetch          || exit $?
#source `dirname $0`/command-exist.sh  lcg-info		  || exit $?
#source `dirname $0`/command-exist.sh  lcg-infosites	  || exit $?
#source `dirname $0`/command-exist.sh  lcg-job-monitor	  || exit $?
#source `dirname $0`/command-exist.sh  lcg-job-status	  || exit $?
#source `dirname $0`/command-exist.sh  lcg-ManageSoftware || exit $?
#source `dirname $0`/command-exist.sh  lcg-ManageVOTag	  || exit $?
#source `dirname $0`/command-exist.sh  lcg-mon-wn	  || exit $?
#source `dirname $0`/command-exist.sh  lcg-replica-manager || exit $?
#source `dirname $0`/command-exist.sh  lcg-tags		  || exit $?
#source `dirname $0`/command-exist.sh  lcg-version        || exit $?
#source `dirname $0`/command-exist.sh  lcg-wn-os 	  || exit $?

echo "    ===  Ok  ===    "
