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
#include "boost/format.hpp"

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
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Cream Transfer Start Event" )
{

}

int cream_transfer_start_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogTransferSTARTProxy( *(ctx->el_context), 
                                          EDG_WLL_SOURCE_LRMS, 
                                          m_job.getCreamURL().c_str(),
                                          ctx->el_s_unavailable,
                                          m_job.getJDL().c_str(),  
                                          ctx->el_s_unavailable,
                                          ctx->el_s_unavailable );
#else
    return edg_wll_LogTransferSTART( *(ctx->el_context), 
                                     EDG_WLL_SOURCE_LRMS, 
                                     m_job.getCreamURL().c_str(),
                                     ctx->el_s_unavailable,
                                     m_job.getJDL().c_str(),  
                                     ctx->el_s_unavailable,
                                     ctx->el_s_unavailable );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// Cream Transfer OK event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_ok_event::cream_transfer_ok_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Cream Transfer OK Event" )
{

}

int cream_transfer_ok_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogTransferOKProxy( *(ctx->el_context), 
                                       EDG_WLL_SOURCE_LRMS, 
                                       m_job.getCreamURL().c_str(),
                                       ctx->el_s_unavailable,
                                       m_job.getJDL().c_str(),  
                                       ctx->el_s_unavailable,
                                       m_job.getCreamJobID().c_str() );    
#else
    return edg_wll_LogTransferOK( *(ctx->el_context), 
                                  EDG_WLL_SOURCE_LRMS, 
                                  m_job.getCreamURL().c_str(),
                                  ctx->el_s_unavailable,
                                  m_job.getJDL().c_str(),  
                                  ctx->el_s_unavailable,
                                  m_job.getCreamJobID().c_str() );    
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// transfer FAIL event
//
//////////////////////////////////////////////////////////////////////////////
cream_transfer_fail_event::cream_transfer_fail_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format( "Cream Transfer Fail Event, reason=[%1%]") % reason ) ),
    m_reason( reason )
{

}

int cream_transfer_fail_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogTransferFAILProxy( *(ctx->el_context), 
                                         EDG_WLL_SOURCE_LRMS, 
                                         m_job.getCreamURL().c_str(),
                                         ctx->el_s_unavailable,
                                         m_job.getJDL().c_str(),  
                                         m_reason.c_str(), 
                                         ctx->el_s_unavailable );
#else
    return edg_wll_LogTransferFAIL( *(ctx->el_context), 
                                    EDG_WLL_SOURCE_LRMS, 
                                    m_job.getCreamURL().c_str(),
                                    ctx->el_s_unavailable,
                                    m_job.getJDL().c_str(),  
                                    m_reason.c_str(), 
                                    ctx->el_s_unavailable );
#endif
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
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogAcceptedProxy( *(ctx->el_context), 
                                     EDG_WLL_SOURCE_JOB_SUBMISSION, 
                                     ctx->el_s_localhost_name.c_str(),
                                     ctx->el_s_unavailable,
                                     m_job.getCreamJobID().c_str()
                                     );
#else
    return edg_wll_LogAccepted( *(ctx->el_context), 
                                EDG_WLL_SOURCE_JOB_SUBMISSION, 
                                ctx->el_s_localhost_name.c_str(),
                                ctx->el_s_unavailable,
                                m_job.getCreamJobID().c_str()
                                );
#endif
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
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogAcceptedProxy( *(ctx->el_context), 
                                     EDG_WLL_SOURCE_LRMS,
                                     m_job.getCEID().c_str(),
                                     ctx->el_s_unavailable,
                                     m_job.getCreamJobID().c_str()
                                     );
#else
    return edg_wll_LogAccepted( *(ctx->el_context), 
                                EDG_WLL_SOURCE_LRMS,
                                m_job.getCEID().c_str(),
                                ctx->el_s_unavailable,
                                m_job.getCreamJobID().c_str()
                                );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// cream refused event
//
//////////////////////////////////////////////////////////////////////////////
cream_refused_event::cream_refused_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format( "Cream Refused Event, reason=[%1%]") % reason ) ),
    m_reason( reason )
{

}

int cream_refused_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogRefusedProxy( *(ctx->el_context), 
                                    EDG_WLL_SOURCE_JOB_SUBMISSION,
                                    ctx->el_s_unavailable,
                                    ctx->el_s_unavailable,
                                    m_reason.c_str() );
