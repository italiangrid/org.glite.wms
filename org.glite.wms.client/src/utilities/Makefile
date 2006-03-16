CC=gcc
CPP=g++
LIBS=
COFLAGS=-O2
//CWFLAGS=-Wall
CWFLAGS=
CIFLAGS=
CMFLAGS=


REPOSITORY=../../../
repository=$(REPOSITORY)/repository
stage=$(REPOSITORY)/stage
BOOST=$(repository)/boost/1.32.0/rhel30_gcc32
CLASSAD=$(repository)/classads/0.9.6/rhel30_gcc32
CURL=-L/usr/lib -lcurl


WMPCLIENT_LD_FLAGS= \
	-L$(BOOST)/lib/release \
	-L$(CLASSAD)/lib \
	-L$(stage)/lib \
	-lglite_wmsutils_exception \
	-lglite_wms_util \
	-lglite_wms_conf \
	-lglite_wmsutils_jobid  \
	-lglite_wmsutils_cjobid \
	-lglite_jdl_api_cpp \
	-lclassad



WMPCLIENT_LD_EXE_FLAGS= \
	$(WMPCLIENT_LD_FLAGS) \
	$(CURL) \
	-lglite_wms_wmproxy_api_cpp \
	-lgridsite \
	-lcrypto \
	-lboost_fs \
	-lboost_thread \
	-lboost_signals \
	-lboost_regex \
	-lxml2

WMPCLIENT_C_FLAGS= \
	-I$(BOOST)/include \
	-I$(CLASSAD)/include \
	-I$(stage)/include 

WMPCLIENT_FILES= utils.cpp \
	adutils.cpp \
	excman.cpp \
 	logman.cpp \
	options_utils.cpp

all: libutils
libutils:
	@echo "compiling utils....."
	$(CPP)  -Wall  -DHAVE_CONFIG_H  -DWANT_NAMESPACES -DWITH_OPENSSL \
	$(WMPCLIENT_LD_FLAGS) $(WMPCLIENT_C_FLAGS) -shared -o libglite_wms_client_utils.so  $(WMPCLIENT_FILES)
	@echo "DONE"
utils:
	@echo "compiling utils....."
	$(CPP)  -Wall  -DHAVE_CONFIG_H  -DWANT_NAMESPACES \
	$(WMPCLIENT_LD_EXE_FLAGS) $(WMPCLIENT_C_FLAGS) -o glite-wmp-utils glite-wmp-utils.cpp $(WMPCLIENT_FILES)
	@echo "DONE"
