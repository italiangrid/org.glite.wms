cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../.. -I../
CMFLAGS=-DHAVE_STRINGSTREAM

CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I$(CLASSAD)/include \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include

all:	eventlogger 

eventlogger:	
		@echo "Compiling eventlogger..."
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I. -DSHOW_EXECUTION \
		    -o libglite_wms_wmproxy_eventlogger.so wmpeventlogger.cpp 
		$(LIBS)

clean:
	@echo Removing
	@rm libglite_wms_wmproxy_eventlogger.so
