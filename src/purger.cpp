// File: purger.cpp
// Author: Salvatore Monforte <salvatore.monforte@ct.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//
// $Id$

#include <boost/filesystem/operations.hpp> 
#include <boost/filesystem/exception.hpp>

#include "edg/workload/common/jobid/JobId.h"
#include "edg/workload/common/jobid/manipulation.h"
#include "edg/workload/common/jobid/JobIdExceptions.h"

#include "edg/workload/common/configuration/Configuration.h"
#include "edg/workload/common/configuration/NSConfiguration.h"
#include "edg/workload/common/configuration/exceptions.h"

#include "edg/workload/common/logger/edglog.h"
#include "edg/workload/common/logger/manipulators.h"

#include "edg/workload/logging/client/Job.h"
#include "edg/workload/logging/client/JobStatus.h"
#include "edg/workload/logging/client/context.h"
#include "edg/workload/logging/client/producer.h"

#include <string>
#include <time.h>

namespace fs            = boost::filesystem;
namespace jobid         = edg::workload::common::jobid;
namespace logger	= edg::workload::common::logger::threadsafe;
namespace logging       = edg::workload::logging;
namespace configuration = edg::workload::common::configuration;

#define edglog_fn(name) edg::workload::common::logger::StatePusher    pusher(logger::edglog, #name);

namespace edg {
namespace workload {
namespace purger {

namespace 
{
  const configuration::NSConfiguration* f_ns_conf = 0;
}

void purgeQuota(const fs::path& p)
{
  std::string uid;
  fs::path quotafile(p << ".quid");
  std::ifstream quidfilestream( quotafile.file_path().c_str() );
  if( quidfilestream ) {
      quidfilestream >> uid;
      std::string cmdline("edg-wl-quota-adjustment -d - ");
      cmdline.append(uid);
      if (!system(cmdline.c_str())) {
	logger::edglog << " Error during quota purging." << std::endl;
      }	
  } 
}
	
bool purgeStorage(const jobid::JobId& jobid, const std::string& sandboxdir)
{
  edglog_fn(purgeStorage);	
  fs::path p;
  try {
	  if( sandboxdir.empty() ) {
		if( !f_ns_conf ) {
		  f_ns_conf = configuration::Configuration::instance() -> ns();
		}
		p = fs::path(f_ns_conf->sandbox_staging_path(), fs::system_specific);
	  }
	  else p = fs::path(sandboxdir, fs::system_specific);
	  p <<= jobid::get_reduced_part( jobid );
	  p <<= jobid::to_filename( jobid );
  }
  catch( jobid::JobIdException& jide )
  {
    logger::edglog << jide.what() << std::endl;
    return false; 	
  }
  catch( fs::filesystem_error& fse )
  {
     logger::edglog << fse.what() << std::endl;
     return false;	
  }
  
  if (fs::remove_all( p )) {
    purgeQuota( p ); 
    return true;
  }
  return false;
}


bool purgeStorageEx(const fs::path& p, int purge_threshold, bool fake_rm)
{
  edglog_fn(purgeStorageEx);	
  bool result = false;
  
  try {
			
    jobid::JobId jobid( jobid::from_filename( p.leaf() ) ); 	
    logging::client::JobStatus job_status = logging::client::Job( jobid ).status(0);
    std::string dg_jobid( jobid.toString() );
    std::string seqcode;
    logger::edglog << dg_jobid << " ->";
    fs::path     seqfile( p << ".edg_wll_seq" );
    std::ifstream seqfilestream( seqfile.file_path().c_str() );
    if( seqfilestream ) {
      seqfilestream >> seqcode;
    }	
    else {
      logger::edglog << " .edg_wll_seq does not exist " << std::endl; 
      bool ret = (!fake_rm && fs::remove_all( p ));
      purgeQuota( p );
      return ret;
    }
    //                        
    // Initialize the logging context
    // 
    edg_wll_Context      log_ctx;
    if( edg_wll_InitContext( &log_ctx ) != 0 ||
	edg_wll_SetParam( log_ctx, EDG_WLL_PARAM_SOURCE,  EDG_WLL_SOURCE_NETWORK_SERVER  ) != 0 ||
	edg_wll_SetLoggingJob( log_ctx, jobid, seqcode.c_str(), EDG_WLL_SEQ_NORMAL ) != 0 )  {
	  
      char 	*et,*ed;
      edg_wll_Error(log_ctx,&et,&ed);

      logger::edglog << " unable to initialize logging context " << et << ": " << ed;
      free(et); free(ed);
    }	  
    
    switch( job_status.status ) {
  
    case logging::client::JobStatus::CLEARED:	
			
      logger::edglog << " removing";
      
      if( !fake_rm && fs::remove_all( p ) ) {	
	
        result = true;
	
	if( edg_wll_LogClearTIMEOUT( log_ctx ) != 0) logger::edglog << ": edg_wll_LogClearTIMEOUT failed";
	else {
		logger::edglog << ": Ok";
	}
      }
      else {
        logger::edglog << ": fail";	
      }		
      break;
    case logging::client::JobStatus::ABORTED:
    case logging::client::JobStatus::DONE: 
      {
	struct timeval last_update_time = 
	  job_status.getValTime(logging::client::JobStatus::LAST_UPDATE_TIME);
        time_t now;
	time( &now);
	
	if( now -  last_update_time.tv_sec > purge_threshold ) {		
	  
	  logger::edglog << " removing";
	  
	  if( !fake_rm && fs::remove_all( p ) ) {

      	    result = true;
  	    
	    if( edg_wll_LogClearTIMEOUT( log_ctx ) !=0 ) logger::edglog << ": edg_wll_LogClearTIMEOUT failed";
	    else {
		    logger::edglog << ": Ok";
	    }
	  }
	  else {	
	    logger::edglog << ": fail"; 
	  } 	
	}
	else logger::edglog << " skipping ";	
      }
    break;
    default: ; logger::edglog << " skipping ";
    }
    logger::edglog << std::endl;   
  }
  catch( fs::filesystem_error& fse ) 
    {
      logger::edglog << fse.what() << std::endl;
    }
  catch( jobid::JobIdException& jide )
    {
      logger::edglog << jide.what() << std::endl;
    }
  catch( std::exception&e ) 
    {
      logger::edglog << e.what() << std::endl;
    }
  if (result) purgeQuota( p ); 
  return result;
}

} // namespace purger
} // namespace workload
} // namesapce edg

