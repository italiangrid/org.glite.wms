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
 * ICE Event Status change listener
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE stuff
#include "eventStatusListener.h"
#include "subscriptionManager.h"
#include "iceConfManager.h"
#include "iceUtils.h"
#include "ice-core.h"
#include "iceCommandUpdateStatus.h"
#include "iceThreadPool.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other GLITE stuff
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// System includes
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <sys/select.h>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

extern int errno;

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

#define ACCEPT_TIMEOUT 30

void eventStatusListener::createObject( void )
{
    try {
        m_myname = getHostName( );
    } catch( runtime_error& ex ) {
        m_isOK = false;
        return;
    }
    

    CREAM_SAFE_LOG(m_log_dev->info( "eventStatusListener::CTOR() - Listener created!" ));
    
}

//______________________________________________________________________________
eventStatusListener::eventStatusListener(const int& i)
    : CEConsumer(i),
      iceThread( "event status listener" ),
      m_conf(iceConfManager::getInstance()->getConfiguration()->ice()),
      m_log_dev( api::util::creamApiLogger::instance()->getLogger() ),
      m_isOK( true ),
      m_ice_manager( glite::wms::ice::Ice::instance() )
{
    createObject();
}

//______________________________________________________________________________
eventStatusListener::eventStatusListener(const int& i,
						  const string& cert,
						  const string& key)
  : CEConsumer(i, cert.c_str(), key.c_str()),
    iceThread( "event status listener" ),
    m_conf(iceConfManager::getInstance()->getConfiguration()->ice()),
    m_log_dev( api::util::creamApiLogger::instance()->getLogger() ),
    m_isOK( true ),
    m_ice_manager( glite::wms::ice::Ice::instance() )
{
    createObject();
}

//______________________________________________________________________________
void eventStatusListener::body( void )
{
    while( !isStopped() ) {
        if(!getenv("NO_LISTENER_MESS"))
            CREAM_SAFE_LOG(m_log_dev->infoStream() 
                           << "eventStatusListener::body() - "
                           << "Waiting for job status notification"
                           << log4cpp::CategoryStream::ENDLINE);
        acceptJobStatus();
        sleep(1);
    }
}

//______________________________________________________________________________
void eventStatusListener::acceptJobStatus(void)
{
  if( isStopped() ) {
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "eventStatusListener is ending."
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }
  /**
   * Waits for an incoming connection
   */
  int localSocket = this->getLocalSocket();
  fd_set fds;
  struct timeval timeout;
  //sigset_t sig;
  int ret;
  FD_ZERO(&fds);
  FD_SET(localSocket, &fds);
  timeout.tv_sec = ACCEPT_TIMEOUT;
  timeout.tv_usec = 0;
  ret = select(localSocket+1, &fds, NULL, NULL, &timeout);

  if (ret == -1) {
    CREAM_SAFE_LOG(m_log_dev->errorStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "select() error: "
		   << strerror(errno)
		   << log4cpp::CategoryStream::ENDLINE);
    //perror("select()");
    return;
  }
  else if (ret) { /* will do actualy the this->accept() call */ 
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "Listening socket received something to read: ready to accept a SOAP connection"
		   << log4cpp::CategoryStream::ENDLINE);
  }
    //printf("Data is available now.\n");
    /* FD_ISSET(0, &rfds) will be true. */
  else {
    //printf("No data within five seconds.\n");
    CREAM_SAFE_LOG(m_log_dev->infoStream()
		   << "eventStatusListener::acceptJobStatus() - "
		   << "No connection within " << ACCEPT_TIMEOUT << " seconds. Listening again."
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }

    if( !this->accept() ) {
      //if(!getenv("NO_LISTENER_MESS"))
	CREAM_SAFE_LOG(m_log_dev->errorStream()
		       << "eventStatusListener::acceptJobStatus()"
		       << " - CEConsumer::Accept() returned false."
		       << log4cpp::CategoryStream::ENDLINE);
      return;
    }

    if(!getenv("NO_LISTENER_MESS"))
      CREAM_SAFE_LOG(m_log_dev->infoStream()
		     << "eventStatusListener::acceptJobStatus() - "
		     << "Connection accepted from ["
		     << this->getClientName() << "] (" << this->getClientIP() << ")"
		     << log4cpp::CategoryStream::ENDLINE);

  /**
   * acquires the event from the client
   * and deserializes the data structures
   */
  if( !this->serve() ) {
    CREAM_SAFE_LOG(m_log_dev->errorStream() 
		   << "eventStatusListener::acceptJobStatus() - "
		   << "ErrorCode=["
		   << this->getErrorCode() << "]"
		   << " ErrorMessage=["
		   << this->getErrorMessage() << "]"
		   << log4cpp::CategoryStream::ENDLINE);
    return;
  }

  { // start TIMING notification handling

    //    api::util::scoped_timer T("*** NOTIF HANDLING ***");

    if( m_conf->listener_enable_authz() && m_conf->listener_enable_authn() ) {
      string dn = this->getClientDN();
      if(!getenv("NO_LISTENER_MESS"))
	CREAM_SAFE_LOG(m_log_dev->infoStream()
		       << "eventStatusListener::acceptJobStatus() - "
		       << "Got DN ["
		       << dn << "] for host ["
		       << this->getClientName() << "]"
		       << log4cpp::CategoryStream::ENDLINE);
      
      if( !subscriptionManager::getInstance()->isAuthorized( dn ) ) {
	if(!getenv("NO_LISTENER_MESS"))
	  CREAM_SAFE_LOG(m_log_dev->warnStream() 
			 << "eventStatusListener::acceptJobStatus() - "
			 << "Remote notifying client DN has not been found is the"
			 << " subscriptionManager. Cannot authorize this notification. Ignoring it..." 
			 << log4cpp::CategoryStream::ENDLINE);
	return;
      }
    }
    
    /**
     * Checks if the SOAP message was correct (i.e.
     * containing a non-null topic)
     */
    if(!this->getEventTopic()) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() 
		     << "eventStatusListener::acceptJobStatus() - "
		     << "NULL Topic received. Ignoring this notification...." 
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }
    
    /**
     * Ignores notifications generated by non-ICE topics
     */
    if(this->getEventTopic()->Name != m_conf->ice_topic()) {
      CREAM_SAFE_LOG(m_log_dev->warnStream() 
		     << "eventStatusListener::acceptJobStatus() - "
		     << "Received a notification with TopicName="
		     << this->getEventTopic()->Name
		     << " that differs from the ICE's topic "
		     << "official name. Ignoring this notification."
		     << log4cpp::CategoryStream::ENDLINE);
      return;
    }
    
    iceThreadPool* threadPool( m_ice_manager->get_ice_commands_pool() );

    const vector< monitortypes__Event >& events( getEvents() );    
    iceAbsCommand* cmd = new iceCommandUpdateStatus( events, getClientDN() );
    threadPool->add_request( cmd ); // ownership of the cmd pointer is passed to the threadPool object

  } // stop TIMING notification handling
}

