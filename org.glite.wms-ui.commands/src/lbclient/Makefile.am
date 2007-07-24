## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = Job.pyc \
	Listener.pyc \
	UIchecks.pyc \
        UIutils.pyc \
	glite-wms-job-attach \
        glite-wms-job-logging-info \
        glite-wms-job-status \
        Job.py \
	Listener.py \
	UIchecks.py \
	UIutils.py

bin_SCRIPTS = glite-wms-job-attach \
	glite-wms-job-logging-info \
	glite-wms-job-status 

clilibdir = $(libdir)/python
clilib_SCRIPTS=Job.py \
	Listener.py \
        UIchecks.py \
        UIutils.py \
        Job.pyc \
        Listener.pyc \
        UIchecks.pyc \
        UIutils.pyc

Job.pyc:
	/bin/touch Job.pyc

Listener.pyc:
	/bin/touch Listener.pyc

UIchecks.pyc:
	/bin/touch UIchecks.pyc

UIutils.pyc:
	/bin/touch UIutils.pyc

CLEANFILES = Job.pyc \
	Listener.pyc \
        UIchecks.pyc \
        UIutils.pyc

MAINTAINERCLEANFILES = Makefile.in

