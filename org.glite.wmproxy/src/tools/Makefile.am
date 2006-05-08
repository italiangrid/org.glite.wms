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
#       Authors: Giuseppe Avellino <giuseppe.avellino@datamat.it>
#

bin_PROGRAMS = glite_wms_wmproxy_purge_proxycache \
 glite_wms_wmproxy_gacladmin \
 glite_wms_wmproxy_gridmapfile2gacl

glite_wms_wmproxy_purge_proxycache_SOURCES = \
	wmpcleanproxies.cpp ../utilities/wmputils.cpp
glite_wms_wmproxy_purge_proxycache_LDFLAGS = -Xlinker --export-dynamic \
 -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_purge_proxycache_CFLAGS = \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(BOOST_CFLAGS)
glite_wms_wmproxy_purge_proxycache_LDADD = \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(BOOST_FS_LIBS)


glite_wms_wmproxy_gacladmin_SOURCES = \
wmpgacladmin.cpp ../authorizer/wmpgaclmanager.cpp ../authorizer/wmpauthorizer.cpp \ ../utilities/wmputils.cpp
glite_wms_wmproxy_gacladmin_LDFLAGS = -Xlinker --export-dynamic \
 -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_gacladmin_CFLAGS = \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(BOOST_CFLAGS)
glite_wms_wmproxy_gacladmin_LDADD = \
	$(GRIDSITE_OPENSSL_LIBS)


glite_wms_wmproxy_gridmapfile2gacl_SOURCES = \
mapfile2gacl.cpp ../authorizer/wmpgaclmanager.cpp ../authorizer/wmpauthorizer.cpp \ ../utilities/wmputils.cpp log.cpp
glite_wms_wmproxy_gridmapfile2gacl_LDFLAGS = -Xlinker --export-dynamic \
 -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_gridmapfile2gacl_CFLAGS = \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(BOOST_CFLAGS)
glite_wms_wmproxy_gridmapfile2gacl_LDADD = \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(BOOST_FS_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir)/../org.glite.wms.common/src/utilities \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	-DHAVE_STRINGSTREAM

MAINTAINERCLEANFILES = Makefile.in


