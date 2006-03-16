#ifndef  GLITE_WMS_UI_CLIENT_LOGGING_H
#define GLITE_WMS_UI_CLIENT_LOGGING_H
/*
 * Logging.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

 // DagAd
#include "glite/jdl/ExpDagAd.h"
#include "glite/jdl/JobAd.h"
// JobId
#include "glite/wmsutils/jobid/JobId.h"
// Lb logger API
#include "glite/lb/consumer.h"

namespace glite {
namespace wmsui {
namespace api {

class Logging  {
   public:
	enum txType {
		START,
		OK,
		FAIL
	};
	Logging( ) ;
	virtual ~Logging( )  throw() { edg_wll_FreeContext(ctx); };
	void init (  const std::string& nsHost , int nsPort , glite::wmsutils::jobid::JobId* id ) ;
	glite::jdl::ExpDagAd* registerJob( glite::jdl::JobAd* ad , int resource ) ;
	void registerJob( glite::jdl::JobAd* ad ) ;
	void registerDag( glite::jdl::ExpDagAd* ad   ) ;
	void transfer(  txType tx, const std::string& jdl ,  const char* error ="" ) ;
	std::string getSequence (){   return   std::string ( edg_wll_GetSequenceCode(ctx)  ) ;  };
	void logUserTags( classad::ClassAd* userTags ) ;
	void logUserTags( std::vector<  std::pair<  std::string  ,     classad::ExprTree* > > userTags  );	
	
   private:
	void registerSubJobs( glite::jdl::ExpDagAd* ad ,  edg_wlc_JobId* subjobs ) ;   
	const char* error_message (const char* api );
	edg_wll_Context ctx ;
	glite::wmsutils::jobid::JobId* id ;
	static pthread_mutex_t dgtransfer_mutex;
	std::string nsHost ;
	int nsPort ;
	bool jCollect ;
};

} // api
} // wmsui
} // glite

#endif
