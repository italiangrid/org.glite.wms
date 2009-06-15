#!/bin/bash


. lib.sh

# get good SE hostname
VO=`vo`
#SE=`get_default_se $VO` || SE=`get_default_se ops` || SE='srm.cern.ch'
SE=`get_default_se $VO` || SE=`get_default_se ops` || SE='lxbra1910.cern.ch'

if [ ! "$LCG_GFAL_INFOSYS" ]; then
    echo "LCG_GFAL_INFOSYS not set"
    exit 1
fi
BDII=${LCG_GFAL_INFOSYS%%:*}

TIME1=`date +%s`
# do the search
SE_NAME=`ldapsearch -H ldap://$LCG_GFAL_INFOSYS -x -b o=grid -LLL \
        "(&(objectclass=GlueSE)(GlueSEUniqueID=$SE))" GlueSEName | \
        sed -n '/^GlueSEName: */s///p'`
TIME2=`date +%s`
DIFF=$[$TIME2-$TIME1]

# check results
if [ ! "$SE_NAME" ]; then
  echo "$BDII: GlueSEName for $SE not found in ${DIFF}s"
  exit 1
fi

# check response time
test "$DIFF" -ge 15 && exit 2
test "$DIFF" -ge 30 && exit 2
test "$DIFF" -ge 60 && exit 1
echo "$BDII returned $SE_NAME in ${DIFF}s"
echo "-TEST PASSED-"
exit 0

