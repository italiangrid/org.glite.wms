//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.cpp
//
// See also:
// org.glite.lb.client-interface/build/producer.h
// org.glite.lb.client-interface/doc/C/latex/refman.pdf

#include "iceLBEvent.h"
#include "iceLBContext.h"

using namespace glite::wms::ice::util;

//////////////////////////////////////////////////////////////////////////////
//
// iceLBEvent
//
//////////////////////////////////////////////////////////////////////////////
iceLBEvent::iceLBEvent( const CreamJob& j, edg_wll_Source src, const std::string& dsc ) :
    _job( j ),
    _src( src ),
    _description( dsc )
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Cream Transfer Start Event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_start_event::cream_transfer_start_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Transfer Start Event" )
{

}

int cream_transfer_start_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogTransferSTART( *(ctx->el_context), 
                                     EDG_WLL_SOURCE_LRMS, 
                                     _job.getCreamURL().c_str(),
                                     ctx->el_s_unavailable,
                                     _job.getJDL().c_str(),  
                                     ctx->el_s_unavailable,
                                     ctx->el_s_unavailable );
}

//////////////////////////////////////////////////////////////////////////////
//
// Cream Transfer OK event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_ok_event::cream_transfer_ok_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Transfer OK Event" )
{

}

int cream_transfer_ok_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogTransferOK( *(ctx->el_context), 
                                  EDG_WLL_SOURCE_LRMS, 
                                  _job.getCreamURL().c_str(),
                                  ctx->el_s_unavailable,
                                  _job.getJDL().c_str(),  
                                  ctx->el_s_unavailable,
                                  _job.getJobID().c_str() );    
}

//////////////////////////////////////////////////////////////////////////////
//
// transfer FAIL event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_fail_event::cream_transfer_fail_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Transfer Fail Event" ),
    _reason( reason )
{

}

int cream_transfer_fail_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogTransferFAIL( *(ctx->el_context), 
                                    EDG_WLL_SOURCE_LRMS, 
                                    _job.getCreamURL().c_str(),
                                    ctx->el_s_unavailable,
                                    _job.getJDL().c_str(),  
                                    _reason.c_str(), 
                                    ctx->el_s_unavailable );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream accepted event
//
//////////////////////////////////////////////////////////////////////////////
cream_accepted_event::cream_accepted_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Accepted Event" )
{

}

int cream_accepted_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogAccepted( *(ctx->el_context), 
                                EDG_WLL_SOURCE_JOB_SUBMISSION, 
                                ctx->el_s_localhost_name.c_str(),
                                ctx->el_s_unavailable,
                                _job.getJobID().c_str()
                                );
}

//////////////////////////////////////////////////////////////////////////////
//
// lrms accepted event
//
//////////////////////////////////////////////////////////////////////////////
lrms_accepted_event::lrms_accepted_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "LRMS Accepted Event" )
{

}

int lrms_accepted_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogAccepted( *(ctx->el_context), 
                                EDG_WLL_SOURCE_LRMS,
                                _job.getCEID().c_str(),
                                ctx->el_s_unavailable,
                                _job.getJobID().c_str()
                                );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream refused event
//
//////////////////////////////////////////////////////////////////////////////
cream_refused_event::cream_refused_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Cream Refused Event" ),
    _reason( reason )
{

}

int cream_refused_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogRefused( *(ctx->el_context), 
                               EDG_WLL_SOURCE_JOB_SUBMISSION,
                               ctx->el_s_unavailable,
                               ctx->el_s_unavailable,
                               _reason.c_str() );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream cancel request event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_request_event::cream_cancel_request_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Cancel Request Event" )
{

}

int cream_cancel_request_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogCancelREQ( *(ctx->el_context), 
                                 ctx->el_s_unavailable 
                                 );
}



//////////////////////////////////////////////////////////////////////////////
//
// cancel refuse event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_refuse_event::cream_cancel_refuse_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Cancel Refuse Event" ),
    _reason( reason )
{

}

int cream_cancel_refuse_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogCancelREFUSE( *(ctx->el_context), 
                                    _reason.c_str()
                                    );
}


//////////////////////////////////////////////////////////////////////////////
//
// job running event
//
//////////////////////////////////////////////////////////////////////////////
job_running_event::job_running_event( const CreamJob& j, const std::string& host ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Runnign Event" ),
    _host( host )
{

}

int job_running_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogRunning( *(ctx->el_context), _host.c_str() );
}


//////////////////////////////////////////////////////////////////////////////
//
// job cancelled event
//
//////////////////////////////////////////////////////////////////////////////
job_cancelled_event::job_cancelled_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Cancelled Event" )
{

}

int job_cancelled_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogDoneCANCELLED( *(ctx->el_context), 
                                     ctx->el_s_unavailable, 
                                     0 ); // FIXME
}


//////////////////////////////////////////////////////////////////////////////
//
// job syspended event
//
//////////////////////////////////////////////////////////////////////////////
job_suspended_event::job_suspended_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Suspended Event" )
{

}

int job_suspended_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogSuspend( *(ctx->el_context), ctx->el_s_unavailable );
}

//////////////////////////////////////////////////////////////////////////////
//
// job done ok event
//
//////////////////////////////////////////////////////////////////////////////
job_done_ok_event::job_done_ok_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Done Ok Event" )
{

}

int job_done_ok_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogDoneOK( *(ctx->el_context), 
                              ctx->el_s_unavailable, 0 ); // FIXME
}

//////////////////////////////////////////////////////////////////////////////
// 
// job done failed event
//
//////////////////////////////////////////////////////////////////////////////
job_done_failed_event::job_done_failed_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Done Failed Event" )
{

}

int job_done_failed_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogDoneFAILED( *(ctx->el_context), 
                                  ctx->el_s_unavailable, 0 ); // FIXME
}


//////////////////////////////////////////////////////////////////////////////
//
// ns enqueued start event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_start_event::ns_enqueued_start_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "NS Enqueued Start Event" ),
    _qname( qname )
{

}

int ns_enqueued_start_event::execute( iceLBContext* ctx ) 
{
    return edg_wll_LogEnQueuedSTART( *(ctx->el_context), 
                                     _qname.c_str(),
                                     _job.getJobID().c_str(),
                                     ctx->el_s_unavailable
                                     );
}


//////////////////////////////////////////////////////////////////////////////
//
// ns enqueued fail event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_fail_event::ns_enqueued_fail_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "NS Enqueued Fail Event" ),
    _qname( qname )
{

}

int ns_enqueued_fail_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogEnQueuedFAIL( *(ctx->el_context), 
                                    _qname.c_str(),
                                    _job.getJobID().c_str(),
                                    ctx->el_s_unavailable
                                    );
}

//////////////////////////////////////////////////////////////////////////////
//
// wms enqueued ok event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_ok_event::ns_enqueued_ok_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "NS Enqueued OK Event" ),
    _qname( qname )
{

}

int ns_enqueued_ok_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogEnQueuedOK( *(ctx->el_context), 
                                  _qname.c_str(),
                                  _job.getJobID().c_str(),
                                  ctx->el_s_unavailable
                                  );
}
