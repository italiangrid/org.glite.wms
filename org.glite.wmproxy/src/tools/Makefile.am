#
#       Copyright (c) 2004 on behalf of the EU EGEE Project:
#       The European Organization for Nuclear Research (CERN),
#       Istituto Nazionale di Fisica Nucleare (INFN), Italy
#       Datamat Spa, Italy
#       Centre National de la Recherche Scientifique (CNRS), France
#       CS Systeme d'Information (CSSI), France
#       Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
#       Universiteit van Amsterdam (UvA), Netherlands
#       University of Helsinki (UH.HIP), Finland
#       University of Bergen (UiB), Norway
#       Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#
#       Authors: Giuseppe Avellino <egee@datamat.it>
#

noinst_HEADERS = converter.h  log.h proxy_cleaner.h

bin_PROGRAMS = glite-wms-wmproxy-purge-proxycache-binary \
		glite-wms-wmproxy-gacladmin \
		glite-wms-wmproxy-gridmapfile2gacl

glite_wms_wmproxy_purge_proxycache_binary_SOURCES = wmpcleanproxies.cpp \
							wmputils.cpp utils.cpp \
							log.cpp

glite_wms_wmproxy_purge_proxycache_binary_LDFLAGS = -Xlinker --export-dynamic -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_purge_proxycache_binary_LDADD = \
	$(GRIDSITE_OPENSSL_LIBS) \
  	$(CLASSAD_LIBS) \
	$(ARES_LIBS) \
        $(GLITE_LB_COMMON_LIBS) \
        $(GLITE_LB_CLIENT_LIBS) \
        $(GLITE_LB_CLIENTPP_LIBS) \
	$(BOOST_FS_LIBS) \
	$(GLITE_JOBID_LIBS)

glite_wms_wmproxy_gacladmin_SOURCES = wmpgacladmin.cpp \
					wmpgaclmanager.cpp \
					wmpauthorizer.cpp \
					wmputils.cpp \
					utils.cpp

glite_wms_wmproxy_gacladmin_LDFLAGS = -Xlinker --export-dynamic -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_gacladmin_LDADD = \
	$(GLITE_LB_COMMON_LIBS) \
        $(GLITE_LB_CLIENT_LIBS) \
        $(GLITE_LB_CLIENTPP_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(GRIDSITE_LIBS) \
        $(ARES_LIBS) \
	$(GLITE_JOBID_LIBS)

glite_wms_wmproxy_gridmapfile2gacl_SOURCES = wmpgridmap2gacl.cpp \
						wmpgaclmanager.cpp \
						wmpauthorizer.cpp \
						wmputils.cpp \
						log.cpp \
						utils.cpp

glite_wms_wmproxy_gridmapfile2gacl_LDFLAGS = -Xlinker --export-dynamic -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_gridmapfile2gacl_LDADD = \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(GRIDSITE_LIBS) \
        $(ARES_LIBS) \
	$(GLITE_LB_COMMON_LIBS) \
        $(GLITE_LB_CLIENT_LIBS) \
        $(GLITE_LB_CLIENTPP_LIBS) \
	$(BOOST_FS_LIBS) \
	$(GLITE_JOBID_LIBS)

wmputils.cpp:
	/bin/cp $(top_srcdir)/src/utilities/wmputils.cpp wmputils.cpp

wmpauthorizer.cpp:
	/bin/cp $(top_srcdir)/src/authorizer/wmpauthorizer.cpp wmpauthorizer.cpp

wmpgaclmanager.cpp:
	/bin/cp $(top_srcdir)/src/authorizer/wmpgaclmanager.cpp wmpgaclmanager.cpp



AM_CPPFLAGS = \
	-DGLITE_WMS_WMPROXY_TOOLS \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/authorizer \
	-I$(top_srcdir)/src/utilities \
	-I$(top_srcdir)/src/eventlogger \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(GRIDSITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLITE_WMS_COMMON_LOGGER_CFLAGS) \
	$(GLITE_JDL_CFLAGS) \
	$(GLITE_JOBID_CFLAGS) \
	$(BOOST_CFLAGS)

CLEANFILES = wmputils.cpp wmpauthorizer.cpp wmpgaclmanager.cpp

MAINTAINERCLEANFILES = Makefile.in


