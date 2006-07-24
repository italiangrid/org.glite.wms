//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.h
//
#ifndef __ICELBCONTEXT_H__
#define __ICELBCONTEXT_H__

#include <exception>

#include "glite/lb/context.h"
#include "creamJob.h"

// Forward declaration
namespace log4cpp {
    class Category;
};

typedef  struct _edg_wll_Context  *edg_wll_Context;

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                class iceLBException : public std::exception {
                public:
                    iceLBException( const char *reason );
                    iceLBException( const std::string &reason );
                    virtual ~iceLBException( void ) throw();
                    
                    virtual const char *what( void ) const throw();
                    
                private:
                    std::string     m_le_reason;
                };

                // Forward Declaration
                class jobCache;

                class iceLBContext {
                public:

                    iceLBContext( );

                    virtual ~iceLBContext( void );

                    static void set_lb_retries( unsigned int r ) { s_el_s_retries = r; return; }
                    static void set_lb_interval( unsigned int sec ) { s_el_s_sleep = sec; return; }
                    static void set_lb_destination( const char* dest ) { el_s_destination = dest; return; }

                    /**
                     * @obsolete{Registers a new job (this would
                     * normally be done by the UI).}
                     */
                    void registerJob( const CreamJob& theJob );

                    inline void startLogging( void ) { this->m_el_count = 0; this->m_el_hostProxy = false; }

                    void setLoggingJob( const CreamJob& theJob, edg_wll_Source src  ) throw( iceLBException& );

                    void testCode( int &code, bool retry = false );
                    std::string getLoggingError( const char *preamble );

                    /**
                     * Modifies the job passed as parameter by
                     * changing its sequence code. The new sequence
                     * code is the on in the currently el_context data
                     * structure. ONLY IF IT IS ALREADY IN THE CACHE,
                     * the modified job is stored in the job cache. If
                     * the job is currently NOT in the job cache, it
                     * is *not put there.
                     *
                     * @param theJob the job to be modified and stored
                     */
                    void update_and_store_job( CreamJob& theJob );

                    edg_wll_Context* el_context;

                    static const char *el_s_notLogged, *el_s_unavailable, *el_s_OK, *el_s_failed;
                    static const char *el_s_destination; 

                    std::string el_s_localhost_name;

                protected:
                    bool                m_el_hostProxy;
                    unsigned int        m_el_count;
                    log4cpp::Category  *m_log_dev;
                    jobCache           *m_cache;

                    static unsigned int s_el_s_retries, 
		                        s_el_s_sleep;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
