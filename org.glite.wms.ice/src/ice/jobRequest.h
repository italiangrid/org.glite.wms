
#ifndef __GLITE_WMS_ICE_JOBREQ_H__
#define __GLITE_WMS_ICE_JOBREQ_H__

#include "ClassadSyntax_ex.h"
#include "JobRequest_ex.h"
#include "classad_distribution.h"

namespace glite {
  namespace wms {
    namespace ice {

      class jobRequest {
	
      public:
	
	enum requestSupportedCommand {
	  jobsubmit,
	  jobcancel,
	  unknown
	};

	jobRequest() : command(unknown), grid_jobid(""), jdl(""), certfile("") { }
	jobRequest(const std::string& request) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
	virtual ~jobRequest() {}
	
	requestSupportedCommand getCommand() { return command; }
	std::string             getUserJDL() { return jdl;}
	std::string             getGridJobID() { return grid_jobid; }
	std::string             getProxyCertificate() { return certfile; }
	void                    unparse(const std::string& request) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
	
      private:
	requestSupportedCommand command;
	std::string grid_jobid;
	std::string jdl;
	std::string certfile;
	classad::ClassAdParser parser;
	classad::ClassAdUnParser unp;
      };
    }
  }
}

#endif
