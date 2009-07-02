#!/bin/sh

#
#bug #45167: Provide a CLI for group management
#

export GLITE_LOCATION="/opt/glite"

group=`date +%s`
echo "Using group: $group"

echo "Adding member1"
glite-transfer-group-addmember $group member1
if [ $? -ne 0 ]; then
  echo "Error adding group member"
  exit 1
fi

echo "Adding member2 and member3"
glite-transfer-group-addmember $group member2 member3
if [ $? -ne 0 ]; then
  echo "Error adding group member"
  exit 1
fi

echo "Listing grup members"
glite-transfer-group-list $group > out.txt
if [ $? -ne 0 ]; then
  echo "Error listing group"
  exit 1
fi

grep member1 out.txt  >>/dev/null
if [ $? -ne 0 ]; then
  echo "Added member not listed"
  exit 1
fi
grep member2 out.txt >>/dev/null
if [ $? -ne 0 ]; then
  echo "Added member not listed"
  exit 1
fi
grep member3 out.txt >>/dev/null
if [ $? -ne 0 ]; then
  echo "Added member not listed"
  exit 1
fi


echo "Removing member1"
glite-transfer-group-removemember $group member1
if [ $? -ne 0 ]; then
  echo "Error removing member"
  exit 1
fi

echo "Removing member2 and member3"
glite-transfer-group-removemember $group member2 member3
if [ $? -ne 0 ]; then
  echo "Error removing member"
  exit 1
fi

echo "Listing grup members"
glite-transfer-group-list $group > out.txt
if [ $? -ne 0 ]; then
  echo "Error listing group"
  exit 1
fi

grep member1 out.txt  >>/dev/null
if [ $? -eq 0 ]; then
  echo "Member1 should not exist anymore"
  exit 1
fi
grep member2 out.txt >>/dev/null
if [ $? -eq 0 ]; then
  echo "Member2 should not exist anymore"
  exit 1
fi
grep member2 out.txt >>/dev/null
if [ $? -eq 0 ]; then
  echo "Member2 should not exist anymore"
  exit 1
fi

echo "-TEST PASSED-"
exit 0

