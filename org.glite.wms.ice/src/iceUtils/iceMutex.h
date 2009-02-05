
#ifndef GLITE_WMS_ICE_UTIL_ICEMUTEX_H
#define GLITE_WMS_ICE_UTIL_ICEMUTEX_H

#include <pthread.h>
#include <string>

#include "glite/ce/cream-client-api-c/scoped_timer.h"
//#include "boost/thread/recursive_mutex.hpp"

namespace api_util = glite::ce::cream_client_api::util;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class iceLock {
	  friend class iceMutex;

	protected:
	  pthread_mutex_t     m_t;
	  pthread_mutexattr_t m_attr;
	public:
	  iceLock() {
	    pthread_mutexattr_init(&m_attr);
	    pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE );
	    pthread_mutex_init( &m_t, &m_attr );
	  }

	  ~iceLock() {
	    pthread_mutexattr_destroy(&m_attr);
	  }

	};

	class iceMutex {
	  
	  pthread_mutex_t     *m_t;
	  
	  std::string m_callerName;
	  api_util::scoped_timer *m_Timer_acquisition;
	  api_util::scoped_timer *m_Timer_execution;

	public:
	  iceMutex(const std::string& callerName, iceLock& lock) :  m_t( &(lock.m_t)), 
	    m_callerName( callerName ), 
	    m_Timer_acquisition(0), 
	    m_Timer_execution(0)
	      {
		
#ifdef ICE_PROFILE_ENABLE
		if( !m_callerName.empty() ) {
		  m_Timer_acquisition = new api_util::scoped_timer( std::string("ICE_MUTEX_ACQ Caller=") + m_callerName );
		}
#endif
		pthread_mutex_lock( m_t );
		
#ifdef ICE_PROFILE_ENABLE
		
		if( !m_callerName.empty() ) {
		  delete m_Timer_acquisition;
		  m_Timer_execution = new api_util::scoped_timer( std::string("ICE_MUTEX_EXE Caller=") + m_callerName );
		}
#endif
	      }
	    
	    ~iceMutex() throw( ) {
#ifdef ICE_PROFILE_ENABLE
	      if( !m_callerName.empty() )
		delete m_Timer_execution;
#endif
	      pthread_mutex_unlock( m_t );
	    }
	    
	};
	
      }
    }
  }
}

#endif
