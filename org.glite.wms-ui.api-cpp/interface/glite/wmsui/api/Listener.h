#ifndef  GLITE_WMS_UI_CLIENT_LISTENER_H
#define GLITE_WMS_UI_CLIENT_LISTENER_H
/*
 * Listener.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
*/

namespace glite {
namespace wmsui {
namespace api {

class Shadow ;
/**
* The listener interface is used in order to manage the interaction of interactive jobs.
* <p>
* Depending on how it is implemented, the task of the implemented class should be:
* <ul>
* <li> Reading the output of the job inside the output named pipe
* <li> Prompt the output message to the user (by using a swing window or simply to the std output)
* <li> Catch the standard input from the user and write it to the input name pipe
* </ul>
*/
class Listener {
  public:
	/**
	This Method is called once the shadow has been successfully launched
	and the Job is ready to perform the interactive console
	@param shadow the Shadow pointer that stores all the information needed to perform a console interactivity
	*/
     virtual void run ( Shadow *shadow) const = 0 ;
};
//end Listener class

} // api
} // wmsui
} // glite 

#endif
