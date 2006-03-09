#include "iceLBEventFactory.h"
#include "iceLBEvent.h"

#include <string>

using namespace glite::wms::ice::util;

iceLBEvent* iceLBEventFactory::mkEvent( const CreamJob& theJob )
{
    switch( theJob.getStatus() ) {
    case glite::ce::cream_client_api::job_statuses::PENDING:
        return new cream_accepted_event( theJob );
    case glite::ce::cream_client_api::job_statuses::IDLE:
        return new lrms_accepted_event( theJob );
    case glite::ce::cream_client_api::job_statuses::RUNNING:
        return new job_running_event( theJob, std::string( "unavailable" ) ); // FIXME
    case glite::ce::cream_client_api::job_statuses::CANCELLED:
        return new job_cancelled_event( theJob );
    case glite::ce::cream_client_api::job_statuses::HELD:
        return new job_suspended_event( theJob );
    case glite::ce::cream_client_api::job_statuses::ABORTED:
        return new job_done_failed_event( theJob );
    case glite::ce::cream_client_api::job_statuses::DONE_OK:
        return new job_done_ok_event( theJob );
    case glite::ce::cream_client_api::job_statuses::DONE_FAILED:
        return new job_done_failed_event( theJob );
    default:
        return 0;
    }
}
