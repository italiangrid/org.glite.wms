CC=gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=-Wall

CIFLAGS=-I$(REPOSITORY)/org.glite.wms.wmproxy/src
CMFLAGS=-DWITH_FASTCGI -DHAVE_STRINGSTREAM
#CMFLAGS=

GSOAPVER=2.7.6b
SOAPPARSER=$(REPOSITORY)/repository/gsoap/$(GSOAPVER)/rhel30_gcc32/wsdl2h
GSOAP=$(REPOSITORY)/repository/gsoap/$(GSOAPVER)/rhel30_gcc32/soapcpp2
SOAPH=$(REPOSITORY)/repository/gsoap/$(GSOAPVER)/rhel30_gcc32/stdsoap2.h
SOAPC=$(REPOSITORY)/repository/gsoap/$(GSOAPVER)/rhel30_gcc32/stdsoap2.c
SOAPCPP=$(REPOSITORY)/repository/gsoap/$(GSOAPVER)/rhel30_gcc32/stdsoap2.cpp

GSOAPPATH=$(REPOSITORY)/repository/gsoap/$(GSOAPVER)/rhel30_gcc32
REPLIBS=$(REPOSITORY)/stage/lib
INTERFACE=$(REPOSITORY)/org.glite.wms.wmproxy-interface/interface

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32
#GLOBUS_LOCATION=/opt/globus
CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32
ARES=$(REPOSITORY)/repository/ares/1.1.1/rhel30_gcc32
EXPAT=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32

GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr
GLOBUS_SSL_THR_LIBS = -lssl_gcc32dbgpthr -lcrypto_gcc32dbgpthr
GLOBUS_GSS_THR_LIBS = -lglobus_gssapi_gsi_gcc32dbgpthr -lglobus_gss_assist_gcc32dbgpthr
GLOBUS_COMMON_THR_LIBS = -lglobus_common_gcc32dbgpthr
GLOBUS_SSL_LIBS = -lssl_gcc32dbg -lcrypto_gcc32dbg
GLOBUS_GSS_LIBS = -lglobus_gssapi_gsi_gcc32dbg -lglobus_gss_assist_gcc32dbg
GLOBUS_COMMON_LIBS = -lglobus_common_gcc32dbg


CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) \
	-DWANT_NAMESPACES \
	-I/usr/include/httpd -I/usr/include/apr-0 \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR) -g \
	-I$(ARES)/include \
	-I$(REPOSITORY)/org.glite.wms.wmproxy/src \
	-I$(GSOAPPATH) \
	-I$(REPOSITORY)/stage/include \
	-I$(CLASSAD)/include \
	-I$(BOOST)/include/boost \
	-I$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32/include

LDFLAGS= \
	-L$(GLOBUS_LOCATION)/lib \
	-L$(REPLIBS) \
	-L$(CLASSAD)/lib \
	-L$(BOOST)/lib/release \
	-L$(ARES)/lib \
	-L$(EXPAT)/lib \
	-lglite_lb_common_gcc32 \
	-lglite_lb_client_gcc32 \
	-lglite_lb_clientpp_gcc32 \
	$(GLOBUS_SSL_LIBS) \
	$(GLOBUS_GSS_LIBS) \
	$(GLOBUS_COMMON_LIBS) \
	-lglite_security_proxyrenewal_gcc32 \
	-lglite_wms_util \
	-lglite_wmsutils_jobid \
	-lglite_wmsutils_cjobid \
	-lglite_wms_jdl \
	-lglite_wms_checkpointing \
	-lglite_wms_logger \
	-lglite_wms_conf \
	-lglite_wmsutils_exception \
	-lglite_wms_partitioner \
	-lglite_wms_wmproxy_pipe \
	-lglite_wms_wmproxy_commands \
	-lglite_wms_wmproxy_utilities \
	-lglite_wms_wmproxy_eventlogger \
	-lglite_wms_wmproxy_authorizer \
	-lares \
	-lclassad \
	-lclassad_dl \
	-lboost_fs \
	-lboost_thread \
	-lboost_signals \
	-lprg_exec_monitor \
	-lboost_regex \
	-lexpat \
	-lgridsite_globus \
	-llcmaps \
	-llcmaps_return_poolindex_without_gsi \
	-lvomsc \
	-lxml2 \
	-lfcgi++

C1FLAGS= \
        -I$(REPOSITORY)/stage/include

LD1FLAGS= -lssl \
	-lcrypto \
	-lxml2 \
	-lgridsite

	#$(GLOBUS_COMMON_LIBS) \
	#-lglite_wmsutils_cjobid \

all:	wmProxy wmClient

old:	oldwmProxy oldwmClient

wsdl:	
		$(SOAPPARSER) -o wm.h -t$(GSOAPPATH)/typemap.dat $(INTERFACE)/WMProxy.wsdl
		$(GSOAP) -I$(GSOAPPATH) wm.h
		rm *.xml

BUILD=$(REPOSITORY)/org.glite.wms.wmproxy/build

wmClient:		$(BUILD)/src/server/wm.h wmclient.cp $(SOAPH) $(SOAPCPP)
			@echo "wmclient....."
			$(CPP) -I $(BUILD)/src/server \
				-I. $(C1FLAGS) -I$(GSOAPPATH) -L$(REPLIBS) -DWITH_OPENSSL \
				-o wmClient wmclient.cp $(BUILD)/src/server/soapC.cpp $(BUILD)/src/server/soapClient.cpp \
				$(SOAPCPP) $(LIBS) $(LD1FLAGS)

wmProxy:        wm.h \
                                wmproxy.cpp \
                                wmpoperations.cpp wmpoperations.h  \
                                wmpgsoapoperations.cpp \
								wmpconfiguration.h wmpconfiguration.cpp \
								wmpgsoapfaultmanipulator.h wmpgsoapfaultmanipulator.cpp \
                                wmpdelegation.cpp wmpdelegation.h \
                                wmpdispatcher.cpp wmpdispatcher.h \
                                wmpmanager.cpp wmpmanager.h \
                                wmp2wm.cpp wmp2wm.h \
                                wmpresponsestruct.h $(SOAPH) $(SOAPCPP)
			@echo "wmproxy....."
			$(CPP) -I../utilities $(CFLAGS) $(CMFLAGS) $(EXTRALIBFLAGS) \
                                -o wmProxy wmproxy.cpp soapC.cpp soapServer.cpp \
                                wmpoperations.cpp \
                                wmpgsoapoperations.cpp \
                                wmpgsoapfaultmanipulator.cpp \
                                wmpdispatcher.cpp wmpconfiguration.cpp\
                                wmpdelegation.cpp \
                                wmpmanager.cpp wmp2wm.cpp \
                                $(SOAPCPP) $(LIBS) $(LDFLAGS) 



clean:
		rm -f *.o soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp soapClient.c \
			soapServer.cpp soapServer.c soap*Lib.* soap*.h *.xml

distclean:
		rm -f *.o *.xsd *.xml *.nsmap *.log soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp \
			soapClient.c soapServer.cpp soapServer.c soap*Proxy.h
