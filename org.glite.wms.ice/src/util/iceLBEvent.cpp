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
 * ICE Logging&Bookeeping events
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */


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
    m_job( j ),
    m_src( src ),
    m_description( dsc )
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
                                     m_job.getCreamURL().c_str(),
                                     ctx->el_s_unavailable,
                                     m_job.getJDL().c_str(),  
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
                                  m_job.getCreamURL().c_str(),
                                  ctx->el_s_unavailable,
                                  m_job.getJDL().c_str(),  
                                  ctx->el_s_unavailable,
                                  m_job.getJobID().c_str() );    
}

//////////////////////////////////////////////////////////////////////////////
//
// transfer FAIL event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_fail_event::cream_transfer_fail_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, "Cream Transfer Fail Event" ),
    m_reason( reason )
{

}

int cream_transfer_fail_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogTransferFAIL( *(ctx->el_context), 
                                    EDG_WLL_SOURCE_LRMS, 
                                    m_job.getCreamURL().c_str(),
                                    ctx->el_s_unavailable,
                                    m_job.getJDL().c_str(),  
                                    m_reason.c_str(), 
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
                                m_job.getJobID().c_str()
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
                                m_job.getCEID().c_str(),
                                ctx->el_s_unavailable,
                                m_job.getJobID().c_str()
                                );
}

//////////////////////////////////////////////////////////////////////////////
//
// cream refused event
//
//////////////////////////////////////////////////////////////////////////////
cream_refused_event::cream_refused_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Cream Refused Event" ),
    m_reason( reason )
{

}

int cream_refused_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogRefused( *(ctx->el_context), 
                               EDG_WLL_SOURCE_JOB_SUBMISSION,
                               ctx->el_s_unavailable,
                               ctx->el_s_unavailable,
                               m_reason.c_str() );
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
    m_reason( reason )
{

}

int cream_cancel_refuse_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogCancelREFUSE( *(ctx->el_context), 
                                    m_reason.c_str()
                                    );
}


//////////////////////////////////////////////////////////////////////////////
//
// job running event
//
//////////////////////////////////////////////////////////////////////////////
job_running_event::job_running_event( const CreamJob& j, const std::string& host ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Runnign Event" ),
    m_host( host )
{

}

int job_running_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogRunning( *(ctx->el_context), m_host.c_str() );
}

//////////////////////////////////////////////////////////////////////////////
//
// job really running event
//
//////////////////////////////////////////////////////////////////////////////
job_really_running_event::job_really_running_event( const CreamJob& j, const std::string& wn_seq ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Really Running Event" ),
    m_wn_seq( wn_seq )
{

}

int job_really_running_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogReallyRunning( *(ctx->el_context), m_wn_seq.c_str() );
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
    m_qname( qname )
{

}

int ns_enqueued_start_event::execute( iceLBContext* ctx ) 
{
    return edg_wll_LogEnQueuedSTART( *(ctx->el_context), 
                                     m_qname.c_str(),
                                     m_job.getJobID().c_str(),
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
    m_qname( qname )
{

}

int ns_enqueued_fail_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogEnQueuedFAIL( *(ctx->el_context), 
                                    m_qname.c_str(),
                                    m_job.getJobID().c_str(),
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
    m_qname( qname )
{

}

int ns_enqueued_ok_event::execute( iceLBContext* ctx )
{
    return edg_wll_LogEnQueuedOK( *(ctx->el_context), 
                                  m_qname.c_str(),
                                  m_job.getJobID().c_str(),
                                  ctx->el_s_unavailable
                                  );
}
