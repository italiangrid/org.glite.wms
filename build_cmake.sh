#!/bin/bash
cmake . -DOS=$1
make -C org.glite.wms.configuration install
make -C org.glite.wms.common install
make -C org.glite.wms.purger install
make -C org.glite.wms.core install
make RPM
