/*
 * ListenerConsole.java
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
import org.glite.wmsui.apij.Shadow ; //  UI
import org.glite.wmsui.apij.Listener ; //  UI
import java.io.IOException ; // Stream writing Exception
/**
 * Implementation of the Listener interface.
 * <p>
 * IT provides a simple command-line from the same console where the java exututahble has been launched
 * which is able to perform interactivity between the user and the bypass  background shadow process
 * @see Listener
 * @see Shadow
 *
 * @version 1.0
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it> */
public class ListenerConsole  implements Listener {
	/** Default Constructor */
	public ListenerConsole( ) { }
	/**
	* Set the terminator string. Once the interactivity is started the console needs
	* a string that indicates that the session is finished and the Shadow must be detached
	* @param tr the terminator string to be set
	*/
	public void setTerminator(String tr) {
		terminator = tr ;
	}
	/***
	* This is the core method implemented for Listener interface.
	It launch in the background a series of threads which read/write the pipe streams and performs interactivity
	@param sh this Shadow instance allows the Listener to retrieve any needed information for the interaction
	*/
	public void run ( Shadow sh){
		shadow = sh ;
		thOut = new Thread ( new ReadingPipe (  shadow  )  ) ;
		thOut.setDaemon(  true )  ;
		thOut.start() ;
		String str = new String() ;
		System.out.println ("***************  Type " + terminator +" to end session ***************") ;
		while (true) {
			try {
					char tmpChar = (char)  System.in.read() ;
					if ( tmpChar!=-1)  str = str + String.valueOf( tmpChar) ;
						else   Thread.sleep (1000) ;
					if (str.lastIndexOf("\n") != -1){
						if (   str.equals(terminator +"\n")  )  break ;
						else{ sh.writeIn (str ) ;  str = new String () ;   }
					}
			} catch (Exception exc ) {  /* Do nothing */  }
		} // end while
		stop() ;
	}
	/**
	* Close session
	*/
	private void stop(){
		/** Flush output /  error Streams */
		try{
			thOut.join( 1  ) ;
			shadow.detach();
		}
		catch (NoSuchFieldException exc) { /*  Do nothing */     }
		catch (java.lang.InterruptedException exc) { /*  Do nothing */     }
		System.out.println( "Intearctive Session closed by User.\nBye\n" ) ;
		System.exit(0) ;
	}
	/*  write the input information into the pipe*/
	private void send (String msg) {
		if (   thIn.isAlive()  ) {
			// Check The previous Thread
			System.out.println( "\n*** Error: Unable to send the message '" + msg +"'.\nStill waiting for the previous message to be read by input stream" );
			return ;
		}
		// Launch the Thread which writes into the input pipe
		thIn.setDaemon(  true )  ;
		thIn.start() ;
	}
	// Terminator string
	private String terminator ="$QUIT" ;
	// Shadow instance
	private Shadow shadow;
/* DEPRECATED STUFF
	static int OUT = 1 ;
	static int IN = 2 ;
	static int ERR = 3 ;
	BufferedWriter inputBuffer ;
*/
	Thread thIn=null, thOut ;  // input writing Thread
};  //End ListenerConsole class
