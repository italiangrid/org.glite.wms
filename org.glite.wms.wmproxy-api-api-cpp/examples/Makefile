cc=Gcc
CPP=g++
LIBS=
COFLAGS=-O2
CWFLAGS=
CIFLAGS=-I../..
CMFLAGS=-DHAVE_STRINGSTREAM

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

REPLIBS=$(REPOSITORY)/stage/lib

BOOST_LIBS=-lboost_fs -lboost_thread -lboost_regex 
CLASSAD_LIBS=-lclassad -lclassad_dl 
ARES_LIBS=-lares
GSI_PLUGIN=/home/peppe/webservices/gsoap/gsoap-gsi-plugin-2.0

CIFLAGS=-I$(REPOSITORY)/stage/include \
	-I./ \
	-I$(REPOSITORY)/org.glite.wms.wmproxy-api-cpp/interface \
	-I$(REPOSITORY)/org.glite.wms.wmproxy/src \
	-I$(BOOST)/include

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS) \
-DDEBUG -DWANT_NAMESPACES -DWITH_OPENSSL


GRIDSITE_LIBS=-lgridsite -lxml2

UTILITIES_LIB=glite-wmp-job-uilities

CURL=-L/usr/lib -lcurl

LDFLAGS= -L$(REPOSITORY)/org.glite.wms.wmproxy-api-cpp/src\
-L$(REPLIBS) \
-L$(lib_dir) \
$(GRIDSITE_LIBS) \
-lwmproxy_api_cpp \
-lssl -lcrypto \
-l$(UTILITIES_LIB)

JOBAD= -I$(REPOSITORY)/org.glite.wms.jdl/src \
-lglite_wms_jdl  -lglite_wms_util -lglite_wmsutils_exception -lglite_wmsutils_cjobid \
-lglite_wmsutils_jobid \
-L$(BOOST)/lib/release -lboost_fs -lboost_thread -lboost_regex \
-L$(CLASSAD)/lib -lclassad -lclassad_dl\

utilities_SOURCE=glite-wmp-job-utilities.cpp
utilities_OBJ=lib$(UTILITIES_LIB).so
utilities_CFLAGS=$(CFLAGS)
utilities_DEPS=
UTILITIES=$(lib_dir)/$utilities_OBJ

glite_wmp_job_submit_SOURCE=glite-wmp-job-submit.cpp
glite_wmp_job_submit_OBJ=glite-wmp-job-submit
glite_wmp_job_submit_CFLAGS=$(CFLAGS) \
-I$(CLASSAD)/include
glite_wmp_job_submit_DEPS=$(LDFLAGS) \
$(JOBAD) \
$(CURL)

glite_wmp_job_delegation_SOURCE=glite-wmp-job-delegation.cpp
glite_wmp_job_delegation_OBJ=glite-wmp-job-delegation
glite_wmp_job_delegation_CFLAGS=$(CFLAGS)
glite_wmp_job_delegation_DEPS=$(LDFLAGS)


glite_wmp_job_register_SOURCE=glite-wmp-job-register.cpp
glite_wmp_job_register_OBJ=glite-wmp-job-register
glite_wmp_job_register_CFLAGS=$(CFLAGS) \
-I$(CLASSAD)/include
glite_wmp_job_register_DEPS=$(LDFLAGS) \
$(JOBAD)

glite_wmp_job_start_SOURCE=glite-wmp-job-start.cpp
glite_wmp_job_start_OBJ=glite-wmp-job-start
glite_wmp_job_start_CFLAGS=$(CFLAGS)
glite_wmp_job_start_DEPS=$(LDFLAGS)

glite_wmp_job_cancel_SOURCE=glite-wmp-job-cancel.cpp
glite_wmp_job_cancel_OBJ=glite-wmp-job-cancel
glite_wmp_job_cancel_CFLAGS=$(CFLAGS)
glite_wmp_job_cancel_DEPS=$(LDFLAGS)


