cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include \
	-I$(REPOSITORY)/org.glite.wms.purger/src \
	-I$(REPOSITORY)/org.glite.wms.common/src/utilities
	
EXTRALIBFLAGS = \
	-L$(REPOSITORY)/org.glite.wms.purger/build/src/libglite_wms_purger.la \
	-L$(REPOSITORY)/org.glite.wms.common/build/src/utilities/libglite_wms_quota.la
	
all:	utilities


utilities: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) $(EXTRALIBFLAGS) -I../server \
	        -o libglite_wms_wmproxy_utilities.so \
	        wmputils.cpp wmpexceptions.cpp\
 		$(LIBS)


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
