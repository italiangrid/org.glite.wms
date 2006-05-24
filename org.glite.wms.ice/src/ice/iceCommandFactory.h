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
 * ICE command factory
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICECOMMANDFACTORY_H
#define GLITE_WMS_ICE_ICECOMMANDFACTORY_H

#include "ClassadSyntax_ex.h"
#include "JobRequest_ex.h"
#include <string>

namespace glite {
    namespace wms {
        namespace ice {

            class iceAbsCommand;

            /**
             * This class is used to build an iceAbsCommand object
             * corresponding to the command serialized in the request
             * classad. This class implements the classic "factory"
             * design pattern.
             */
            class iceCommandFactory {
            public:
                virtual ~iceCommandFactory( ) { };
               
                /**
                 * Returns an object instance of iceAbsCommand class;
                 * the actual object class depends on the content of
                 * the "request" parameter. The caller owns the
                 * returned object, and is responsible for
                 * deallocating it.
                 *
                 * @param request the classad containing the
                 * serialized version of the command to be built.
                 *
                 * @return a dynamically allocated iceAbsCommand object.
                 */ 
                static iceAbsCommand* mkCommand( const std::string& request ) throw( glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);
                
            protected:
                iceCommandFactory( ) { };
                
            };
        }
    }
}

#endif
