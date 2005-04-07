
CC=gcc
CPP=g++
LIBS=
COFLAGS=-O2
#CWFLAGS=-Wall
CWFLAGS=

CIFLAGS=-I$(REPOSITORY)/org.glite.wms.wmproxy/src
CMFLAGS=-DWITH_FASTCGI -DHAVE_STRINGSTREAM
#CMFLAGS=

GSOAPVER=2.7
SOAPPARSER=$(REPOSITORY)/repository/gsoap/2.7.0/rhel30_gcc32/wsdl2h
GSOAP=$(REPOSITORY)/repository/gsoap/2.7.0/rhel30_gcc32/soapcpp2
SOAPH=$(REPOSITORY)/repository/gsoap/2.7.0/rhel30_gcc32/stdsoap2.h
SOAPC=$(REPOSITORY)/repository/gsoap/2.7.0/rhel30_gcc32/stdsoap2.c
SOAPCPP=$(REPOSITORY)/repository/gsoap/2.7.0/rhel30_gcc32/stdsoap2.cpp

GSOAPPATH=$(REPOSITORY)/repository/gsoap/2.7.0/rhel30_gcc32
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
GSI_PLUGIN=/home/peppe/webservices/gsoap/gsoap-gsi-plugin-2.3
BOOST=/home/fpacini/CVS_REP/cern/repository/boost/1.29.1/rhel30_gcc32/
LDFLAGS= #-L$(GLOBUS_LOCATION)/lib \
	#-L$(GSI_PLUGIN)/src/.libs \
	#-L$(SSL_HELPERS)/.libs \
	#-ledg_wl_ssl_helpers \
	#-ledg_wl_ssl_pthr_helpers \
	#-lpthread \
	#-lgsigsoapthreads \
  	#-lglobus_gss_assist_$(GLOBUS_FLAVOR_THREADS) \
   	#-lglobus_gssapi_gsi_$(GLOBUS_FLAVOR_THREADS) \
	#-lglobus_common_$(GLOBUS_FLAVOR_THREADS)
all: client
cli: lib client
allinone: wsdl soap lib client

wsdl:
		$(SOAPPARSER) -o wm.h -t$(GSOAPPATH)/typemap.dat $(INTERFACE)/WMProxy.wsdl
		$(GSOAP) -I$(GSOAPPATH) wm.h
		rm *.xml


client: wm.h $(SOAPH) $(SOAPCPP)
	$(CPP) $(CFLAGS) -I$(BOOST)/include -DWITH_OPENSSL\
	-o raskclient raskclient.cpp $(SOAPCPP) $(LIBS) -L./ -lwmproxy_api_cpp -lssl -lcrypto $(LDFLAGS)


lib: wm.h $(SOAPH) $(SOAPCPP)
	$(CPP) $(CFLAGS) -I$(GSOAPPATH) -L$(REPLIBS) -DWITH_OPENSSL \
	-I$(BOOST)/include -I../interface \
	-shared -o libwmproxy_api_cpp.so wmproxy_api.cpp soapC.cpp soapClient.cpp $(SOAPCPP) $(LIBS) $(LDFLAGS)

clean:		
		rm -f *.o soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp soapClient.c \
			soapServer.cpp soapServer.c soap*Proxy.h
distclean:	
		rm -f *.o *.xsd *.xml *.nsmap *.log soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp \
			soapClient.c soapServer.cpp soapServer.c soap*Proxy.h
