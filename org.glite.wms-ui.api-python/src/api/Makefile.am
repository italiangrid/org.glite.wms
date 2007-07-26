## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	wmsui_api.pyc \
	wmsui_listener.pyc \
	wmsui_checks.pyc \
        wmsui_utils.pyc \
        wmsui_api.py \
	wmsui_listener.py \
	wmsui_checks.py \
	wmsui_utils.py

clilibdir = $(libdir)/python
clilib_SCRIPTS = \
	wmsui_api.py \
	wmsui_listener.py \
        wmsui_checks.py \
        wmsui_utils.py \
        wmsui_api.pyc \
        wmsui_listener.pyc \
        wmsui_checks.pyc \
        wmsui_utils.pyc

wmsui_api.pyc:
	/bin/touch wmsui_api.pyc

wmsui_listener.pyc:
	/bin/touch wmsui_listener.pyc

wmsui_checks.pyc:
	/bin/touch wmsui_checks.pyc

wmsui_utils.pyc:
	/bin/touch wmsui_utils.pyc

CLEANFILES = \
	wmsui_api.pyc \
	wmsui_listener.pyc \
        wmsui_checks.pyc \
        wmsui_utils.pyc

MAINTAINERCLEANFILES = Makefile.in
