#!/bin/bash
OS=`uname -a | awk '{print $2}' | awk -F"-" '{print $1}'`
cmake . -DOS=$OS
make -C org.glite.wms.configuration install
make -C org.glite.wms.common install
make -C org.glite.wms.purger install
make -C org.glite.wms.core install
make -C org.glite.wms.jobsubmission install
make -C org.glite.wms.interface install
make -C org.glite.wms.ice install
make -C org.glite.wms.nagios nagios
make -C org.glite.wms.brokerinfo-access install
make RPM
