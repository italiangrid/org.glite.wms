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
    class iceLBLogger;
    class Request;
}
     
  
 class iceCommandSubmit : public iceAbsCommand {
     
 private:
     void  doSubscription( const glite::wms::ice::util::CreamJob& );
     
     Ice *m_theIce;
     
 public:
     iceCommandSubmit( util::Request* request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
     
     virtual ~iceCommandSubmit() { }
     
     /**
      * This method is invoked to execute this command.
      */
     virtual void execute( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& );
     
     std::string get_grid_job_id( void ) const { return m_theJob.getGridJobID(); };

     static boost::recursive_mutex s_localMutexForSubscriptions;
     
 protected:

     /**
      * This method is called only by the execute() method. This
      * method does the real work of submitting the job. If something
      * fails, this method raises an exception. The caller ( execute()
      * ) must take care of the post-failure operations, that is
      * logging the appropriate events to LB and try to resubmit.
      */
     void try_to_submit( void ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& );
     
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
     glite::wms::common::configuration::Configuration* m_configuration;
     std::string m_myname_url;
     util::iceLBLogger *m_lb_logger;
     util::Request* m_request;
};

} // namespace ice
} // namespace wms
} // namespace glite

#endif
