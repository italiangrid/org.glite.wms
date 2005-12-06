cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2 -Wall
CWFLAGS=
CIFLAGS=-I../.. -I../
CMFLAGS=-DHAVE_STRINGSTREAM -DGLITE_WMS_HAVE_LBPROXY

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32
GLOBUS_FLAVOR=gcc32dbg

CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(CLASSAD)/include \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include

all:	eventlogger install 

eventlogger:	
		@echo "Compiling eventlogger..."
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I. -DSHOW_EXECUTION \
		    -o libglite_wms_wmproxy_eventlogger.so wmpeventlogger.cpp wmpexpdagad.cpp
		$(LIBS)

install:
		echo "Installing lib ..."
		cp -f libglite_wms_wmproxy_eventlogger.so $(REPOSITORY)/stage/lib


clean:
	@echo Removing
	@rm libglite_wms_wmproxy_eventlogger.so
