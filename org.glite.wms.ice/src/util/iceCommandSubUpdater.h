/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE subscription updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
 
#ifndef GLITE_WMS_ICE_ICECOMMANDSUBUPD_H
#define GLITE_WMS_ICE_ICECOMMANDSUBUPD_H

#include "iceAbsCommand.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"

#include <boost/scoped_ptr.hpp>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
        class subscriptionManager;
      }
      }
      }
      };

namespace log4cpp {
  class Category;
};

namespace glite {
namespace wms {

namespace common {
namespace configuration {
    class Configuration;
} // namespace configuration
} // namespace common

namespace ice {
     
// Forward declarations
namespace util {                
    class iceConfManager;
    class iceLBLogger;

     
  
 class iceCommandSubUpdater : public iceAbsCommand {
   std::string m_proxyfile;
   subscriptionManager *m_subMgr;
   
   
   //boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
   //glite::wms::ice::util::CreamJob* m_theJob;
   log4cpp::Category *m_log_dev;
   
   iceConfManager *m_conf;
   
//   time_t m_threshold_time;
   //time_t m_delay;
//   glite::wms::ice::util::iceLBLogger* m_lb_logger;
//   void killJob( glite::wms::ice::util::CreamJob&, const time_t );

   void retrieveCEURLs( std::set<std::string>& );
   void renewSubscriptions( std::vector<Subscription>& vec );

  public:
   iceCommandSubUpdater( const std::string& certfile ) throw();
   
   virtual void execute( ) throw( );
   
   virtual ~iceCommandSubUpdater() { }
   
   std::string get_grid_job_id( void ) const { return ""; }
 };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite


#endif
