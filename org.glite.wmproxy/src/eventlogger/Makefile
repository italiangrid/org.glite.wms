cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../.. -I../
CMFLAGS=-DHAVE_STRINGSTREAM

REPOSITORY=$(HOME)/Repository/cern
REPLIBS=$(REPOSITORY)/stage/lib
INTERFACE=$(REPOSITORY)/org.glite.wms.wmproxy-interface/interface

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32
CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32
ARES=$(REPOSITORY)/repository/ares/1.1.1/rhel30_gcc32
EXPAT=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32

GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr
GLOBUS_SSL_THR_LIBS = -lssl_gcc32dbgpthr -lcrypto_gcc32dbgpthr
OBGLOBUS_GSS_THR_LIBS = -lglobus_gssapi_gsi_gcc32dbgpthr -lglobus_gss_assist_gcc32dbgpthr
GLOBUS_COMMON_THR_LIBS = -lglobus_common_gcc32dbgpthr
GLOBUS_SSL_LIBS = -lssl_gcc32dbg -lcrypto_gcc32dbg
GLOBUS_GSS_LIBS = -lglobus_gssapi_gsi_gcc32dbg -lglobus_gss_assist_gcc32dbg
GLOBUS_COMMON_LIBS = -lglobus_common_gcc32dbg


CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(ARES)/include \
	-I$(REPOSITORY)/stage/include \
	-I$(CLASSAD)/include \
	-I$(BOOST)/include/boost \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include

all:	eventlogger 

eventlogger:	
		@echo "Compiling eventlogger..."
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I$(CLASSAD)/include -I. -DSHOW_EXECUTION \
		    -o libglite_wms_wmproxy_eventlogger.so wmpeventlogger.cpp 
		$(LIBS)

clean:
	@echo Removing
	@rm libglite_wms_wmproxy_eventlogger.so
