CC=gcc
CPP=g++
COFLAGS=-O2
CWFLAGS=-Wall
CIFLAGS=
CMFLAGS=

## CONFIGURATION VALUES ##
GLITE_REPOSITORY=../..

GLITE_EXTERNALS=$(GLITE_REPOSITORY)/repository
GLITE_LOCATION=$(GLITE_REPOSITORY)/stage

FLAVOUR=rhel30_gcc32
GLOBUS_LOCATION=$(GLITE_EXTERNALS)/globus/2.4.3-VDT-1.2.2/$(FLAVOUR)
GLOBUS_FLAVOR=gcc32dbgpthr
GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR=gcc32dbg
CLASSAD_LOCATION=$(GLITE_EXTERNALS)/classads/0.9.6/$(FLAVOUR)
ARES_LOCATION=$(GLITE_EXTERNALS)/ares/1.1.1/$(FLAVOUR)

BOOST_LOCATION=$(GLITE_EXTERNALS)/boost/1.32.0/$(FLAVOUR)
BOOST_LOCATION=$(GLITE_EXTERNALS)/boost/1.32.0-slc3/$(FLAVOUR)


CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS) -DDEBUG \
	-DWANT_NAMESPACES \
	-DHAVE_CONFIG_H \
	-I$(CLASSAD_LOCATION)/include \
	-I$(BOOST_LOCATION)/include \
	-I$(GLITE_LOCATION)/include \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR)


EXTERNAL_LIBS= -L$(CLASSAD_LOCATION)/lib -lclassad \
	-L$(ARES_LOCATION)/lib -lares \
	-lpthread \
	-L$(GLOBUS_LOCATION)/lib \
	-lglobus_gsi_proxy_core_$(GLOBUS_FLAVOR) \
	-lglobus_gsi_credential_$(GLOBUS_FLAVOR) \
	-lglobus_gsi_callback_$(GLOBUS_FLAVOR) \
	-lglobus_oldgaa_$(GLOBUS_FLAVOR) \
	-lglobus_gsi_sysconfig_$(GLOBUS_FLAVOR) \
	-lglobus_gsi_cert_utils_$(GLOBUS_FLAVOR) \
	-lglobus_openssl_$(GLOBUS_FLAVOR) \
	-lglobus_proxy_ssl_$(GLOBUS_FLAVOR) \
	-lglobus_openssl_error_$(GLOBUS_FLAVOR) \
	-lglobus_ftp_client_$(GLOBUS_FLAVOR) \
	-lglobus_ftp_control_$(GLOBUS_FLAVOR) \
	-lglobus_io_$(GLOBUS_FLAVOR) \
	-lssl_$(GLOBUS_FLAVOR) \
	-lcrypto_$(GLOBUS_FLAVOR) \
	-lglobus_gss_assist_$(GLOBUS_FLAVOR) \
	-lglobus_gssapi_gsi_$(GLOBUS_FLAVOR) \
	-lglobus_common_$(GLOBUS_FLAVOR) \
	$(BOOST_LIBS)
LDFLAGS=$(GLITE_LIBS) $(EXTERNAL_LIBS)


all: status


status:

	$(CC) $(CFLAGS) -o proxy-cert make_proxy_cert.c $(LDFLAGS)

clean:
	@echo TB implemented
