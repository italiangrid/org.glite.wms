#ifndef __ICELBEVENT_H__
#define __ICELBEVENT_H__

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
                        return _description;
                    };

                    /**
                     * Gets the job being logged
                     *
                     * @return the current job
                     */
                    CreamJob& getJob( ) {
                        return _job; 
                    };

                    /**
                     *
                     */
                    edg_wll_Source getSrc( void ) const {
                        return _src;
                    }

                protected:
                    iceLBEvent( const CreamJob& j, edg_wll_Source src, const std::string& dsc );

                    CreamJob _job;
                    edg_wll_Source _src;
                    std::string _description;
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
                    std::string _reason;
                };


                class cream_accepted_event : public iceLBEvent {
                public:
                    cream_accepted_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


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
                    std::string _reason;
                };


                class cream_cancel_request_event : public iceLBEvent {
                public:
                    cream_cancel_request_event( const CreamJob& j );
                    int execute( iceLBContext* ctx );
                };


                class cream_cancel_refuse_event : public iceLBEvent {
                public:
                    cream_cancel_refuse_event( const CreamJob& j, const std::string& reason );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string _reason;
                };


                class job_running_event : public iceLBEvent {
                public:
                    job_running_event( const CreamJob& j, const std::string& host );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string _host;
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
                    std::string _qname;
                };


                class ns_enqueued_fail_event : public iceLBEvent {
                public:
                    ns_enqueued_fail_event( const CreamJob& j, const std::string& qname );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string _qname;
                };


                class ns_enqueued_ok_event : public iceLBEvent {
                public:
                    ns_enqueued_ok_event( const CreamJob& j, const std::string& qname );
                    int execute( iceLBContext* ctx );
                protected:
                    std::string _qname;
                };


            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
