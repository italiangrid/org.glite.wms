//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.h
//
#ifndef __ICEEVENTLOGGER_H__
#define __ICEEVENTLOGGER_H__

#include <exception>

#include "glite/lb/context.h"

// Forward declaration
namespace log4cpp {
    class Category;
};

typedef  struct _edg_wll_Context  *edg_wll_Context;

namespace classad { class ClassAd; }

namespace glite {
    namespace wms {
        namespace ice {
            namespace util {

                struct ProxySet {
                    const char  *ps_x509Proxy, *ps_x509Key, *ps_x509Cert;
                };

                class iceLoggerException : public std::exception {
                public:
                    iceLoggerException( const char *reason );
                    iceLoggerException( const std::string &reason );
                    virtual ~iceLoggerException( void ) throw();
                    
                    virtual const char *what( void ) const throw();
                    
                private:
                    std::string     le_reason;
                };

                class iceEventLogger {
                public:
                    static iceEventLogger* instance( void );

                    ~iceEventLogger( void );
                    iceEventLogger &reset_user_proxy( const std::string &proxyfile );

                    // iceEventLogger &set_ice_context( const std::string &jobid, const std::string &sequence, const std::string &proxyfile);
                    
                    iceEventLogger &initialize_ice_context( ProxySet *ps ) throw( iceLoggerException& );
                    // iceEventLogger &reset_context( const std::string& jobid, const std::string &sequence ) throw( iceLoggerException& );

                    void unhandled_event( const char *descr );
                    void execute_event( const char *jobid, const char *host ) throw( iceLoggerException& );

                    /*
                      Extractors
                    */
                    std::string sequence_code( void );

                    inline operator edg_wll_Context *( void ) { return this->el_context; }

                    inline static void set_lb_retries( unsigned int r ) { el_s_retries = r; return; }
                    inline static void set_lb_interval( unsigned int sec ) { el_s_sleep = sec; return; }
                    inline static void set_lb_destination( const char* dest ) { el_s_destination = dest; return; }

                protected:
                    inline void startLogging( void ) { this->el_count = 0; this->el_hostProxy = false; }
                    void testCode( int &code, bool retry = false );
                    std::string getLoggingError( const char *preamble );

                    /**
                     * Default costructor
                     * 
                     * This method builds a new iceEventLogger object;
                     * moreover, a new edg_wll_Context is created and
                     * associated to this logger. By default, the
                     * logger is set to remove the context when this
                     * object is destroyed.
                     *
                     * @throw iceLoggerException if the logging
                     * context cannot be initialized.
                     */
                    iceEventLogger( void ) throw ( iceLoggerException& );

                    /**
                     * Costructor
                     *
                     * Initializes an iceEventLogger by specifying an
                     * initial context. The logger is set not to
                     * destroy the given logging context upon
                     * termination.
                     */
                    // iceEventLogger( edg_wll_Context *cont, int flag = EDG_WLL_SEQ_NORMAL );

                    /**
                     * Sets the service which is logging to "Job Controller"
                     */
                    void setJCPersonality( void ) throw( iceLoggerException& );

                    /**
                     * Sets the service which is logging to "Log Monitor"
                     */
                    void setLMPersonality( void ) throw( iceLoggerException& );

                    /**
                     * Sets the jobid being logger
                     */ 
                    void setLoggingJob( const char* jobid ) throw( iceLoggerException& );                   

                    bool               el_remove, el_hostProxy;
                    int                el_flag;
                    unsigned int       el_count;
                    edg_wll_Context   *el_context;
                    std::string        el_proxy;
                    log4cpp::Category *log_dev;

                    static unsigned int         el_s_retries, el_s_sleep;
                    static const char          *el_s_notLogged, *el_s_unavailable, *el_s_OK, *el_s_failed;
                    static const char *el_s_destination; 

                    static iceEventLogger* _instance;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
