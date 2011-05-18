#  Copyright (c) Members of the EGEE Collaboration. 2004. 
#  See http://www.eu-egee.org/partners/ for details on the copyright
#  holders.  
#  
#  Licensed under the Apache License, Version 2.0 (the "License"); 
#  you may not use this file except in compliance with the License. 
#  You may obtain a copy of the License at 
#  
#      http://www.apache.org/licenses/LICENSE-2.0 
#  
#  Unless required by applicable law or agreed to in writing, software 
#  distributed under the License is distributed on an "AS IS" BASIS, 
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
#  See the License for the specific language governing permissions and 
#  limitations under the License.

dist_man1_MANS = glite-wms-job-cancel.1 \
	glite-wms-job-output.1 \
	glite-wms-job-list-match.1 \
	glite-wms-job-delegate-proxy.1 \
	glite-wms-job-submit.1 \
	glite-wms-job-perusal.1 \
	glite-wms-job-info.1 \
        glite-wms-job-logging-info.1 \
        glite-wms-job-status.1 

%.1: %.xml
	xsltproc $(GLITE_DB_MANPAGES_STYLESHEET) $<

install-data-hook:
	gzip $(DESTDIR)$(mandir)/man1/glite-wms-*.1

MAINTAINERCLEANFILES = Makefile.in