#else
    return edg_wll_LogRefused( *(ctx->el_context), 
                               EDG_WLL_SOURCE_JOB_SUBMISSION,
                               ctx->el_s_unavailable,
                               ctx->el_s_unavailable,
                               m_reason.c_str() );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// cream cancel request event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_request_event::cream_cancel_request_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format( "Cream Cancel Request Event, reason=[%1%]" ) % reason ) ),
    m_reason( reason )
{

}

int cream_cancel_request_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogCancelREQProxy( *(ctx->el_context), 
                                      m_reason.c_str()
                                      );
#else
    return edg_wll_LogCancelREQ( *(ctx->el_context), 
                                 m_reason.c_str()
                                 );
#endif
}



//////////////////////////////////////////////////////////////////////////////
//
// cancel refuse event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_refuse_event::cream_cancel_refuse_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format("Cream Cancel Refuse Event, reason=[%1%]" ) % reason ) ),
    m_reason( reason )
{

}

int cream_cancel_refuse_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogCancelREFUSEProxy( *(ctx->el_context), 
                                         m_reason.c_str()
                                         );
#else
    return edg_wll_LogCancelREFUSE( *(ctx->el_context), 
                                    m_reason.c_str()
                                    );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// cancel done event
//
//////////////////////////////////////////////////////////////////////////////
cream_cancel_done_event::cream_cancel_done_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format( "Job Cancel OK Event, reason=[%1%]" ) % reason ) ),
    m_reason( reason )
{

}

int cream_cancel_done_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogCancelDONEProxy( *(ctx->el_context), 
                                       m_reason.c_str()
                                       );
#else
    return edg_wll_LogCancelDONE( *(ctx->el_context), 
                                  m_reason.c_str()
                                  );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// job running event
//
//////////////////////////////////////////////////////////////////////////////
job_running_event::job_running_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format("Job running event, worker_node=[%1%]" ) % j.get_worker_node() ) )
{

}

int job_running_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogRunningProxy( *(ctx->el_context), m_job.get_worker_node().c_str() );
#else
    return edg_wll_LogRunning( *(ctx->el_context), m_job.get_worker_node().c_str() );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// job really running event
//
//////////////////////////////////////////////////////////////////////////////
job_really_running_event::job_really_running_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, "Job Really Running Event" )
{

}

int job_really_running_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogReallyRunningProxy( *(ctx->el_context), m_job.get_wn_sequence_code().c_str() );
#else
    return edg_wll_LogReallyRunning( *(ctx->el_context), m_job.get_wn_sequence_code().c_str() );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// job cancelled event
//
//////////////////////////////////////////////////////////////////////////////
job_cancelled_event::job_cancelled_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format( "Job Cancelled Event, reason=[%1%]" ) % j.get_failure_reason() ) )
{

}

int job_cancelled_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY

    edg_wll_LogCancelDONEProxy( *(ctx->el_context), 
                                m_job.get_failure_reason().c_str() );

    return edg_wll_LogDoneCANCELLEDProxy( *(ctx->el_context), 
                                          m_job.get_failure_reason().c_str(),
                                          m_job.get_exit_code() );
#else

    edg_wll_LogCancelDONE( *(ctx->el_context), 
                           m_job.get_failure_reason().c_str() );

    return edg_wll_LogDoneCANCELLED( *(ctx->el_context), 
                                     m_job.get_failure_reason().c_str(),
                                     m_job.get_exit_code() );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// job cancelled event
//
//////////////////////////////////////////////////////////////////////////////
job_aborted_event::job_aborted_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format( "Job Aborted Event, reason=[%1%]" ) % j.get_failure_reason() ) )
{

}

int job_aborted_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogAbortProxy( *(ctx->el_context), 
                                  m_job.get_failure_reason().c_str() );
#else
    return edg_wll_LogAbort( *(ctx->el_context), 
                             m_job.get_failure_reason().c_str() );
#endif
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
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogSuspendProxy( *(ctx->el_context), ctx->el_s_unavailable );
#else
    return edg_wll_LogSuspend( *(ctx->el_context), ctx->el_s_unavailable );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// job done ok event
//
//////////////////////////////////////////////////////////////////////////////
job_done_ok_event::job_done_ok_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format("Job Done Ok Event, ExitCode=[%1%]" ) % j.get_exit_code() ) )
{

}

int job_done_ok_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogDoneOKProxy( *(ctx->el_context), 
                                   ctx->el_s_unavailable, 
                                   m_job.get_exit_code() );
#else
    return edg_wll_LogDoneOK( *(ctx->el_context), 
                              ctx->el_s_unavailable, 
                              m_job.get_exit_code() );
#endif
}

