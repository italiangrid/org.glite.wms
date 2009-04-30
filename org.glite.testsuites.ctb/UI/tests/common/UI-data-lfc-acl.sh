#!/bin/sh
#
# Create a directory in LFC, list ACL, modify ACL, list ACL, delete directory.
#
# See comments in lfc-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lfc-tests-common.sh

echo ""
echo "    === lfc-acl test ===    "
echo ""

# ... define LFC_HOST and TEST_DIR

lfc_test_startup $@


# ... I. Create directory

echo ""; myecho "lfc-mkdir $TEST_DIR"; echo ""
lfc-mkdir $TEST_DIR || exit_failure

TEST_DIR_CREATED="yes"


# ... II. List ACL

echo ""; myecho "lfc-getacl $TEST_DIR"; echo ""
lfc-getacl $TEST_DIR || exit_failure


# ... III. Set ACL

NEWACL="group::r-x"

echo ""; myecho "lfc-setacl $NEWACL $TEST_DIR"; echo ""
lfc-setacl -m $NEWACL $TEST_DIR || exit_failure


# ... IV. List ACL

echo ""; myecho "lfc-getacl $TEST_DIR"; echo ""
OUTPUT=`lfc-getacl $TEST_DIR`

if [ $? -ne 0 ]; then
  echo "$OUTPUT"
  myecho "Could not get ACL for $TEST_DIR"
  exit_failure
fi

echo "$OUTPUT"

if grep -q "$NEWACL" <<<"$OUTPUT"; then
  echo ""
  myecho "ACL has been successfully set"
else
  myecho "Error: The new ACL does not seem to be set correctly"
  exit_failure
fi

# ... V. Remove the directory

echo ""; myecho "lfc-rm -r $TEST_DIR"; echo ""
lfc-rm -r $TEST_DIR || exit_failure

# ... Finish

exit_success
