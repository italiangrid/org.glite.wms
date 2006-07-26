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
 * ICE LB Logger Context
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

//
// This file is heavily based on org.glite.wms.jobsubmission/src/common/EventLogger.h
//
#ifndef ICELBCONTEXT_H
#define ICELBCONTEXT_H

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

                    edg_wll_Context* el_context;

                    static const char *el_s_notLogged, *el_s_unavailable, *el_s_OK, *el_s_failed;
                    static const char *el_s_destination; 

                    std::string el_s_localhost_name;

                protected:
                    bool                m_el_hostProxy;
                    unsigned int        m_el_count;
                    log4cpp::Category  *m_log_dev;

                    static unsigned int s_el_s_retries, 
		                        s_el_s_sleep;
                };

            } // namespace util
        } // namespace ice
    } // namespace wns
}; // namespace glite

#endif
