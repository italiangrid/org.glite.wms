#!/bin/sh

showUsage ()
{
 echo "                                           "
 echo "Usage: drop_user_objects.sh <username>     "
 echo "                                           "
}

if [ -z $1 ]; then
  echo "A user name must be provided"
  showUsage
  exit 1
else
  username=$1
fi

echo "Please enter the FTS DB password for $username"
read pass

sqlplus $username/$pass@"(DESCRIPTION=(AD
DRESS=(PROTOCOL=TCP)(HOST=int11r2-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)
(HOST=int11r1-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DED
ICATED)(SERVICE_NAME=lcg_fts_int11r.cern.ch)))" @./drop_user_objects.sql

exit 0
