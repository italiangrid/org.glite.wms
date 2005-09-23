cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM

CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I$(REPOSITORY)/stage/include \
	-I$(CLASSAD)/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include


all:	glitePipe gliteWMPCommands install

glitePipe:	
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I$(CLASSAD)/include \
		-I.. -DSHOW_EXECUTION \
		-o libglite_wms_wmproxy_pipe.so MatchingPipe.cpp MatchingPipe_nb.cpp \
		$(LIBS)

gliteWMPCommands: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I..\
		-I../../../org.glite.wms.purger/src -DSHOW_EXECUTION \
		-o libglite_wms_wmproxy_commands.so \
		Command.cpp ExecuteFunction.cpp \
		ForwardRequest.cpp listjobmatch.cpp \
		listfiles.cpp CommandFactoryServerImpl.cpp \
		$(LIBS)


install:
		echo "Installing libs ..."
		cp -f libglite_wms_wmproxy_commands.so $(REPOSITORY)/stage/lib
		cp -f libglite_wms_wmproxy_pipe.so $(REPOSITORY)/stage/lib



clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
