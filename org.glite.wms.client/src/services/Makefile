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
BOOST=$(repository)/boost/1.29.1/rhel30_gcc32
CLASSAD=$(repository)/classads/0.9.6/rhel30_gcc32

GEN_LD_FLAGS=-L$(GLITE_REP)/lib \
#	-L$(GLOBUS_LOCATION)/lib \
#	-lglite_wmsutils_exception \
#	-lglite_wmsutils_jobid  \
#	-lglite_wmsutils_cjobid  \

WMPCLIENT_LD_FLAGS= \
#	-L$(BOOST)/lib/release \
#	-L$(CLASSAD)/lib \
#	-L$(stage)/lib \
#	-lglite_wms_jdl \
#	-lglite_wmsutils_exception \
#	-lglite_wms_util \
#	-lglite_wmsutils_jobid  \
#	-lglite_wmsutils_cjobid \
#	-lclassad \
#	-lboost_fs \
#	-lboost_thread \
#	-lboost_signals \
#	-lboost_regex \
#	-lcrypto

WMPCLIENT_C_FLAGS= \
	-I$(stage)/include
#	-I$(BOOST)/include \
#	-I$(CLASSAD)/include \


all: libservices
libservices:
	@echo "compiling services....."
	$(CPP)  -Wall  -DHAVE_CONFIG_H  -DWANT_NAMESPACES \
	$(WMPCLIENT_LD_FLAGS) $(WMPCLIENT_C_FLAGS) -shared -o libglite_wms_client_services.so  \
	jobattach.cpp  jobcancel.cpp  joblistmatch.cpp  jobloginfo.cpp  joboutput.cpp  jobstatus.cpp  jobsubmit.cpp
	@echo "DONE"
services:
	@echo "compiling utils....."
	$(CPP)  -Wall  -DHAVE_CONFIG_H  -DWANT_NAMESPACES \
	$(WMPCLIENT_LD_FLAGS) $(WMPCLIENT_C_FLAGS) -o glite-wmp-services \
	jobattach.cpp  jobcancel.cpp  joblistmatch.cpp  jobloginfo.cpp  joboutput.cpp  jobstatus.cpp  jobsubmit.cpp
	@echo "DONE"
