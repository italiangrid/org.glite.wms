
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
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String delegationId, String jdlFile, String certsPath   ) throws  org.glite.wms.wmproxy.BaseException {
		//jdl
		String jdlString = "";
		// output results
		JobIdStructType result = null;
		WMProxyAPI client = null;
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
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		 }
		 // test
		System.out.println ("Testing ....");
		result= client.jobRegister( jdlString, delegationId );
		// test results
		if ( result != null ) {
			System.out.println ("RESULT:");
			System.out.println ("=======================================================================");
			System.out.println ("Your job has been successfully registered:");
			printResult ( result );
			System.out.println("=======================================================================");
		}
		// end
		System.out.println ("End of the test");
	}

	public static void main(String[] args) throws Exception {
		String url = "" ;
		String jdlFile = "" ;
		String proxyFile = "";
		String delegationID = "";
		String certsPath = "";
		// Reads the  input arguments
		if ((args == null) || (args.length < 4)) {
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <delegationID> <proxyFile> <JDL-FIlePath>  [CAs paths (optional)])");
		} else if (args.length > 5) {
			 throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <delegationID> <proxyFile> <JDL-FIlePath>  [CAs paths (optional)]");
		}
		url = args[0];
                delegationID = args[1];
		jdlFile = args[2];
		proxyFile = args[3];
		if (args.length == 5) {
			certsPath = args[4];
		} else  {
			certsPath = "";
		}
		// Launches the test
		runTest ( url, jdlFile, delegationID, proxyFile, certsPath);
	}
}
