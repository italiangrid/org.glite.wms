cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2 -g -Wall
CWFLAGS=
CIFLAGS=
CMFLAGS=-DHAVE_STRINGSTREAM



GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr
GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32

BOOST=$(REPOSITORY)/repository/boost/1.32.0-slc3/rhel30_gcc32

REPLIBS=$(REPOSITORY)/stage/lib

BOOST_LIBS=-L$(BOOST)/lib -lboost_filesystem




COMMON_CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(OBJFLAGS) \
        -DWANT_NAMESPACES \
        -DALLOW_EMPTY_CREDENTIALS \
        -DLCMAPS_GSI_MODE \
	-DGLITE_WMS_WMPROXY_TOOLS \
	-I ../ \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/stage/include \
	-I$(BOOST)/include


COMMON_LDFLAGS=-L$(REPLIBS) \
-lgridsite \
-lssl$(FLAVOR_EXT) \
-lxml2


CFLAGS_MAP2GACL=$(COMMON_CFLAGS)
LDFLAGS_MAP2GACL=$(COMMON_LDFLAGS)


CFLAGS_GACLADMIN=$(COMMON_CFLAGS)
LDFLAGS_GACLADMIN=$(COMMON_LDFLAGS)

CFLAGS_CLEANPROXIES=$(COMMON_CFLAGS)
LDFLAGS_CLEANPROXIES=$(COMMON_LDFLAGS) \
$(BOOST_LIBS)

all: map2gacl gacladmin cleanproxies

map2gacl:
	$(CPP) $(CMFLAGS) $(CFLAGS_MAP2GACL)  $(LDFLAGS_MAP2GACL) \
		-o glite-wmp-mapfile2gacl \
		wmpgridmap2gacl.cpp ../authorizer/wmpgaclmanager.cpp ../authorizer/wmpauthorizer.cpp ../utilities/wmputils.cpp \
		log.cpp

gacladmin:
	$(CPP) $(CMFLAGS) $(CFLAGS_GACLADMIN)  $(LDFLAGS_GACLADMIN) \
		-o glite-wmp-gacladmin \
		wmpgacladmin.cpp ../authorizer/wmpgaclmanager.cpp ../authorizer/wmpauthorizer.cpp ../utilities/wmputils.cpp


cleanproxies:
	$(CPP) $(CMFLAGS) $(CFLAGS_CLEANPROXIES)  $(LDFLAGS_CLEANPROXIES) \
	-o glite-wmp-cleanproxies \
	wmpcleanproxies.cpp ../utilities/wmputils.cpp


install:


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
