## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************


wmppyapilibdir = $(libdir)/python
wmppyapilib_SCRIPTS=WMPClient.py \
	wmproxymethods.py \
	WMPConnection.py \
	WMPClient.pyc \
	wmproxymethods.pyc \
	WMPConnection.pyc

WMPClient.pyc:
	/bin/touch WMPClient.pyc

wmproxymethods.pyc:
	/bin/touch wmproxymethods.pyc

WMPConnection.pyc:
	/bin/touch WMPConnection.pyc

CLEANFILES = WMPClient.pyc \
	wmproxymethods.pyc \
	WMPConnection.pyc

MAINTAINERCLEANFILES = Makefile.in

