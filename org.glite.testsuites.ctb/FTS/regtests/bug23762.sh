#!/bin/sh

#FTS: info provider script does not check sqlplus return code

#The script has to be executed as root on the FTS host.

DBFILE="/opt/glite/var/glite-data-transfer-fts-info-provider.dbparams"
tmpfile=out.txt
#backup file
cp -f $DBFILE ${DBFILE}.orig

#change the password to access the DB
sed "s/DBPASS=/DBPASS=x/" $DBFILE > $tmpfile
cp -f $tmpfile $DBFILE

/opt/glite/sbin/glite-data-transfer-fts-info-provider --bdii --channels --site cert-tb-cern  --vo atlas --vo alice --vo lhcb --vo cms --vo dteam --vo ops --vo org.glite.voms-test --hostalias vtb-generic-88.cern.ch >>/dev/null
if [ $? -ne 0 ]; then
  echo "BUG FIXED"
  cp -f ${DBFILE}.orig $DBFILE
  rm -f $DBFILE.orig
  exit 0
else
  echo "BUG PRESENT"
  cp -f ${DBFILE}.orig $DBFILE
  rm -f $DBFILE.orig
  exit 1
fi
