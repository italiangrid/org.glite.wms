cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM

#REPOSITORY=/home/fpacini/CVS_REP/cern
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

LDFLAGS= -L$(GLOBUS_LOCATION)/lib \
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
	-lglobus_gsi_proxy_core_gcc32dbg \
	-lglobus_gsi_credential_gcc32dbg \
	-lglobus_gssapi_gsi_gcc32dbg \
	-lglobus_gsi_callback_gcc32dbg \
	-lglobus_oldgaa_gcc32dbg \
	-lglobus_gsi_sysconfig_gcc32dbg \
	-lglobus_gsi_cert_utils_gcc32dbg \
	-lglobus_openssl_gcc32dbg \
	-lglobus_proxy_ssl_gcc32dbg \
	-lglobus_openssl_error_gcc32dbg \
	-lglite_security_proxyrenewal_gcc32 \
	-lglite_wms_util \
	-lglite_wmsutils_jobid \
	-lglite_wmsutils_cjobid \
	-lglite_wms_jdl \
	-lglite_wms_checkpointing \
	-lglite_wms_logger \
	-lglite_wmsutils_exception \
	-lglite_wms_conf \
	-lglite_wms_broker \
	-lglite_wms_matchmaking \
	-lglite_wms_ism \
	-lares \
	-lclassad \
	-lclassad_dl \
	-lboost_fs \
	-lboost_thread \
	-lboost_signals \
	-lprg_exec_monitor \
	-lboost_regex \
	-lexpat \
	-lxml2

LDCLIENTFLAGS= -lssl \
	-lcrypto



all:	glitePipe gliteWMPCommands

glitePipe:	
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I$(CLASSAD)/include -I. -DSHOW_EXECUTION \
		    -o libglitePipe.so MatchingPipe.cpp MatchingPipeFiller.cpp \
		$(LIBS)

gliteWMPCommands: 
		$(CPP) -shared $(CFLAGS) $(CMFLAGS) -I../server \
		-I ../../../org.glite.wms.purger/src -DSHOW_EXECUTION \
	        -o libgliteWMPCommands.so common.cpp \
	        Command.cpp ExecuteFunction.cpp \
	        ForwardRequest.cpp \
	        listjobmatch.cpp listfiles.cpp \
		utils.cpp \
		CommandFactoryServerImpl.cpp \
		DagCommandFactoryServerImpl.cpp \
 		$(LIBS)


clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
