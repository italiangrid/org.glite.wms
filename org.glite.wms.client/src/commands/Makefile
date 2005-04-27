cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=

COFLAGS=-O2
CWFLAGS=
#CWFLAGS=-Wall

CMFLAGS=


top_srcdir= ..
src_dir = $(top_srcdir)/src
bin_dir = $(top_srcdir)/bin
lib_dir = $(top_srcdir)/lib

REPOSITORY = ../../..
GLOBUS_FLAVOR=gcc32dbg
GLOBUS_FLAVOR_THREADS=gcc32dbgpthr

GLOBUS_LOCATION=$(REPOSITORY)/repository/globus/2.4.3-VDT-1.2.2/rhel30_gcc32
BOOST_LOCATION=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32
EXPAT_LOCATION=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32/

ARES=$(REPOSITORY)/repository/ares/1.1.1/rhel30_gcc32
EXPAT=$(REPOSITORY)/repository/expat/1.95.7/rhel30_gcc32
CLASSAD=$(REPOSITORY)/repository/classads/0.9.6/rhel30_gcc32
BOOST=$(REPOSITORY)/repository/boost/1.29.1/rhel30_gcc32

REPLIBS=-L$(REPOSITORY)/stage/lib

BOOST_LIBS=-L$(BOOST_LOCATION)/lib/release \
-lboost_fs -lboost_thread -lboost_regex
CLASSAD_LIBS=-L$(CLASSAD)/lib  -lclassad -lclassad_dl
ARES_LIBS=-lares
GSI_PLUGIN=/home/peppe/webservices/gsoap/gsoap-gsi-plugin-2.0

CIFLAGS=-I../ \
-I$(REPOSITORY)/stage/include \
-I$(CLASSAD)/include

CFLAGS= -DDEBUG -DWANT_NAMESPACES -DWITH_OPENSSL\
$(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)

JDL_LIB=-lglite_wms_jdl \
-lglite_wms_util
JOBID_LIB=-lglite_wmsutils_jobid \
-lglite_wmsutils_cjobid
EXC_LIB=-lglite_wmsutils_exception

GRIDSITE_LIBS=-lgridsite -lxml2

CURL=-L/usr/lib -lcurl

#LDFLAGS=$(REPLIBS) \
#$(EXC_LIB) \
#$(JDL_LIB) \
#$(JOBID_LIB) \
#$(CLASSAD_LIBS) \
#$(BOOST_LIBS) \
#-lcrypto

COMMON_LD_FLAGS=\
-L../services \
-L../utilities \
-lglite_wms_client_utils \
-lglite_wms_client_services


job_submit_SOURCE=wmp-job-submit.cpp
job_submit_OBJ=glite-job-submit
job_submit_CFLAGS=$(CFLAGS)
job_submit_DEPS=$(COMMON_LD_FLAGS)

job_status_SOURCE=wmp-job-status.cpp
job_status_OBJ=glite-job-status
job_status_CFLAGS=$(CFLAGS)
job_status_DEPS=$(COMMON_LD_FLAGS)

job_attach_SOURCE=wmp-job-attach.cpp
job_attach_OBJ=glite-job-attach
job_attach_CFLAGS=$(CFLAGS)
job_attach_DEPS=$(COMMON_LD_FLAGS)


all: submit status attach

submit:	$(job_submit_SOURCE)
		@echo "building "$(job_submit__OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi

		$(CPP) -g -Wall  $(job_submit_CFLAGS) $(job_submit_DEPS) \
			-o  $(job_submit_OBJ)  \
			$(job_submit_SOURCE)
		@mv $(job_submit_OBJ) $(bin_dir)

status:	$(job_status_SOURCE)
		@echo "building "$(job_status_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi
		$(CPP) -g -Wall  $(job_status_CFLAGS) $(job_status_DEPS) \
			-o  $(job_status_OBJ)  \
			$(job_status_SOURCE)
		@mv $(job_status_OBJ) $(bin_dir)

attach:	$(job_attach_SOURCE)
		@echo "building "$(job_attach_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi
		$(CPP) -g -Wall  $(job_attach_CFLAGS) $(job_attach_DEPS) \
			-o  $(job_attach_OBJ)  \
			$(job_attach_SOURCE)
		@mv $(job_attach_OBJ) $(bin_dir)

clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