glite_wmp_job_output_SOURCE=glite-wmp-job-output.cpp
glite_wmp_job_output_OBJ=glite-wmp-job-output
glite_wmp_job_output_CFLAGS=$(CFLAGS)
glite_wmp_job_output_DEPS=$(LDFLAGS) \
$(CURL)

all:	utilities \
submit delegation \
register start \
cancel output

utilities:	$(utilities_SOURCE)
		@if ! test -d $(lib_dir) ; then \
			mkdir $(lib_dir) ; \
		fi

		@echo "building "$(utilities_OBJ)" ..."
		$(CPP) -shared $(utilities_CFLAGS) $(utilities_DEPS) \
			-o  $(utilities_OBJ)  \
			$(utilities_SOURCE)



		@mv $(utilities_OBJ) $(lib_dir)

submit:	$(glite_wmp_job_submit_SOURCE)
		@echo "building "$(glite_wmp_job_submit_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi

		$(CPP) -g -Wall $(glite_wmp_job_submit_CFLAGS) $(glite_wmp_job_submit_DEPS) \
			-o  $(glite_wmp_job_submit_OBJ)  \
			$(glite_wmp_job_submit_SOURCE)
		@mv $(glite_wmp_job_submit_OBJ) $(bin_dir)

delegation:	 $(glite_wmp_job_delegation_SOURCE)
			@echo "building "$(glite_wmp_job_delegation_OBJ)" ..."

			@if ! test -d $(bin_dir) ; then \
				mkdir $(bin_dir) ; \
			fi

			$(CPP) -g -Wall $(glite_wmp_job_delegation_CFLAGS) $(glite_wmp_job_delegation_DEPS) \
				-o  $(glite_wmp_job_delegation_OBJ)  \
				$(glite_wmp_job_delegation_SOURCE)
			@mv $(glite_wmp_job_delegation_OBJ) $(bin_dir)

register:	$(glite_wmp_job_register_SOURCE)
		@echo "building "$(glite_wmp_job_register_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi

		$(CPP) -g -Wall $(glite_wmp_job_register_CFLAGS) $(glite_wmp_job_register_DEPS) \
			-o  $(glite_wmp_job_register_OBJ)  \
			$(glite_wmp_job_register_SOURCE)
		@mv $(glite_wmp_job_register_OBJ) $(bin_dir)

start:	 $(glite_wmp_job_start_SOURCE)
		@echo "building "$(glite_wmp_job_start_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi

		$(CPP) -g -Wall $(glite_wmp_job_start_CFLAGS) $(glite_wmp_job_start_DEPS) \
			-o  $(glite_wmp_job_start_OBJ)  \
			$(glite_wmp_job_start_SOURCE)
		@mv $(glite_wmp_job_start_OBJ) $(bin_dir)

cancel:	$(glite_wmp_job_cancel_SOURCE)
		@echo "building "$(glite_wmp_job_cancel_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi

		$(CPP) -g -Wall $(glite_wmp_job_cancel_CFLAGS) $(glite_wmp_job_cancel_DEPS) \
			-o  $(glite_wmp_job_cancel_OBJ)  \
			$(glite_wmp_job_cancel_SOURCE)
		@mv $(glite_wmp_job_cancel_OBJ) $(bin_dir)

output:	$(glite_wmp_job_output_SOURCE)
		@echo "building "$(glite_wmp_job_output_OBJ)" ..."

		@if ! test -d $(bin_dir) ; then \
			mkdir $(bin_dir) ; \
		fi

		$(CPP) -g -Wall $(glite_wmp_job_output_CFLAGS) $(glite_wmp_job_output_DEPS) \
			-o  $(glite_wmp_job_output_OBJ)  \
			$(glite_wmp_job_output_SOURCE)
		@mv $(glite_wmp_job_output_OBJ) $(bin_dir)

clean:
		rm -f *.o *.so

distclean:
		rm -f *.o *.so
