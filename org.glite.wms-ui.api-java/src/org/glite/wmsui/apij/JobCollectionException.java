/*
* JobCollectionException.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
/**
 * Manage the exception raised while using a collection of jobs such as adding two jobs with the same jobId,
 * performing an action over an empty collection...
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class JobCollectionException extends JobException {
	/**Default Constructor
	* @param msg the string representation of the Error Message*/
	public JobCollectionException (String msg) {
		super(msg);
	}
	/**Constructor with Throwable object
	* @param msg the string representation of the Error Message
	* @param ex the Throwable object that raised the exception*/
	public JobCollectionException(String msg, Throwable ex) {
		super(msg, ex);
	}
}
