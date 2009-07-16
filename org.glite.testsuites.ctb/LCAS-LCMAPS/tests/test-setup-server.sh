#!/bin/bash
#definable variables
LCAS_CONF_DIR=/opt/glite/etc/lcas
TESTVOMS=dteam
VOMSCERT="/DC=ch/DC=cern/OU=computers/CN=lxbra2309.cern.ch"

#Different certificates
TESTCERT_OK="/C=CH/O=CERN/OU=GD/CN=Test user 100"
TESTCERT_BAN="/C=CH/O=CERN/OU=GD/CN=Test user 101"


PHASE=0
# This scirpt sets up a proper environment for the tests. 

usage() {
 echo
 echo "Script for setting up the LCAS test environment on the server."
 echo "The tests are run in three phases, with different configurations."
 echo "test-setup-server.sh --phase 1|2|3"
 echo
}


while [ $# -gt 0 ]
do
 case $1 in
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --phase | -phase | --p | -p ) PHASE=$2
  shift
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done

if [ $PHASE -lt 1 ] || [ $PHASE -gt 3 ] ; then
 usage
 exit 0
fi

echo \"${TESTCERT_BAN}\" > $LCAS_CONF_DIR/ban_users.db




if [ $PHASE -eq 1 ] ; then

 cat > $LCAS_CONF_DIR/lcas.db << EOF
# LCAS database/plugin list
#
# Format of each line:
# pluginname="<name/path of plugin>", pluginargs="<arguments>"
#
pluginname=lcas_userban.mod,pluginargs=ban_users.db
pluginname=lcas_voms.mod,pluginargs="-vomsdir /etc/grid-security/vomsdir/ -certdir /etc/grid-security/certificates/ -authfile /etc/grid-security/grid-mapfile -authformat simple -use_user_dn" 

EOF

 echo "Setup for phase 1 done"

elif [ $PHASE -eq 2 ] ; then
 cat > $LCAS_CONF_DIR/lcas.db << EOF
# LCAS database/plugin list
#
# Format of each line:
# pluginname="<name/path of plugin>", pluginargs="<arguments>"
#
pluginname=lcas_userban.mod,pluginargs=ban_users.db
pluginname=lcas_voms.mod,pluginargs="-vomsdir /etc/grid-security/vomsdir/ -certdir /etc/grid-security/certificates/ -authfile $LCAS_CONF_DIR/gaclfile -authformat gacl" 
pluginname=lcas_timeslots.mod,pluginargs=timeslots.db
EOF


 cat > $LCAS_CONF_DIR/timeslots.db << EOF
         *             *           *            *            *           *
EOF

 cat > $LCAS_CONF_DIR/gaclfile << EOF
<?xml version="1.0"?>
<gacl version="0.0.1">
<entry>
<voms-cred>
<voms>$VOMSCERT</voms>
<vo>$TESTVOMS</vo>
</voms-cred>
<allow><read/><write/></allow>
<deny><list/></deny>
</entry>
</gacl>
EOF

 echo "Setup for phase 2 done"

else

#PHASE 3

 cat > $LCAS_CONF_DIR/lcas.db << EOF
# LCAS database/plugin list
#
# Format of each line:
# pluginname="<name/path of plugin>", pluginargs="<arguments>"
#
pluginname=lcas_userban.mod,pluginargs=ban_users.db
pluginname=lcas_voms.mod,pluginargs="-vomsdir /etc/grid-security/vomsdir/ -certdir /etc/grid-security/certificates/ -authfile /etc/grid-security/grid-mapfile -authformat simple -use_user_dn" 
pluginname=lcas_timeslots.mod,pluginargs=timeslots.db
EOF

 #open at midnight
 cat > $LCAS_CONF_DIR/timeslots.db << EOF
         0-1             0-1           *         *            *           *
EOF
 echo "Setup for phase 3 done"
fi


