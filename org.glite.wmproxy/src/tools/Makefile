cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2 -Wall
CWFLAGS=
CIFLAGS=
CMFLAGS=-DHAVE_STRINGSTREAM



GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32

ARES=$(REPOSITORY)/repository/ares/1.1.1/rhel30_gcc32
EXPAT=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32
CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.32.0-slc3/rhel30_gcc32

REPLIBS=$(REPOSITORY)/stage/lib

BOOST_LIBS=-lboost_fs -lboost_thread -lboost_regex
CLASSAD_LIBS=-lclassad -lclassad_dl 
ARES_LIBS=-lares
GLITE_LIBS=-lgridsite \
        -lglite_wms_logger \
        -lglite_wmsutils_cjobid \
        -lglite_lb_clientpp_gcc32 \
        -lglite_wmsutils_jobid \
        -lglite_wms_wmproxy_utilities \
        -lglite_wmsutils_exception \
        -lglite_wms_wmproxy_authorizer \
        -lglite_lb_common_gcc32 \
        -lglite_wms_util \
        -lglite_wms_conf 



CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(OBJFLAGS) \
        -DWANT_NAMESPACES \
        -DALLOW_EMPTY_CREDENTIALS \
        -DLCMAPS_GSI_MODE \
	-DWMPROXY_TOOLS \
	-I ../ \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/stage/include \
	-I$(BOOST)/include \
	-I/usr/include


LDFLAGS=-L$(REPOSITORY)/stage/lib \
-lgridsite \
-lssl$(FLAVOR_EXT) \
-lxml2


all: map2gacl


map2gacl:
	$(CPP) -g -Wall -DGLITE_GACL_TOOLS \
		$(CMFLAGS) $(CFLAGS)  $(LDFLAGS) \
		-o mapfile2gacl \
		mapfile2gacl.cpp ../authorizer/wmpgaclmanager.cpp ../authorizer/wmpauthorizer.cpp ../utilities/wmputils.cpp \
		log.cpp


install:


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
