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
 * ICE LB Events factory
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef ICELBEVENTFACTORY_H
#define ICELBEVENTFACTORY_H

#include "creamJob.h"

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                class iceLBEvent; // forward declaration

                /**
                 * This class is a factory used to build a logging
                 * event corresponding to a job status change.
                 */
                class iceLBEventFactory {
                public:
                    virtual ~iceLBEventFactory( ) { };

                    /**
                     * Factory method used to create an iceLBEvent
                     * object which corresponds to the last (most
                     * recent) status change for job j. The caller
                     * owns the pointer which is returned; thus, the
                     * caller is responsible for freeing the pointer
                     * when necessary.
                     *
                     * @param j the job whose more recent status is to
                     * be logged 
                     *
                     * @return a logging event corresponding to the
                     * most recent status of job j; if the current job
                     * status for j does not correspond to any valid
                     * LB event, then the null pointer is returned.
                     * The caller owns the returned pointer, and is
                     * thus responsible for relinquishing it.
                     */
                    static iceLBEvent* mkEvent( const CreamJob& j );
                protected:
                    iceLBEventFactory( ) { };
                };

            } // namespace util
        } // namespace ice
    } // namespace wms
} // namespace glite

#endif
