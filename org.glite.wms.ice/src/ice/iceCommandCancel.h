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

#include "iceAbsCommand.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

namespace glite {
    namespace wms {
        namespace ice {

            namespace util {
                class iceLBLogger;                 // Forward declaration
            };

            class iceCommandCancel : public iceAbsCommand {
	
            public:
                iceCommandCancel( const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

                virtual ~iceCommandCancel() {};

                void execute( Ice* ice ) throw ( iceCommandFatal_ex&, iceCommandTransient_ex& );          
            protected:
                std::string m_gridJobId;
                std::string m_sequence_code;
                log4cpp::Category* m_log_dev;
                util::iceLBLogger *m_lb_logger;
            };
        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
