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

CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS) -DDEBUG \
	-DWANT_NAMESPACES \
	-DHAVE_CONFIG_H \
	-I$(CLASSAD_LOCATION)/include \
	-I$(BOOST_LOCATION)/include \
	-I$(GLITE_LOCATION)/include \
	-I$(GLOBUS_LOCATION)/include/$(GLOBUS_FLAVOR)

GLITE_LIBS= -L$(GLITE_LOCATION)/lib \
	-lvomsapi \
	-lglite_wmsutils_tls_gsisocket_pp \
	-lglite_wmsutils_cjobid \
	-lglite_wmsutils_jobid \
	-lglite_wmsutils_exception \
	-lglite_jdl_api_cpp \
	-lglite_wmsutils_classads \
	-lglite_wms_logger \
	-lglite_wms_globus_ftp_util \
	-lglite_wms_util \
	-lglite_wms_checkpointing \
	-lglite_wms_partitioner \
	-lglite_wmsui_client \
	-lglite_lb_client_gcc32 \
	-lglite_lb_clientpp_gcc32 \
	-lglite_lb_common_gcc32 \
	-lglite_dgas_jobAuthClient \
	-lglite_dgas_xmlutil

##  BOOST 1.32 approach  ##
BOOST_FLAVOUR=gcc-mt
BOOST_LIBS=-L$(BOOST_LOCATION)/lib \
	-lboost_thread-$(BOOST_FLAVOUR)-d \
	-lboost_filesystem-$(BOOST_FLAVOUR) \
	-lboost_regex-$(BOOST_FLAVOUR)
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



all: uc match dag job jobs status loginfo logquery
	@echo "TEST SUCCESSFULLY COMPILED:"
	@ls glite-test-*

uc:
	@echo "compiling USER CREDENTIAL  ....."
	$(CPP) $(CFLAGS) -o glite-test-user-credential glite-user-credential.cpp $(LDFLAGS)
	@echo "DONE glite-test-user-credential"
match:
	@echo "compiling LIST  MATCH....."
	@$(CPP) $(CFLAGS) -o glite-test-job-listmatch glite-job-listmatch.cpp $(LDFLAGS)
	@echo "DONE: glite-test-job-listmatch"
dag:
	@echo "compiling DAG  SUBMIT....."
	@$(CPP) $(CFLAGS) -o glite-test-dag-submit glite-dag-submit.cpp $(LDFLAGS)
	@echo "DONE glite-test-dag-submit"
job:
	@echo "compiling JOB SUBMIT....."
	@$(CPP)  $(CFLAGS) -o glite-test-job-submit glite-job-submit.cpp $(LDFLAGS)
	@echo "DONE glite-test-job-submit"
jobs:
	@echo "compiling JOBS SUBMIT....."
	@$(CPP)  $(CFLAGS) -o glite-test-jobs-submit glite-jobs-submit.cpp $(LDFLAGS)
	@echo "DONE glite-test-jobs-submit"
status:
	@echo "compiling JOB STATUS....."
	@$(CPP) $(CFLAGS) -o glite-test-job-status glite-job-status.cpp $(LDFLAGS)
	@echo "DONE glite-test-job-status"
loginfo:
	@echo "compiling JOB LOGGING INFO....."
	@$(CPP) $(CFLAGS) -o glite-test-job-logging-info glite-job-logging-info.cpp $(LDFLAGS)
	@echo "DONE glite-test-job-logging-info"
logquery:
	@echo "compiling JOB LOGGING QUERY....."
	@$(CPP) $(CFLAGS) -o glite-test-job-logging-query glite-job-logging-query.cpp $(LDFLAGS)
	@echo "DONE glite-test-job-logging-query"

clean:
	@rm glite-test*
