SOAPPARSER=../wsdl2h
GSOAP=../soapcpp2
SOAPH=../stdsoap2.h
SOAPC=../stdsoap2.c
SOAPCPP=../stdsoap2.cpp
SOAPCPPOBJ=stdsoap2.o
CC=gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
#CWFLAGS=-Wall
CIFLAGS=-I../.. -I..
CMFLAGS=
CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS) -DDEBUG

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
		@echo "THIS IS THE REAL WSDL!!!"
		$(SOAPPARSER) -o WMProxy.h WMProxy.wsdl
soap:
		$(GSOAP) -I. -I.. WMProxy.h
		rm *.xml

client: WMProxy.h $(SOAPH) $(SOAPCPP)
	$(CPP) $(CFLAGS) -I$(BOOST)/include -DWITH_OPENSSL\
	-o raskclient raskclient.cpp $(SOAPCPP) $(LIBS) -L./ -lwmproxy_api_cpp -lssl -lcrypto $(LDFLAGS)


lib: WMProxy.h $(SOAPH) $(SOAPCPP)
	$(CPP) $(CFLAGS) -I$(BOOST)/include \
	-shared -o libwmproxy_api_cpp.so wmproxy_api.cpp soapC.cpp soapClient.cpp $(SOAPCPP) $(LIBS) $(LDFLAGS)

clean:		
		rm -f *.o soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp soapClient.c \
			soapServer.cpp soapServer.c soap*Proxy.h
distclean:	
		rm -f *.o *.xsd *.xml *.nsmap *.log soapH.h soapStub.h soapC.cpp soapC.c soapClient.cpp \
			soapClient.c soapServer.cpp soapServer.c soap*Proxy.h
