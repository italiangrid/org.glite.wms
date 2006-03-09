#ifndef __ICELBEVENT_H__
#define __ICELBEVENT_H__

#include "creamJob.h"
#include <string>

// Forward declaration
namespace log4cpp {
    class Category;
};

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
                     */
                    virtual void execute( iceLBContext* ctx ) = 0;
                protected:
                    iceLBEvent( );

                    log4cpp::Category* log_dev;
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
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
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
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
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
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _reason;
                };


                class cream_accepted_event : public iceLBEvent {
                public:
                    cream_accepted_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class lrms_accepted_event : public iceLBEvent {
                public:
                    lrms_accepted_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class cream_refused_event : public iceLBEvent {
                public:
                    cream_refused_event( const CreamJob& j, const std::string& reason  );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _reason;
                };


                class cream_cancel_request_event : public iceLBEvent {
                public:
                    cream_cancel_request_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class cream_cancel_refuse_event : public iceLBEvent {
                public:
                    cream_cancel_refuse_event( const CreamJob& j, const std::string& reason );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _reason;
                };


                class job_running_event : public iceLBEvent {
                public:
                    job_running_event( const CreamJob& j, const std::string& host );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _host;
                };


                class job_cancelled_event : public iceLBEvent {
                public:
                    job_cancelled_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class job_suspended_event : public iceLBEvent {
                public:
                    job_suspended_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class job_done_ok_event : public iceLBEvent {
                public:
                    job_done_ok_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class job_done_failed_event : public iceLBEvent {
                public:
                    job_done_failed_event( const CreamJob& j );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                };


                class ns_enqueued_start_event : public iceLBEvent {
                public:
                    ns_enqueued_start_event( const CreamJob& j, const std::string& qname );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _qname;
                };


                class ns_enqueued_fail_event : public iceLBEvent {
                public:
                    ns_enqueued_fail_event( const CreamJob& j, const std::string& qname );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _qname;
                };


                class ns_enqueued_ok_event : public iceLBEvent {
                public:
                    ns_enqueued_ok_event( const CreamJob& j, const std::string& qname );
                    void execute( iceLBContext* ctx );
                protected:
                    CreamJob _job;
                    std::string _qname;
                };


            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
