#ifndef  GLITE_WMS_UI_CLIENT_SHADOW_H
#define GLITE_WMS_UI_CLIENT_SHADOW_H
/*
 * Shadow.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
*/


#include "Listener.h"
#include "glite/wmsutils/jobid/JobId.h"

namespace glite {
namespace wmsui {
namespace api {

/**
 * This class provides the core management for interactive jobs.
 * once the glite-grid-console-shadow has started successfully and the job is running
 * the user should interact with the submitted job  (or might have attached to a previous job)
* At the end of the interaction the background bypass process should be
 * killed and the I/O pipes have to be removed. This is done automatically by the 'detach' method.
 * The shadow class must be used togheter with an implementation of the Listener interface, which actually performs the
 * final visual interactivity with the user.
 * @see Listener
 */

class Shadow {
  public:
	/** Attach a new listener to the Job*/
	void attach( int port = 0 ) ;
	/** Read the specified buffer and return it's content */
	std::string empty (std::string buffer) ;
	/** Stop the launched processes and remove the created listener pipes*/
	void detach();
	/** Start the Listener run method
	* @see Listener
	*/
	void start();
	/** @return the error pipe string representation*/
	std::string getPipeErr();
	/** @return the Input pipe string representation*/
	std::string getPipeIn();
	/** @return the Output pipe string representation*/
	std::string getPipeOut();
	/** Get the port where the shadow is listening to*/
	int getPort();
	/** Get the process id of the launched listener process*/
	int getPid();	
	/** @return the local host name */
	static std::string getHost() ;

  private:
	friend class Job ;
	/**Constructor*/
	Shadow (glite::wmsutils::jobid::JobId  jid ,  Listener* ls) ;
	/**Ctor */
	Shadow();
	/**CDtor */
	~Shadow();
	/**Set the JobId where to listen to */
	void set(  glite::wmsutils::jobid::JobId  jid , Listener* ls=NULL) ;
	/** Launch the listener process */
	void console( int port = 0);
	/** Private */
	void kill();
	int pid;
	int port ;
	std::string host;
	Listener *listener ;
	std::string pipeRoot;
	glite::wmsutils::jobid::JobId jobId;
};
//end Listener class

} // api
} // wmsui
} // glite

#endif
