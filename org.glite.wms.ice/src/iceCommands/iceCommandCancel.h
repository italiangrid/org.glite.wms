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
 * ICE command cancel class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICECOMMANDCANCEL_H
#define GLITE_WMS_ICE_ICECOMMANDCANCEL_H

#include <string>

#include "iceAbsCommand.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "filelist_request.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <boost/scoped_ptr.hpp>

namespace glite {
    namespace wms {
        namespace ice {

            namespace util {
                class iceLBLogger;                 // Forward declaration
            };

            class iceCommandCancel : public iceAbsCommand {
	    
	      boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
	      //glite::ce::cream_client_api::soap_proxy::CreamProxy* theProxy;
                
            public:
                iceCommandCancel( glite::ce::cream_client_api::soap_proxy::CreamProxy*, const filelist_request& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

                virtual ~iceCommandCancel() { }

                void execute( /* Ice* ice, glite::ce::cream_client_api::soap_proxy::CreamProxy* theProxy */) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& );          
                std::string get_grid_job_id( void ) const { return m_gridJobId; };

            protected:
	        
		std::string m_gridJobId;
                std::string m_sequence_code;
                log4cpp::Category* m_log_dev;
                util::iceLBLogger *m_lb_logger;
                filelist_request m_request;
            };
        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
