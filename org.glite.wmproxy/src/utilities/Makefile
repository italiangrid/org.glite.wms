cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2 -Wall
CWFLAGS=
#-DWANT_NAMESPACES \
#CIFLAGS=-I. -I../server
CMFLAGS=-DHAVE_STRINGSTREAM

GLOBUS_FLAVOR=gcc32dbg
GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include \
	-I$(REPOSITORY)/org.glite.wms.purger/src \
	-I$(REPOSITORY)/org.glite.wms.common/src/utilities
	
EXTRALIBFLAGS = \
        $(REPOSITORY)/org.glite.wms.purger/build/src/.libs/libglite_wms_purger.al \
        $(REPOSITORY)/org.glite.wms.common/build/src/utilities/.libs/libglite_wms_quota.al

	
all:	utilities install


utilities: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) $(CIFLAGS) \
	        -o libglite_wms_wmproxy_utilities.so \
	        wmputils.cpp wmpexceptions.cpp wmpsignalhandler.cpp \
 		$(LIBS) $(EXTRALIBFLAGS)


install:
		echo "Installing lib ..."
		cp -f libglite_wms_wmproxy_utilities.so $(REPOSITORY)/stage/lib

clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
