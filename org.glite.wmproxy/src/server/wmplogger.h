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

		WMPLogger();
		virtual ~WMPLogger() throw();

		void init(const std::string &nsHost, int nsPort, 
			glite::wmsutils::jobid::JobId *id);

		std::string getSequence();
		
		void registerJob(glite::wms::jdl::JobAd *ad);
		void registerDag(WMPExpDagAd *ad);
		void registerPartitionable(WMPExpDagAd *ad, int res_num);

		void logAccepted(const std::string &jid);
		void logRefused(const std::string &jid);
		void logAbort(const char *reason = "");

		void logUserTag(std::string name, const std::string &value);
		void logUserTags(classad::ClassAd *userTags);
		void logUserTags(std::vector<std::pair<std::string, 
			classad::ExprTree*> > userTags);
		
		void setDestinationURI(std::string dest_uri);
		
		static const char *GLITE_WMS_LOG_DESTINATION;
		
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
