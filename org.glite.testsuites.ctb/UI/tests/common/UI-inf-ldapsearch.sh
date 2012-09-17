#!/bin/sh

# A set of ldapsearch requests with different attributes.
# Usage: UI-inf-lcg-info-ce.sh [-H giishost:port]
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

function myecho() {

echo ""
echo " =========== $@ =========== "
echo ""

}

function exit_failure() {

  echo ""
  echo " *** ldapsearch failed *** "
  echo " *** please make sure you are supplying correct GIIS with -H giishost:port *** "
  echo " *** test NOT passed *** "
  exit 1
}

echo ""
echo "    === ldapsearch test ===    "
echo ""

DEFAULT_GIIS=lcg-bdii.cern.ch:2170
SIZE_LIMIT=3

if [ "$1" == "-H" ] && [ -n "$2" ]; then
  GIIS=ldap://$2
  shift
  shift
else
  echo " ldapsearch test: no GIIS chosen ! - Have to use default !"
  GIIS=ldap://$DEFAULT_GIIS
fi

echo " ldapsearch test: will query GIIS $GIIS"
echo " ldapsearch test: will display first $SIZE_LIMIT results only"

# ... ask lcg-info for a selection of "interesting" CE attributes

myecho "I. listing selected attributes of GlueCETop objects"

ldapsearch -x -z $SIZE_LIMIT -H $GIIS  -b "mds-vo-name=local, o=grid" 'objectclass=GlueCETop' \
           GlueVOViewLocalID GlueCEStateRunningJobs GlueCEStateWaitingJobs GlueCEInfoDefaultSE || [ $? -eq 4 ] || exit_failure

myecho "II. listing selected attributes of GlueCESEBindGroup objects"

ldapsearch -x -H $GIIS -z $SIZE_LIMIT -b "mds-vo-name=local, o=grid" 'objectclass=GlueCESEBindGroup' \
           GlueCESEBindGroupCEUniqueID GlueCESEBindGroupSEUniqueID || [ $? -eq 4 ] || exit_failure

myecho "III. listing selected attributes of GlueCESEBind objects"

ldapsearch -x -H $GIIS -z $SIZE_LIMIT -b "mds-vo-name=local, o=grid" 'objectclass=GlueCESEBind' \
           GlueCESEBindSEUniqueID GlueCESEBindCEAccesspoint GlueCESEBindCEUniqueID GlueCESEBindMountInfo || [ $? -eq 4 ] || exit_failure

myecho "IV. listing selected attributes of GlueClusterTop objects"

ldapsearch -x -H $GIIS -z $SIZE_LIMIT -b "mds-vo-name=local, o=grid" 'objectclass=GlueClusterTop' \
           GlueClusterService GlueHostOperatingSystemName GlueHostOperatingSystemRelease GlueHostOperatingSystemVersion \
	   GlueHostProcessorModel GlueHostProcessorClockSpeed GlueHostProcessorVendor || [ $? -eq 4 ] || exit_failure

myecho "V. listing selected attributes of GlueSite objects"

ldapsearch -x -H $GIIS -z $SIZE_LIMIT -b "mds-vo-name=local, o=grid" \
           'objectclass=GlueSite' GlueSiteLocation GlueSiteWeb GlueSiteSysAdminContact || [ $? -eq 4 ] || exit_failure

echo ""
echo "    === test PASSED === "
exit 0
