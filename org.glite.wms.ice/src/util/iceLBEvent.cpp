//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.cpp
//
// See also:
// org.glite.lb.client-interface/build/producer.h
// org.glite.lb.client-interface/doc/C/latex/refman.pdf

#include "iceLBEvent.h"
#include "iceLBContext.h"
#include "glite/lb/producer.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

using namespace glite::wms::ice::util;

//////////////////////////////////////////////////////////////////////////////
//
// iceLBEvent
//
//////////////////////////////////////////////////////////////////////////////
iceLBEvent::iceLBEvent( ) :
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Cream Transfer Start Event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_start_event::cream_transfer_start_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void cream_transfer_start_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_transfer_start_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging CREAM transfer start, dest_host = " 
            << _job.getCreamURL()
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogTransferSTART( *(ctx->el_context), 
                                        EDG_WLL_SOURCE_LRMS, 
                                        _job.getCreamURL().c_str(),
                                        ctx->el_s_unavailable,
                                        _job.getJDL().c_str(),  
                                        ctx->el_s_unavailable,
                                        ctx->el_s_unavailable );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// Cream Transfer OK event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_ok_event::cream_transfer_ok_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void cream_transfer_ok_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_transfer_ok_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging cream transfer OK event, "
            << " jobid=" << _job.getGridJobID()
            << " dest_host=" << _job.getCreamURL()
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogTransferOK( *(ctx->el_context), 
                                     EDG_WLL_SOURCE_LRMS, 
                                     _job.getCreamURL().c_str(),
                                     ctx->el_s_unavailable,
                                     _job.getJDL().c_str(),  
                                     ctx->el_s_unavailable,
                                     _job.getJobID().c_str() );
        
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// transfer FAIL event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_fail_event::cream_transfer_fail_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( ),
    _job( j ),
    _reason( reason )
{

}

void cream_transfer_fail_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_transfer_fail_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Transfer FAIL event, "
            << " jobid=" << _job.getGridJobID()
            << " dest_host=" << _job.getCreamURL()
            << " reason=" << _reason
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogTransferFAIL( *(ctx->el_context), 
                                       EDG_WLL_SOURCE_LRMS, 
                                       _job.getCreamURL().c_str(),
                                       ctx->el_s_unavailable,
                                       _job.getJDL().c_str(),  
                                       _reason.c_str(), 
                                       ctx->el_s_unavailable );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream accepted event
//
//////////////////////////////////////////////////////////////////////////////
cream_accepted_event::cream_accepted_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void cream_accepted_event::execute( iceLBContext* ctx )
{
    // FIXME: which is the source? which is the destination?
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_accepted_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging CREAM Accepted event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogAccepted( *(ctx->el_context), 
                                   EDG_WLL_SOURCE_JOB_SUBMISSION, 
                                   ctx->el_s_localhost_name.c_str(),
                                   ctx->el_s_unavailable,
                                   _job.getJobID().c_str()
                                   );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// lrms accepted event
//
//////////////////////////////////////////////////////////////////////////////
lrms_accepted_event::lrms_accepted_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void lrms_accepted_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging lrms_accepted_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging LRMS Accepted event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogAccepted( *(ctx->el_context), 
                                   EDG_WLL_SOURCE_LRMS,
                                   _job.getCEID().c_str(),
                                   ctx->el_s_unavailable,
                                   _job.getJobID().c_str()
                                   );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream refused event
//
//////////////////////////////////////////////////////////////////////////////
cream_refused_event::cream_refused_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( ),
    _job( j ),
    _reason( reason )
{

}


void cream_refused_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_refused_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Refused event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogRefused( *(ctx->el_context), 
                                  EDG_WLL_SOURCE_JOB_SUBMISSION,
                                  ctx->el_s_unavailable,
                                  ctx->el_s_unavailable,
                                  _reason.c_str() );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream cancel request event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_request_event::cream_cancel_request_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void cream_cancel_request_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_cancel_request_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Cancel Request event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogCancelREQ( *(ctx->el_context), 
                                    ctx->el_s_unavailable 
                                    );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}



//////////////////////////////////////////////////////////////////////////////
//
// cancel refuse event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_refuse_event::cream_cancel_refuse_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( ),
    _job( j ),
    _reason( reason )
{

}

void cream_cancel_refuse_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging cream_cancel_refuse_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging Cancel Refuse event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogCancelREFUSE( *(ctx->el_context), 
                                       _reason.c_str()
                                       );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}


