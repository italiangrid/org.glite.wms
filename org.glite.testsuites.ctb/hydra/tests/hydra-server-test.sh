#!/bin/bash
#
# Tests that the required hydra services are running on the server
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

. $(dirname $0)/functions.sh

if [ $# -ne 0 ] ; then
  usage $0
  if [ $1 = "--help" ] ; then
    my_exit $SUCCESS
  else
    my_exit $ERROR
  fi
fi

setup server

###############################
###### Test for services ######
###############################

# Test if MySQL is running (it can be on another host)
TOMCAT_CFG_FILE=`ls  /etc/tomcat5/Catalina/localhost/*glite-data-hydra-service.xml | head -n1`

if [ -e "$TOMCAT_CFG_FILE" ] ; then
  MYSQL_LOCATION=`grep jdbc:mysql $TOMCAT_CFG_FILE | cut -f3 -d /`
  MYSQL_HOST=`echo "$MYSQL_LOCATION" | cut -f1 -d :`
  MYSQL_PORT=`echo "$MYSQL_LOCATION" | cut -f2 -d :`
  MYSQL_USER=`grep username $TOMCAT_CFG_FILE | cut -f 2 -d \"`
  MYSQL_PASSWD=`grep password $TOMCAT_CFG_FILE | cut -f 2 -d \"`
  MYSQL_DB=`grep jdbc:mysql $TOMCAT_CFG_FILE | cut -f4 -d / |cut -f 1 -d "\""`

  mysql -u $MYSQL_USER -p$MYSQL_PASSWD -h "$MYSQL_HOST" -P "$MYSQL_PORT"  -D $MYSQL_DB -e "select * from eds" >/dev/null

  if [ $? -ne 0 ] ; then
    echo "Cannot contact the specified MySQL instance."
    my_exit $ERROR
  fi

  echo "MySQL is running"
else
  echo "MySQL info not found in local config file, checking localhost"
  /sbin/service mysqld status |grep running >/dev/null

  if [ $? -ne 0 ] ; then
    echo "MySQL not running"
    my_exit $ERROR
  fi
  echo "MySQL is running"
fi

# Test if Tomcat is running
/sbin/service tomcat5 status |grep running >/dev/null

if [ $? -ne 0 ] ; then
  echo "Tomcat is required and not running. Exiting."
  my_exit $ERROR
fi

echo "Tomcat is running"

# Test that the bdii service is running
/sbin/service bdii status |grep OK >/dev/null

if [ $? -ne 0 ] ; then
  echo "BDII is required and not running. Exiting."
  my_exit $ERROR
fi

echo "BDII is running"

###############################
############ Done #############
###############################

echo "Tests completed"
my_exit $SUCCESS
