/*
* Listener.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
/* DEPRECATED STUFF...
import  java.io.BufferedReader ; //Stream of info gathering from pipes
import  java.io.BufferedWriter; //Stream of info gathering from pipes
*/


/**
* The listener interface is used in order to manage the interaction of interactive jobs.
* <p>
* Depending on how it is implemented, the task of the implemented class should be:
* <ul>
* <li> Reading the output of the job inside the output named pipe
* <li> Prompt the output message to the user (by using a swing window or simply to the std output)
* <li> Catch the standard input from the user and write it to the input name pipe
* </ul>
* A simple command line implementation is given by the ListenerConsole class
* @see ListenerConsole a command-line implementation of Listener interface
*/
public interface Listener {
	/**
	This Method is called once the shadow has been successfully launched
	and the Job is ready to perform the interactive console
	@param shadow the Shadow instance that stores all the information needed to perform a console interactivity
	*/
	public void run ( Shadow shadow) ;
}
