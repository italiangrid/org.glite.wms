
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 *
 * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */

package org.glite.wms.wmproxy.jobregister;

import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.BaseException;
import org.glite.wms.wmproxy.JobIdStructType;
import org.glite.jdl.JobAd ;

/*
	Test of  "jobRegister" method in org.glite.wms.wmproxy.WMProxyAPI

*/
public class WMProxyJobRegisterTest {
	/**
	* Default constructor
	*/
	public WMProxyJobRegisterTest ( ) { }
	/**
		 Prints the results
	*/
	private static void printResult (JobIdStructType entry) {
		JobIdStructType children[ ] = null;
		int size = 0;
		if ( entry != null ) {
			// id
			System.out.println ("jobID	= [" + entry.getId ( ) + "]" );
			// name
			System.out.println ("name	= [" +  entry.getName ( ) + "]" );

			// children
			children = (JobIdStructType[ ] ) entry.getChildrenJob ( );
			if ( children != null ) {
				size = children.length ;
				System.out.println ("number of children = [" + size + "]" );
				if ( size  > 0 ) {
					for (int i = 0; i < size ; i++){
						System.out.println ("child n. " + (i+1) );
						System.out.println ("--------------------------------------------");
						printResult (children [i] );
					}
				}
			} else
				System.out.println ("no children" );
		}
	}
	/*
	*	Starts the test
	*	@param url service URL
	*  	@param jdlFile the path location of the JDL file
	*  	@param delegationID the id to identify the delegation
	*	@param propFile the path location of the configuration file
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String delegationId, String jdlFile ) throws  org.glite.wms.wmproxy.BaseException {
		//jdl
		String jdlString = "";
		// output results
		JobIdStructType result = null;
		// reads jdl
		JobAd jad = new JobAd ( );
		try {
			jad.fromFile ( jdlFile );
		} catch (Exception exc) {
			throw new BaseException(exc.getMessage());
		}
		jdlString = jad.toString ( );
		// Prints out the input parameters
		System.out.println ("TEST : JobRegister");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL-FILE		= [" + jdlFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL			= [" + jdlString + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// test
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		result= client.jobRegister( jdlString, delegationId );
		// test results
		if ( result != null ) {
			System.out.println ("RESULT:");
			System.out.println ("=======================================================================");
			printResult ( result );
			System.out.println("=======================================================================");
		}
		// end
		System.out.println ("End of the test");
	}

	public static void main(String[] args) throws Exception {
		// input parameters
		String url = "" ;
		String jdlFile = "" ;
		String delegationId = "";
		String proxyFile = "";
		try {
			// Reads the  input arguments
			if ((args == null) || (args.length < 4))
				throw new BaseException ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> <delegationID> <JDL-FIlePath>)");
			url = args[0];
			proxyFile = args[1];
			delegationId = args[2];
			jdlFile = args[3];
			runTest ( url, proxyFile, delegationId, jdlFile);
		} catch (BaseException exc) {
			System.out.println("\nException caught:\n");
			System.out.println("Message:\n----------\n"+ exc.getMessage() + "\n");
			System.err.println("Stack Trace:\n------------");
                        exc.printStackTrace();
		}
	}
}