//////////////////////////////////////////////////////////////////////////////
//
// job running event
//
//////////////////////////////////////////////////////////////////////////////
job_running_event::job_running_event( const CreamJob& j, const std::string& host ) :
    iceLBEvent( ),
    _job( j ),
    _host( host )
{

}

void job_running_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging job_running_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job running event, host = " << _host
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogRunning( *(ctx->el_context), _host.c_str() );
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}


//////////////////////////////////////////////////////////////////////////////
//
// job cancelled event
//
//////////////////////////////////////////////////////////////////////////////
job_cancelled_event::job_cancelled_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void job_cancelled_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging job_cancelled_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job cancelled event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogDoneCANCELLED( *(ctx->el_context), 
                                        ctx->el_s_unavailable, 
                                        0 ); // FIXME
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}


//////////////////////////////////////////////////////////////////////////////
//
// job syspended event
//
//////////////////////////////////////////////////////////////////////////////
job_suspended_event::job_suspended_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void job_suspended_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging job_suspended_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job suspended event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogSuspend( *(ctx->el_context), ctx->el_s_unavailable );
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// job done ok event
//
//////////////////////////////////////////////////////////////////////////////
job_done_ok_event::job_done_ok_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void job_done_ok_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging job_done_ok_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job done_ok event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogDoneOK( *(ctx->el_context), 
                                 ctx->el_s_unavailable, 0 ); // FIXME
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
// 
// job done failed event
//
//////////////////////////////////////////////////////////////////////////////
job_done_failed_event::job_done_failed_event( const CreamJob& j ) :
    iceLBEvent( ),
    _job( j )
{

}

void job_done_failed_event::execute( iceLBContext* ctx )
{
    int res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_LOG_MONITOR );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging job_done_failed_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();

    do {
        log_dev->infoStream() 
            << "Logging job done_failed event..."
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogDoneFAILED( *(ctx->el_context), 
                                     ctx->el_s_unavailable, 0 ); // FIXME
        
        log_dev->infoStream() 
            << "...Got return code " << res 
            << log4cpp::CategoryStream::ENDLINE;
        
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );    
}


//////////////////////////////////////////////////////////////////////////////
//
// ns enqueued start event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_start_event::ns_enqueued_start_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( ),
    _job( j ),
    _qname( qname )
{

}

void ns_enqueued_start_event::execute( iceLBContext* ctx ) 
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging ns_enqueued_start_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging NS enqueued start event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << " queue=[" << _qname << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogEnQueuedSTART( *(ctx->el_context), 
                                        _qname.c_str(),
                                        _job.getJobID().c_str(),
                                        ctx->el_s_unavailable
                                        );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}


//////////////////////////////////////////////////////////////////////////////
//
// ns enqueued fail event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_fail_event::ns_enqueued_fail_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( ),
    _job( j ),
    _qname( qname )
{

}

void ns_enqueued_fail_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging ns_enqueued_fail_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging NS enqueued fail event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << " queue=[" << _qname << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogEnQueuedFAIL( *(ctx->el_context), 
                                       _qname.c_str(),
                                       _job.getJobID().c_str(),
                                       ctx->el_s_unavailable
                                       );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}

//////////////////////////////////////////////////////////////////////////////
//
// wms enqueued ok event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_ok_event::ns_enqueued_ok_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( ),
    _job( j ),
    _qname( qname )
{

}

void ns_enqueued_ok_event::execute( iceLBContext* ctx )
{
    int           res;

    try {
        ctx->setLoggingJob( _job, EDG_WLL_SOURCE_JOB_SUBMISSION );
    } catch( iceLBException& ex ) {
        log_dev->errorStream()
            << "Error logging ns_enqueued_ok_event: "
            << ex.what()
            << log4cpp::CategoryStream::ENDLINE;
        return;
    }
    
    ctx->startLogging();
    
    do {
        log_dev->infoStream() 
            << "Logging NS enqueued ok event, "
            << " jobid=[" << _job.getGridJobID() << "]"
            << " queue=[" << _qname << "]"
            << log4cpp::CategoryStream::ENDLINE;
        
        res = edg_wll_LogEnQueuedOK( *(ctx->el_context), 
                                     _qname.c_str(),
                                     _job.getJobID().c_str(),
                                     ctx->el_s_unavailable
                                     );
        log_dev->infoStream() << "...Got return code " << res 
                              << log4cpp::CategoryStream::ENDLINE;
        ctx->testCode( res );
    } while( res != 0 );        

    ctx->update_and_store_job( _job );
}
