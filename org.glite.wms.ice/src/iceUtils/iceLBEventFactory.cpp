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
 * ICE Event Factory class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceLBEventFactory.h"
#include "iceLBEvent.h"

#include <string>

using namespace glite::wms::ice::util;
namespace jobstat = glite::ce::cream_client_api::job_statuses;

iceLBEvent* iceLBEventFactory::mkEvent( const CreamJob& theJob )
{
    switch( theJob.getStatus() ) {
    case jobstat::PENDING:
    case jobstat::IDLE:
        // nothing to log
        return 0;
    case jobstat::RUNNING:
        return new job_running_event( theJob ); // FIXME
    case jobstat::REALLY_RUNNING:
        return new job_really_running_event( theJob );
    case jobstat::CANCELLED:
        if ( theJob.is_killed_by_ice() )
            return new job_aborted_event( theJob );
        else
            return new job_cancelled_event( theJob );
    case jobstat::HELD:
        // ice should NEVER see this event!
        return 0;
    case jobstat::ABORTED:
        return new job_done_failed_event( theJob );
    case jobstat::DONE_OK:
        return new job_done_ok_event( theJob );
    case jobstat::DONE_FAILED:
        return new job_done_failed_event( theJob );
    default:
        return 0;
    }
}
