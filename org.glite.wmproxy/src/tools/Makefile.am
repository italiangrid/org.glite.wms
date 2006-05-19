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
noinst_HEADERS = converter.h  log.h

bin_PROGRAMS = glite-wms-wmproxy-purge-proxycache \
 glite-wms-wmproxy-gacladmin \
 glite-wms-wmproxy-gridmapfile2gacl

glite_wms_wmproxy_purge_proxycache_SOURCES = wmpcleanproxies.cpp ../utilities/wmputils.cpp
#glite_wms_wmproxy_purge_proxycache_OBJECTS = wmpcleanproxies.o $(top_builddir)/tools/wmputils.o
#glite_wms_wmproxy_purge_proxycache_OBJECTS = wmpcleanproxies.o wmputils.o
glite_wms_wmproxy_purge_proxycache_LDFLAGS = -Xlinker --export-dynamic -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_purge_proxycache_LDADD = \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(BOOST_FS_LIBS)


glite_wms_wmproxy_gacladmin_SOURCES = wmpgacladmin.cpp ../authorizer/wmpgaclmanager.cpp wmpauthorizer.cpp wmputils.cpp
#glite_wms_wmproxy_gacladmin_OBJECTS = wmpgacladmin.o $(top_builddir)/tools/wmpgaclmanager.o $(top_builddir)/tools/wmpauthorizer.o $(top_builddir)/tools/wmputils.o
#glite_wms_wmproxy_gacladmin_OBJECTS = wmpgacladmin.o wmpgaclmanager.o wmpauthorizer.o wmputils.o
glite_wms_wmproxy_gacladmin_LDFLAGS = -Xlinker --export-dynamic -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_gacladmin_LDADD = \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)


glite_wms_wmproxy_gridmapfile2gacl_SOURCES = wmpgridmap2gacl.cpp wmpgaclmanager.cpp wmpauthorizer.cpp wmputils.cpp log.cpp
#glite_wms_wmproxy_gridmapfile2gacl_OBJECTS =  wmpgridmap2gacl.o log.o wmpgaclmanager.o wmpauthorizer.o wmputils.o
#glite_wms_wmproxy_gridmapfile2gacl_OBJECTS =  wmpgridmap2gacl.o log.o $(top_builddir)/tools/wmpgaclmanager.o $(top_builddir)/tools/wmpauthorizer.o $(top_builddir)/tools/wmputils.o
glite_wms_wmproxy_gridmapfile2gacl_LDFLAGS = -Xlinker --export-dynamic -DGLITE_WMS_WMPROXY_TOOLS
glite_wms_wmproxy_gridmapfile2gacl_LDADD = \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(BOOST_FS_LIBS)


#wmpcleanproxies.o: wmpcleanproxies.cpp
#	$(CXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<
#	$(LTCXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<

#wmpgacladmin.o: wmpgacladmin.cpp
#	$(CXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<
#	$(LTCXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<

#wmpgridmap2gacl.o: wmpgridmap2gacl.cpp
#	$(CXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<
#	$(LTCXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<


#wmputils.o: wmputils.cpp
#	$(CXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<
#	$(LTCXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<

wmputils.cpp:
	/bin/cp $(top_srcdir)/src/utilities/wmputils.cpp wmputils.cpp


#wmpauthorizer.o: wmpauthorizer.cpp
#	$(CXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<
#	$(LTCXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<

wmpauthorizer.cpp:
	/bin/cp $(top_srcdir)/src/authorizer/wmpauthorizer.cpp wmpauthorizer.cpp

#wmpgaclmanager.o: wmpgaclmanager.cpp
#	$(CXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<
#	$(LTCXXCOMPILE) -DGLITE_WMS_WMPROXY_TOOLS -c $<

wmpgaclmanager.cpp:
	/bin/cp $(top_srcdir)/src/authorizer/wmpgaclmanager.cpp wmpgaclmanager.cpp



AM_CPPFLAGS = \
	-DGLITE_WMS_WMPROXY_TOOLS \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/authorizer \
	-I$(top_srcdir)/src/utilities \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	-DHAVE_STRINGSTREAM

CLEANFILES = wmputils.cpp wmpauthorizer.cpp wmpgaclmanager.cpp

MAINTAINERCLEANFILES = Makefile.in


