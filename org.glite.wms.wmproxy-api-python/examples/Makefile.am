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
#       Authors: Luca Petronzio <luca.petronzio@elsagdatamat.com>
#

## CONFIGURATION VALUES ##
GLITE_REPOSITORY=../../..

GLITE_EXTERNALS=$(GLITE_REPOSITORY)/repository

FLAVOUR=rhel30_gcc32
GLOBUS_LOCATION=$(GLITE_EXTERNALS)/globus/2.4.3-VDT-1.2.2/$(FLAVOUR)
GLOBUS_FLAVOR=gcc32dbg


WMPROXY_UTILS = $(top_builddir)/examples/libmake_proxy_cert.la
CURL = -L/usr/lib -lcurl

bin_PROGRAMS = \
	glite-proxy-cert

glite_proxy_cert_SOURCES = \
	make_proxy_cert.c
glite_proxy_cert_LDFLAGS = -Xlinker --export-dynamic
glite_proxy_cert_LDADD = \
	-L$(GLOBUS_LOCATION)/lib \
	-lglobus_openssl_$(GLOBUS_FLAVOR)


AM_CPPFLAGS = -I$(top_srcdir)/examples \
	-I$(top_srcdir)/build/examples \
	$(OPENSSL_CFLAGS) \
	-DWANT_NAMESPACES




MAINTAINERCLEANFILES = Makefile.in
