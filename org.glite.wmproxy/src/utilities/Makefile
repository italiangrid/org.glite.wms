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
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include
	
all:	utilities


utilities: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I../server \
	        -o libglite_wms_wmproxy_utilities.so wmputils.cpp \
 		$(LIBS)


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
