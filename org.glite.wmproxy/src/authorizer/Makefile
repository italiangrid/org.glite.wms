cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2 -Wall
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM



GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32
BOOST_LOCATION=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32
EXPAT_LOCATION=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32/

ARES=$(REPOSITORY)/repository/ares/1.1.1/rhel30_gcc32
EXPAT=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32
CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32

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

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DALLOW_EMPTY_CREDENTIALS \
	-DLCMAPS_GSI_MODE \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/org.glite.wms.wmproxy/src \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include

LDFLAGS= \
	-L$(GLOBUS_LOCATION)/lib


ADMIN_CFLAGS=$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(OBJFLAGS) \
        -DWANT_NAMESPACES \
        -DALLOW_EMPTY_CREDENTIALS \
        -DLCMAPS_GSI_MODE \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(REPOSITORY)/stage/include \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include


ADMIN_LDFLAGS=-L$(REPOSITORY)/stage/lib \
-lgridsite \
-lssl$(FLAVOR_EXT) \
-lxml2


all:	authorizer install gacladmin

gacl :
		$(CPP) -shared	$(CFLAGS) $(CMFLAGS) -I. \
		-o	wmpgaclmanager.o \
		wmpgaclmanager.cpp

authorizer:
		$(CPP) -shared \
		$(CFLAGS) $(CMFLAGS) -I. \
	        -o libglite_wms_wmproxy_authorizer.so \
	        wmpauthorizer.cpp wmpvomsauthz.cpp wmpgaclmanager.cpp


gacladmin:
	$(CPP) -g -Wall -DWMPROXY_TOOLS \
		$(CMFLAGS) $(ADMIN_CFLAGS)  $(ADMIN_LDFLAGS) \
		-o wmp-gacl-admin \
		wmpgacladmin.cpp wmpgaclmanager.cpp wmpauthorizer.cpp


install:
		echo "Installing lib ..."
		cp -f libglite_wms_wmproxy_authorizer.so $(REPOSITORY)/stage/lib

clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
