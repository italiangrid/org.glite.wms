## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglite_wms_purger.la

noinst_HEADERS     = purger.h

#sbin_PROGRAMS      = edg-wl-purgeStorage

#wlpurgerdir = $(sysconfdir)/cron
#wlpurger_DATA = edg-wl-purgeStorage-weekly.crontab \
#	edg-wl-purgeStorage-hourly.crontab 

#libexec_SCRIPTS = edg-wl-purgestorage.sh

#wlprdir = $(datadir)/doc/@PACKAGE@-wm-@VERSION@
#wlpr_DATA = README.purge

libglite_wms_purger_la_SOURCES = \
	purger.cpp

#edg_wl_purgeStorage_SOURCES = \
#	edg-wl-purgeStorage.cpp

#edg_wl_purgeStorage_LDADD = \
#	../common/jobid/libedg_wl_jobid.la \
#	../common/utilities/libedg_wl_exceptions.la \
#	../common/configuration/libedg_wl_conf.la \
#	../common/utilities/libedg_wl_util.la \
#	../common/logger/libedg_wl_logger.la \
#	../logging/client/libedg_wl_logging.la \
#	../logging/client/libedg_wl_loggingpp.la \
#	libedg_wl_purger.la \
#	$(BOOST_LIBS) $(GLOBUS_LIBS) $(CLASSAD_LIBS) \
#	$(ARES_LIBS) $(EXPAT_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(ARES_CFLAGS) \
	$(GLOBUS_CFLAGS) \
	$(EXPAT_CFLAGS) \
	-DWITH_LDAP_EXCEPTIONS 

#EXTRA_DIST = edg-wl-purgeStorage-weekly.crontab \
#       edg-wl-purgeStorage-hourly.crontab \
#	README.purge \
#	edg-wl-purgestorage.sh.in

#CLEANFILES = edg-wl-purgestorage.sh

MAINTAINERCLEANFILES = Makefile.in

