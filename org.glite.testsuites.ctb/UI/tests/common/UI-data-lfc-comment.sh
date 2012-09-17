#!/bin/sh
#
# Create a directory in LFC, set its comment, list, delete comment, delete the directory.
#
# See comments in lfc-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lfc-tests-common.sh

echo ""
echo "    === lfc-comment test ===    "
echo ""

# ... define LFC_HOME and TEST_DIR

lfc_test_startup $@


# ... I. Create directory

echo ""; myecho "lfc-mkdir $TEST_DIR"; echo ""
lfc-mkdir $TEST_DIR || exit_failure

TEST_DIR_CREATED="yes"


# ... II. Set comment

COMMENT="very interesting comment"

echo ""; myecho "lfc-setcomment $TEST_DIR \"$COMMENT\""; echo ""
lfc-setcomment $TEST_DIR "$COMMENT" || exit_failure


# ... III. List comment

echo ""; myecho "lfc-ls -d --comment $TEST_DIR"; echo ""
OUTPUT=`lfc-ls -d --comment $TEST_DIR`

if [ $? -ne 0 ]; then
  echo "$OUTPUT"
  myecho "Problem listing directory $TEST_DIR"
  exit_failure
fi

echo "$OUTPUT"

if grep -q "$COMMENT" <<<"$OUTPUT"; then
  echo ""
  myecho "Comment has been successfully set"
else
  myecho "Error: The comment does not seem to set correctly"
  exit_failure
fi


# ... IV. Delete comment

echo ""; myecho "lfc-delcomment $TEST_DIR"; echo ""
lfc-delcomment $TEST_DIR || exit_failure

echo ""; myecho "lfc-ls -d --comment $TEST_DIR"; echo ""
lfc-ls -d --comment $TEST_DIR


# ... V. Remove directory

echo ""; myecho "lfc-rm -r $TEST_DIR"; echo ""
lfc-rm -r $TEST_DIR || exit_failure

# ... Finish

exit_success