//////////////////////////////////////////////////////////////////////////////
// 
// job done failed event
//
//////////////////////////////////////////////////////////////////////////////
job_done_failed_event::job_done_failed_event( const CreamJob& j ) :
    iceLBEvent( j, EDG_WLL_SOURCE_LOG_MONITOR, boost::str( boost::format("Job Done Failed Event, ExitCode=[%1%], FailureReason=[%2%]") % j.get_exit_code() % j.get_failure_reason() ) )
{

}

int job_done_failed_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogDoneFAILEDProxy( *(ctx->el_context), 
                                       m_job.get_failure_reason().c_str(), 
                                       m_job.get_exit_code() );
#else
    return edg_wll_LogDoneFAILED( *(ctx->el_context), 
                                  m_job.get_failure_reason().c_str(), 
                                  m_job.get_exit_code() );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// ns enqueued start event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_start_event::ns_enqueued_start_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format( "NS Enqueued Start Event, qname=[%1%]" ) % qname ) ),
    m_qname( qname )
{

}

int ns_enqueued_start_event::execute( iceLBContext* ctx ) 
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogEnQueuedSTARTProxy( *(ctx->el_context), 
                                          m_qname.c_str(),
                                          m_job.getCreamJobID().c_str(),
                                          ctx->el_s_unavailable
                                          );
#else
    return edg_wll_LogEnQueuedSTART( *(ctx->el_context), 
                                     m_qname.c_str(),
                                     m_job.getCreamJobID().c_str(),
                                     ctx->el_s_unavailable
                                     );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// ns enqueued fail event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_fail_event::ns_enqueued_fail_event( const CreamJob& j, const std::string& qname, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format("NS Enqueued Fail Event, queue=[%1%], reason=[%2%]") % qname % reason ) ),
    m_qname( qname ),
    m_reason( reason )
{

}

int ns_enqueued_fail_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogEnQueuedFAILProxy( *(ctx->el_context), 
                                         m_qname.c_str(),
                                         m_job.getCreamJobID().c_str(),
                                         m_reason.c_str()
                                         );
#else
    return edg_wll_LogEnQueuedFAIL( *(ctx->el_context), 
                                    m_qname.c_str(),
                                    m_job.getCreamJobID().c_str(),
                                    m_reason.c_str()
                                    );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// wms enqueued ok event
//
//////////////////////////////////////////////////////////////////////////////
ns_enqueued_ok_event::ns_enqueued_ok_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format( "NS Enqueued OK Event, qname=[%1%]" ) % qname ) ),
    m_qname( qname )
{

}

int ns_enqueued_ok_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogEnQueuedOKProxy( *(ctx->el_context), 
                                       m_qname.c_str(),
                                       m_job.getCreamJobID().c_str(),
                                       ctx->el_s_unavailable
                                       );
#else
    return edg_wll_LogEnQueuedOK( *(ctx->el_context), 
                                  m_qname.c_str(),
                                  m_job.getCreamJobID().c_str(),
                                  ctx->el_s_unavailable
                                  );
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// ice resubmission event
//
//////////////////////////////////////////////////////////////////////////////
ice_resubmission_event::ice_resubmission_event( const CreamJob& j, const std::string& reason ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format("ICE Resubmission Event, reason=[%1%]") % reason ) ),
    m_reason( reason )
{

}

int ice_resubmission_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogResubmissionWILLRESUBProxy( *(ctx->el_context), 
                                                  m_reason.c_str(),
                                                  ctx->el_s_unavailable
                                                  );
#else
    return edg_wll_LogResubmissionWILLRESUB( *(ctx->el_context), 
                                             m_reason.c_str(),
                                             ctx->el_s_unavailable
                                             );
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// wms dequeued event
//
//////////////////////////////////////////////////////////////////////////////
wms_dequeued_event::wms_dequeued_event( const CreamJob& j, const std::string& qname ) :
    iceLBEvent( j, EDG_WLL_SOURCE_JOB_SUBMISSION, boost::str( boost::format( "WMS Dequeue Event, qname=[%1%]") % qname ) ),
    m_qname( qname )
{

}

int wms_dequeued_event::execute( iceLBContext* ctx )
{
#ifdef GLITE_WMS_HAVE_LBPROXY
    return edg_wll_LogDeQueuedProxy( *(ctx->el_context), 
                                     m_qname.c_str(),
                                     m_job.getGridJobID().c_str()
                                     );
#else
    return edg_wll_LogDeQueued( *(ctx->el_context), 
                                m_qname.c_str(),
                                m_job.getGridJobID().c_str()
                                );
#endif
}

