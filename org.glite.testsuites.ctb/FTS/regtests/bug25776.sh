#!/bin/sh

#bug #25776: /opt/glite/etc/glite-data-transfer-agents.d/*.properties.xml - incorrect files permissions

#To be run as root on the FTS host

for number in `stat -c %a /opt/glite/etc/glite-data-transfer-agents.d/*.xml`;
do
  if [ $number -ne "600" ]; then
    echo "BUG PRESENT"
    exit 1
  fi
done
echo "BUG FIXED"
exit 0


