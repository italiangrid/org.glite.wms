#ifndef  EDG_WORKLOAD_USERINTERFACE_CLIENT_LOGGING_H
#define EDG_WORKLOAD_USERINTERFACE_CLIENT_LOGGING_H
/*
 * Logging.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

 // DagAd
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/JobAd.h"
// JobId
#include "glite/wmsutils/jobid/JobId.h"
// Lb logger API
#include "glite/lb/consumer.h"
#include "glite/wms-ui/api/userinterface_namespace.h"

USERINTERFACE_NAMESPACE_BEGIN //Defining UserInterFace NameSpace
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
	glite::wms::jdl::ExpDagAd* registerJob( edg::workload::common::requestad::JobAd* ad , int resource ) ;
	void registerJob( glite::wms::jdl::JobAd* ad ) ;
	void registerDag( glite::wms::jdl::ExpDagAd* ad   ) ;
	void transfer(  txType tx, const std::string& jdl ,  const char* error ="" ) ;
	std::string getSequence (){   return   std::string ( edg_wll_GetSequenceCode(ctx)  ) ;  };
	void logUserTags( classad::ClassAd* userTags ) ;
	void logUserTags( std::vector<  std::pair<  std::string  ,     classad::ExprTree* > > userTags  );	
	
   private:
	void registerSubJobs( glite::wms::jdl::ExpDagAd* ad ,  edg_wlc_JobId* subjobs ) ;   
	const char* error_message (const char* api );
	edg_wll_Context ctx ;
	glite::wmsutils::jobid::JobId* id ;
	static pthread_mutex_t dgtransfer_mutex;
	std::string nsHost ;
	int nsPort ;
	bool jCollect ;
};
USERINTERFACE_NAMESPACE_END } //Closing  UserInterFace NameSpace
#endif
