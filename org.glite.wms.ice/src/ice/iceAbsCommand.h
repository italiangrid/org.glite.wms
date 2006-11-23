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
 * ICE abstract command class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICEABSCOMMAND_H
#define GLITE_WMS_ICE_ICEABSCOMMAND_H

#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"
#include "JobRequest_ex.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"

// forward declaration
namespace glite {
    namespace ce {
        namespace cream_client_api {
            namespace soap_proxy {
                class CreamProxy;
            }
        }
    }
}

namespace glite {
    namespace wms {
        namespace ice {

            class Ice;

            class iceAbsCommand {
	
            public:

                virtual ~iceAbsCommand() { };

                /**
                 * Executes the command. 
                 *
                 * @param _ice a pointer to an instance of the Ice
                 * class; caller retains ownership of this pointer.
                 *
                 * @param theProxy pointer to an instance of an
                 * already allocated CreamProxy data structure. Caller
                 * retains ownership of this pointer.
                 *
                 * @throw an iceCommandFatal_ex if the command is to
                 * be considered permanently failed;
                 * @throw an iceCommandTransient_ex if the command failed
                 * but could be tried again and succeed.
                 */
                virtual void execute( /* Ice* _ice, glite::ce::cream_client_api::soap_proxy::CreamProxy* theProxy */ ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& ) = 0;

                /**
                 * Returns the Grid jobID for the job this command
                 * refers to.
                 */
                virtual std::string get_grid_job_id( void ) const = 0;
          
            protected:

                iceAbsCommand( ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&) {};

            };
        }
    }
}

#endif
