REPOSITORY_LOC=../..

# Generic Path
GENERIC_INC=-I./
GENERIC_LIB=-L./ -L/usr/lib
COMMON_LIB=-lglite-wmp-job-utilities -lglite_jdl_api_cpp -lgridsite -lssl -lcrypto -lxml2

# Stage
STAGE_LOC=$(REPOSITORY_LOC)/stage
STAGE_INC=-I $(STAGE_LOC)/include
STAGE_LIB=-L $(STAGE_LOC)/lib

# wmprroxyapicpp
WMPROXYAPICPP_LOC=$(REPOSITORY_LOC)/org.glite.wms.wmproxy-api-cpp
WMPROXYAPICPP_INC=-I $(WMPROXYAPICPP_LOC)/src
WMPROXYAPICPP_LIB=-L$(WMPROXYAPICPP_LOC)/src -lglite_wms_wmproxy_api_cpp

# wmproxyapi Utils
WMSUTILS_JOBID_LIB=-lglite_wmsutils_jobid -lglite_wmsutils_cjobid 
WMSUTILS_EXCEPTION_LIB=-lglite_wmsutils_exception
WMSUITLS_CLASSADS=-lglite_wmsutils_classads

# ClassADS
CLASSADS_LOC=$(REPOSITORY_LOC)/repository/externals/classads/0.9.8/slc3_ia32_gcc323
CLASSADS_INC=-I $(CLASSADS_LOC)/include
CLASSADS_LIB=-L $(CLASSADS_LOC)/lib -lclassad -lclassad_dl

# Boost 1.32
BOOST_LOC=$(REPOSITORY_LOC)/repository/externals/boost/1.32.0/slc3_ia32_gcc323
BOOST_INC=-I $(BOOST_LOC)/include
BOOST_LIB=-L $(BOOST_LOC)/lib -lboost_filesystem-gcc-mt

# GSOAP 2.7.6b
GSOAP_LOC=$(REPOSITORY_LOC)/repository/externals/gsoap/2.7.6b/slc3_ia32_gcc323
GSOAP_INC=-I $(GSOAP_LOC)/include

# Curl
CURL_LIB=-lcurl

all:	utilities \
	cancel delegation output register start submit \
	register_JSDL

utilities: glite-wmp-job-utilities.cpp
	g++ -shared -O2  -DDEBUG -DWANT_NAMESPACES -DWITH_OPENSSL -o libglite-wmp-job-utilities.so glite-wmp-job-utilities.cpp \
	$(STAGE_INC) $(GENERIC_INC) $(BOOST_INC) \
	$(STAGE_LIB) $(BOOST_LIB) $(WMPROXYAPICPP_LIB)
               
cancel:	glite-wmp-job-cancel.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-cancel glite-wmp-job-cancel.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB)

delegation: glite-wmp-job-delegation.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-delegation glite-wmp-job-delegation.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB)

output:	glite-wmp-job-output.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-output glite-wmp-job-output.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB) \
	$(CURL_LIB)

register: glite-wmp-job-register.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-register glite-wmp-job-register.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB)

start:	glite-wmp-job-start.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-start glite-wmp-job-start.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB)

submit: glite-wmp-job-submit.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-submit glite-wmp-job-submit.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB) \
	$(CURL_LIB)

register_JSDL: glite-wmp-job-register_JSDL.cpp
	g++ -g -Wall -O2 -DDEBUG -DWANT_NAMESPACES -o glite-wmp-job-register_JSDL glite-wmp-job-register_JSDL.cpp \
	$(STAGE_INC) $(CLASSADS_INC) $(BOOST_INC) $(WMPROXYAPICPP_INC) $(GSOAP_INC) \
	$(GENERIC_LIB) $(STAGE_LIB) $(COMMON_LIB) $(BOOST_LIB) \
	$(WMSUTILS_JOBID_LIB) $(WMSUTILS_EXCEPTION_LIB) $(WMSUITLS_CLASSADS) \
	$(CLASSADS_LIB) $(WMPROXYAPICPP_LIB)

