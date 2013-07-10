#!/bin/bash
cmake . -DOS=$1
cd org.glite.wms.configuration
make install
cd -
cd org.glite.wms.common
make install
cd -
cd org.glite.wms.purger
make install
cd -
make RPM

