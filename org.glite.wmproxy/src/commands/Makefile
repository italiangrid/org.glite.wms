cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32
CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32

GLOBUS_FLAVOR=gcc32dbg
#GLOBUS_FLAVOR_THREADS=gcc32dbgpthr

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/stage/include \
	-I$(CLASSAD)/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include


all:	glitePipe gliteWMPCommands

glitePipe:	
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I$(CLASSAD)/include \
			-I../utilities -I. -DSHOW_EXECUTION \
		    -o libglitePipe.so MatchingPipe.cpp MatchingPipeFiller.cpp \
		$(LIBS)

gliteWMPCommands: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I../server -I../utilities\
		-I../../../org.glite.wms.purger/src -DSHOW_EXECUTION \
	        -o libgliteWMPCommands.so common.cpp \
	        Command.cpp ExecuteFunction.cpp \
	        ForwardRequest.cpp \
	        listjobmatch.cpp listfiles.cpp \
		CommandFactoryServerImpl.cpp \
		DagCommandFactoryServerImpl.cpp \
 		$(LIBS)


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
