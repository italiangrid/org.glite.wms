/*
 	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H
#define GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H

#include <vector>

// DagAd
#include "server/wmpexpdagad.h"
#include "glite/wms/jdl/JobAd.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"

// LB logger API
#include "glite/lb/consumer.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {


class WMPEventLogger  {

	public:
		enum event_name {
			LOG_ACCEPT,
			LOG_CANCEL,
			LOG_CLEAR,
			LOG_ABORT
		};
		WMPEventLogger();
		virtual ~WMPEventLogger() throw();

		void init(const std::string &nsHost, int nsPort,
			glite::wmsutils::jobid::JobId *id);

		void setLBProxy( bool value) {lbProxy_b=value;}
		bool getLBProxy() {return lbProxy_b;}
		std::string getSequence();
		
		void registerProxyRenewal(const std::string &proxy_path,
			const std::string &my_proxy_server);
		void unregisterProxyRenewal();
			
		void registerJob(glite::wms::jdl::JobAd *ad);
		std::vector<std::string> registerDag(WMPExpDagAd *ad);
		std::vector<std::string> registerPartitionable(WMPExpDagAd *ad, int res_num);


		// Event logging
		bool logEvent(event_name event,  const char* reason);
		void logEvent(event_name event,  const char* reason, bool retry);
		void logEvent(event_name  event, const char* reason, bool retry, bool test);

		void logUserTag(std::string name, const std::string &value);
		void logUserTags(classad::ClassAd *userTags);
		void logUserTags(std::vector<std::pair<std::string,
			classad::ExprTree*> > userTags);

		void logEnqueuedJob(std::string jdl, const std::string &file_queue, bool mode,
			const char *reason, bool retry );
		void logEnqueuedJob(std::string jdl, const std::string &proxy_path,
			const std::string &host_cert, const std::string &host_key, const std::string &file_queue,
			bool mode, const char *reason, bool retry, bool test);
		void setLoggingJob( const std::string &jid , const char* seq_code=NULL );


		
	private:
		void registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs);
		void testAndLog( int &code, bool &with_hp, int &lap, const std::string &host_cert, const std::string &host_key);
		int setX509Param(const std::string &name , const std::string &value ,  edg_wll_ContextParam lbX509code );
		void reset_user_proxy( const std::string &proxy_path );
		const char * error_message(const char *api);
		std::string dest_uri;
		glite::wmsutils::jobid::JobId *id;
		edg_wll_Context ctx;
		std::string lb_host;
		int lb_port;
		bool lbProxy_b ;
		static const char *GLITE_WMS_LOG_DESTINATION;
		static const int LB_RENEWAL_PORT = 7512;
		static const int LOG_RETRY_COUNT = 3;
};

typedef WMPEventLogger WMPLogger;

} // eventlogger
} // wmproxy
} // wms
} // glite

#endif // GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H
