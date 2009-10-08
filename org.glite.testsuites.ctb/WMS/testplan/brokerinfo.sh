#!/bin/sh

echo "glite-brokerinfo getCE: `glite-brokerinfo getCE`"
echo "glite-brokerinfo getDataAccessProtocol: `glite-brokerinfo getDataAccessProtocol`"
protocol1=`glite-brokerinfo getDataAccessProtocol|head -1`
if [ -z $protocol1 ]; then
	echo "warning: no protocol returned"
	exit 1
fi
t1=`glite-brokerinfo getInputData`
echo "glite-brokerinfo getInputData: $t1"
if [ -z $t1 ]; then
	exit 1
fi
t2=`glite-brokerinfo getSEs`
echo "glite-brokerinfo getSEs: $t2`
if [ -z $t2 ]; then
	exit 1
fi
t3=`glite-brokerinfo getCloseSEs`
echo "glite-brokerinfo getCloseSEs: $t3"
if [ -z $t3 ]; then
	exit 1
fi
SE1=`glite-brokerinfo getCloseSEs|head -1`
if [ -n $SE1 ]; then
	echo "glite-brokerinfo getSEMountPoint $SE1: `glite-brokerinfo getSEMountPoint $SE1`"
	echo "glite-brokerinfo getSEFreeSpace $SE1: `glite-brokerinfo getSEFreeSpace $SE1`"
	echo "glite-brokerinfo getSEProtocols $SE1: `glite-brokerinfo getSEProtocols $SE1`"

	#glite-brokerinfo getLFN2SFN <LFN>
	if [ -n $protocol1 -a -n $SE1 ]; then
		echo "glite-brokerinfo getSEPort $SE1 $protocol1: `glite-brokerinfo getSEPort $SE1 $protocol1`"
	fi
else
	exit 1
fi
echo "getVirtualOrganization: `glite-brokerinfo getVirtualOrganization`"
# copy brokerinfo back
echo $GLITE_WMS_RB_BROKERINFO
exit 0
