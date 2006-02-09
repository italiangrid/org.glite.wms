
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.jobmatch;

import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.StringAndLongType ;
import org.glite.wms.wmproxy.StringAndLongList ;
import org.glite.jdl.JobAd ;

/*
	Test of  "jobListMatch" method in org.glite.wms.wmproxy.WMProxyAPI

*/
public class WMProxyJobListMatchTest {
	/*
	* Constructor
	*/
	public WMProxyJobListMatchTest ( ) { }
	/*
	* Returns a string with a number of white spaces depending on
	* the length of the input string
	*/
	private static String getTab(String s) {
		int t = 50 - s.length();
		String ws = "";
		for (int i = 0; i < t ; i++) {
			ws += " ";
		}
		return ws;
	}
	/**
	*	Starts the test
	*	@param url service URL
	*  	@param jdlFile the path location of the JDL file
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jdlFile, String delegationID, String proxyFile, String certsPath  ) throws java.lang.Exception {
		// jdl
		WMProxyAPI client = null;
		String jdlString = "";
		// output results
		StringAndLongList result = null;
		StringAndLongType[ ] list = null;
		// Prints out the input parameters
		System.out.println ("TEST : JobListMatch");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL-FILE		= [" + jdlFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		System.out.println ("delegationID		= [" + delegationID + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// Reads JDL
		JobAd jad = new JobAd ( );
		jad.fromFile ( jdlFile );
		jdlString = jad.toString ( );
		System.out.println ("jdlString		= [" + jdlString + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		 }
		 // Test
		System.out.println ("Testing ....");
		result= client.jobListMatch( jdlString, delegationID );
		System.out.println ("End of the test.\n");
		// Results
		if ( result != null ) {
			System.out.println ("Result:");
			System.out.println ("=======================================================================");
			// list of CE's+their ranks
			list = (StringAndLongType[ ] ) result.getFile ( );
			if (list != null) {
				int size = list.length ;
				for (int i = 0; i < size ; i++) {
					String ce = list[i].getName( );
					System.out.println ( "- " + ce + getTab(ce) + list[i].getSize( ) );
				}
			} else {
				System.out.println ( "No Computing Element matching your job requirements has been found!");
			}
			System.out.println ("=======================================================================");
		}
	}
	/**
	* main
	*/
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
		proxyFile = args[2];
		jdlFile = args[3];
		if (args.length == 5) {
			certsPath = args[4];
		} else  {
			certsPath = "";
		}
		// Launches the test
		runTest ( url, jdlFile, delegationID, proxyFile, certsPath);
	}
}
