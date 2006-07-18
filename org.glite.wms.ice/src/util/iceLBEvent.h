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
 * ICE Event 
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef ICELBEVENT_H
#define ICELBEVENT_H

#include "creamJob.h"
#include "glite/lb/producer.h"
#include <string>

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                // Forward declaration
                class iceLBContext;

                /**
                 * This class represents the generic ICE LB event. All
                 * events must implement the execute() method, with
                 * the single ctx parameter being the current LB
                 * context. 
                 */
                class iceLBEvent {
                public:
                    virtual ~iceLBEvent( ) { };

                    /**
                     * Actually logs the event. 
                     *
                     * @param ctx a pointer to the current L&B
                     * context.  the caller is not transfering
                     * ownership of the ctx pointer; this method MUST
                     * NOT try to free the context.
                     *
                     * @return the LB exit code (0 means success)
                     */
                    virtual int execute( iceLBContext* ctx ) = 0;

                    /**
                     * Provides a description for the event being
                     * logged.  useful for debug purposes.
                     *
                     * @return the description of the event being
                     * logged
                     */
                    const std::string& describe( ) const {
                        return m_description;
                    };

                    /**
                     * Gets the job being logged
                     *
                     * @return the current job
                     */
                    CreamJob& getJob( ) {
                        return m_job; 
                    };

                    /**
                     * Gets the source logging the event
                     *
                     * @return the source logging the event
                     */
                    edg_wll_Source getSrc( void ) const {
                        return m_src;
                    }

                protected:
                    /**
                     * Constructor for an iceLBEvent. 
                     *
                     * @param j the job to be logged
                     * @param src the source logging the event
                     * @param dsc a textual description of the event being logged
                     */
                    iceLBEvent( const CreamJob& j, edg_wll_Source src, const std::string& dsc );

                    CreamJob m_job;
                    edg_wll_Source m_src;
                    std::string m_description;
                };

                /**
                 * A job is being transferred from ICE to CREAM.
                 */
                class cream_transfer_start_event : public iceLBEvent {
                public:
                    /**
                     * @param j the job being transferred.
                     */
                    cream_transfer_start_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };

                /**
                 * A job has been succesfully transferred from ICE to CREAM.
                 */
                class cream_transfer_ok_event : public iceLBEvent {
                public:
                    /**
                     * @param j the job succesfully transferred
                     */
                    cream_transfer_ok_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                /**
                 * Transfer of a job from ICE to CREAM failed due to some error
                 */
                class cream_transfer_fail_event : public iceLBEvent {
                public:
                    /**
                     * @param j the job whose transfer failed
                     * @param reason a string describing the failure reason 
                     */
                    cream_transfer_fail_event( const CreamJob& j, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_reason;
                };

                /**
                 * A request has been dequeued from the WMS queue
                 */ 
                class wms_dequeued_event : public iceLBEvent {
                public:
                    wms_dequeued_event( const CreamJob& j, const std::string& qname );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_qname;
                };


                /**
                 * A job has been accepted by CREAM. This event is
                 * logged when ICE receives a PENDING status from CREAM.
                 */
                class cream_accepted_event : public iceLBEvent {
                public:
                    cream_accepted_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };

                /**
                 * A job has been accepted by the LRMS. This event
                 * is logged when ICe receives an IDLE status from CREAM.
                 */
                class lrms_accepted_event : public iceLBEvent {
                public:
                    lrms_accepted_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class cream_refused_event : public iceLBEvent {
                public:
                    cream_refused_event( const CreamJob& j, const std::string& reason  );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_reason;
                };


                class cream_cancel_request_event : public iceLBEvent {
                public:
                    cream_cancel_request_event( const CreamJob& j, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_reason;
                };


                class cream_cancel_refuse_event : public iceLBEvent {
                public:
                    cream_cancel_refuse_event( const CreamJob& j, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_reason;
                };


                class cream_cancel_done_event : public iceLBEvent {
                public:
                    cream_cancel_done_event( const CreamJob& j, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_reason;
                };


                class job_aborted_event : public iceLBEvent {
                public:
                    job_aborted_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };



                class job_running_event : public iceLBEvent {
                public:
                    job_running_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class job_really_running_event : public iceLBEvent {
                public:
                    job_really_running_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class job_cancelled_event : public iceLBEvent {
                public:
                    job_cancelled_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class job_suspended_event : public iceLBEvent {
                public:
                    job_suspended_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class job_done_ok_event : public iceLBEvent {
                public:
                    job_done_ok_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class job_done_failed_event : public iceLBEvent {
                public:
                    job_done_failed_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class ns_enqueued_start_event : public iceLBEvent {
                public:
                    ns_enqueued_start_event( const CreamJob& j, const std::string& qname );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_qname;
                };


                class ns_enqueued_fail_event : public iceLBEvent {
                public:
                    ns_enqueued_fail_event( const CreamJob& j, const std::string& qname, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_qname;
                    std::string m_reason;
                };


                class ns_enqueued_ok_event : public iceLBEvent {
                public:
                    ns_enqueued_ok_event( const CreamJob& j, const std::string& qname );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_qname;
                };


                class ice_resubmission_event : public iceLBEvent {
                public:
                    ice_resubmission_event( const CreamJob& j, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string m_reason;
                };



            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
