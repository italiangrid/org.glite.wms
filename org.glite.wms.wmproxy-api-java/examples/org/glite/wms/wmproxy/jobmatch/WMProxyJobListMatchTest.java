
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.jobmatch;

import org.glite.wms.wmproxy.WMProxyAPI;

import org.glite.wms.wmproxy.StringAndLongType ;
import org.glite.wms.wmproxy.StringAndLongList ;

import org.glite.wms.jdlj.JobAd ;

/*
	Test of  "jobListMatch" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyJobListMatchTest {

	public WMProxyJobListMatchTest ( ) { }

	// Print the results
	private static void printResult ( StringAndLongList entry) {

		int size = 0;

		if ( entry != null ) {
	/*
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
		*/

		}
	}

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jdlFile the path location of the JDL file
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jdlFile, String proxyFile ) throws java.lang.Exception {

		// jdl
		String jdlString = "";

		// output results
		StringAndLongList result = null;

		// reads JDL
		JobAd jad = new JobAd ( );
		jad.fromFile ( jdlFile );
		jdlString = jad.toString ( );

		// Prints out the input parameters
		System.out.println ("TEST : JobListMatch");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL-FILE		= [" + jdlFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL			= [" + jdlString + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// test
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("testing ....");
		result= client.jobListMatch( jdlString );


		// test results
		if ( result != null ) {
			System.out.println ("RESULT:");
			System.out.println ("=======================================================================");
			printResult ( result );
			System.out.println("=======================================================================");
		}

		// end
		System.out.println ("end of the test");
	}

	public static void main(String[] args) throws Exception {

		// input parameters
		String url = "" ;
		String jdlFile = "" ;
		String proxyFile = "";



		// reads the  input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <JDL-FIlePath>  <proxyFile>)");
		url = args[0];
		jdlFile = args[1];
		proxyFile = args[2];

		runTest ( url, jdlFile, proxyFile);


	}
}
