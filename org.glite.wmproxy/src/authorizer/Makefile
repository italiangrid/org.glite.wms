cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32

GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-DALLOW_EMPTY_CREDENTIALS \
	-DLCMAPS_GSI_MODE \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include

LDFLAGS= \
	-L$(REPOSITORY)/stage/lib

all:	authorizer


authorizer: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I. \
			-I../server -I../utilities \
	        -o libglite_wms_wmproxy_authorizer.so \
	        wmpauthorizer.cpp \
 		$(LIBS) $(LDFLAGS)


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
