/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPLOGGER_H
#define GLITE_WMS_WMPROXY_WMPLOGGER_H

// DagAd
#include "wmpexpdagad.h"
#include "glite/wms/jdl/JobAd.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"

// LB logger API
#include "glite/lb/consumer.h"

//namespace glite {
//namespace wms {
//namespace wmproxy {



class WMPLogger  {

	public:
		enum log_type {
			ACCEPTED,
			ABORT,
			REFUSED
		};

		WMPLogger();
		virtual ~WMPLogger() throw();

		void init(const std::string &nsHost, int nsPort, glite::wmsutils::jobid::JobId *id);

		void registerJob(glite::wms::jdl::JobAd *ad);
		void registerDag(WMPExpDagAd *ad);
		void registerDag(WMPExpDagAd *ad, int res_num);

		void log(log_type tx, const std::string &jid, const char *reason = "");

		std::string getSequence();

		void logUserTags(classad::ClassAd *userTags);
		void logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> > userTags);
		
		void setDestinationURI(std::string dest_uri);
		
		void logOriginalJdl(const std::string &jdl);
		
	private:
		void registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs);
		const char * error_message(const char *api);
		std::string dest_uri;
		glite::wmsutils::jobid::JobId *id;
		edg_wll_Context ctx;
		//std::string ll_host;
		std::string lb_host;
		int lb_port;
};

//} // wmproxy
//} // wms
//} // glite

#endif // GLITE_WMS_WMPROXY_WMPLOGGER_H
