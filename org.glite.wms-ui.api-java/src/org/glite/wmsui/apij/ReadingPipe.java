/*
 * ListenerFrame.java
 *
 * Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */
package org.glite.wmsui.apij;
import  java.io.BufferedReader ; //Stream of info gathering from pipes
import  java.io.FileReader;
import  java.io.FileWriter;
import  java.io.BufferedWriter; //Stream of info gathering from pipes
/***
* This class is not visible to the user, it reads a pipe from a shadow
*/
class ReadingPipe implements Runnable  {
	/** 
	* The default constructor.
	* @param shadow instance is able to retrieve al l need3ed information from the interactive job
	* @see Shadow
	*/
	public  ReadingPipe (   Shadow shadow )  {  sh= shadow ;  };
	/**
	* Start the actual reading pipe to run and empty the pipe
	*/
	public void run ()   {
		while ( true )  try{
			String str = sh.emptyOut()  ;
			if ( !str.equals("")   )  System.out.print(str);  else Thread.sleep ( 1000  ) ;
		} catch ( Exception exc) { /* Do nothing*/}
	}
/** Private Members*/
    private Shadow sh = null;
}  ;
