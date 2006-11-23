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
 * ICE command submit class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICECOMMANDSUBMIT_H
#define GLITE_WMS_ICE_ICECOMMANDSUBMIT_H

#include "iceAbsCommand.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "creamJob.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/Topic.h"
#include "glite/ce/monitor-client-api-c/Policy.h"

#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"

#include <boost/scoped_ptr.hpp>

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
}
     
  
 class iceCommandSubmit : public iceAbsCommand {
     
 private:
     void  doSubscription( const std::string& );
     
     //boost::scoped_ptr< Ice > m_theIce;
     Ice *m_theIce;
     boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
     //glite::ce::cream_client_api::soap_proxy::CreamProxy* m_theProxy;
     
 public:
     iceCommandSubmit( Ice*, glite::ce::cream_client_api::soap_proxy::CreamProxy*, const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
     
     virtual ~iceCommandSubmit() { if(m_theProxy) delete( m_theProxy ); }
     
     virtual void execute( /* Ice* ice, glite::ce::cream_client_api::soap_proxy::CreamProxy* theProxy */) throw( iceCommandFatal_ex&, iceCommandTransient_ex& );
     
     std::string get_grid_job_id( void ) const { return m_theJob.getGridJobID(); };
     
 protected:
     
     // Inner class definition, used to manipulate paths
     class pathName {
     public:
         typedef enum { invalid=-1, absolute, uri, relative } pathType_t;
         
         pathName( const std::string& p );
         virtual ~pathName( ) { }
         // accessors
         pathType_t getPathType( void ) const { return m_pathType; }
         const std::string& getFullName( void ) const { return m_fullName; }
         const std::string& getPathName( void ) const { return m_pathName; }
         const std::string& getFileName( void ) const { return m_fileName; }
         
     protected:
         
         log4cpp::Category* m_log_dev;
         const std::string m_fullName;
         pathType_t m_pathType;
         std::string m_pathName;
         std::string m_fileName;
     };
     
     /**
      * This method is used to transform a "standard" jdl into the
      * format expected by CREAM. In particular: the mandatory (for
      * CREAM) attributes QueueName and BatchSystem are
      * added. Moreover, the Input and Output sandbox attributes are
      * modified.
      *
      * @param oldJdl the original jdl
      * @retyrn the CREAM-compliand jdl
      */
     std::string creamJdlHelper( const std::string& oldJdl ) throw( glite::wms::ice::util::ClassadSyntax_ex& );
     
     /**
      * This function updates the "InputSandbox" attribute value on
      * the jdl passed as parameter.
      *
      * @param jdl the original jdl, which will be modified
      * by this function
      */
     void updateIsbList( classad::ClassAd* jdl );
     
     /**
      * This function updates the "OutputSandbox"-related
      * attribute value on the jdl passed as parameter.
      *
      * @param jdl the original jdl, which will be modified
      * by this function
      */
     void updateOsbList( classad::ClassAd* jdl );
     
     std::string m_myname;	
     std::string m_jdl;
     util::CreamJob m_theJob;
     log4cpp::Category* m_log_dev;
     // glite::wms::ice::util::iceConfManager* m_confMgr;
     glite::wms::common::configuration::Configuration* m_configuration;
     std::string m_myname_url;
     util::iceLBLogger *m_lb_logger;
};

} // namespace ice
} // namespace wms
} // namespace glite

#endif
