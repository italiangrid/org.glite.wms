
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.outputlist ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringAndLongList ;
import org.glite.wms.wmproxy.StringAndLongType ;
import org.glite.wms.wmproxy.StringList ;
// UI-API-JAVA
import org.glite.wmsui.apij.JobId ;
// Retrieves the information on server available protocols
import org.glite.wms.wmproxy.common.GetParameters;
/*
	Test of  "getOutputFileList" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyOutputListTest {
	/**
	* Default constructor
	*/
	public WMProxyOutputListTest ( ) { }

	/**
	*	Starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*  	@param jobID the id to identify the job
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url,  String proxyFile, String jobId, String certsPath) throws java.lang.Exception {
		WMProxyAPI client = null;
		String protocol = "";
		StringAndLongList result = null;
                StringAndLongType[ ] list = null;
		int size = 0;
		// Prints  the input parameters
		System.out.println ("TEST : getOutputFileList");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxyFile		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID			= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		}
		// protocol
		 protocol = GetParameters.askForProtocol(client);
		 if (protocol.compareTo(GetParameters.DEFAULT_PROTOCOL_VALUE)==0) {
			System.out.println ("\nPROTOCOL			= [ALL PROTOCOLS]\n" );
		 } else {
			System.out.println ("\nPROTOCOL			= [" + protocol + "]\n" );
		 }
		// Testing .....
		System.out.println ("Calling the getOutputFileList service.....");
		result = client.getOutputFileList(jobId, protocol);
		System.out.println ("End of the test\n" );
		// test results
		System.out.println ("Result:");
		System.out.println ("=======================================================================");
		if ( result != null ) {
			// list of files+their size
			list = (StringAndLongType[ ] ) result.getFile ( );
			if ( list != null ){
				size = list.length;
				for (int i = 0; i < size ; i++){
					System.out.println ("file n. " + (i+1) );
					System.out.println ("--------------------------------------------");
					System.out.println ("name		= [" + list[i].getName ( ) + "]" );
					System.out.println ("size		= [" + list[i].getSize ( ) + "]" );
				}
			} else {
				System.out.println ( "No output files for this job!");
			}
		} else {
			System.out.println ( "An empty list has been received");
		}
		System.out.println("=======================================================================");
	}
	/**
	* main
	*/
	public static void main(String[] args) throws Exception  {
		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		String certsPath = "";
		// input parameters
		if ((args == null) || (args.length < 3)){
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile> [CAs paths (optional)] )");
		} else if (args.length > 4) {
			throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL>  <jobId> <proxyFile> [CAs paths (optional)] )");
		}
		url = args[0];
		// checks the jobid
		JobId id = new JobId(args[1]);
		jobId = id.toString( );
		// proxyFile
		proxyFile = args[2];
		if (args.length == 4) {
			certsPath = args[3];
		} else  {
			certsPath = "";
		}
		// test
		runTest ( url, proxyFile, jobId, certsPath);
	 }
	 /**
	 * 	Protocol constants
	 */
	private static final String ALL_PROTOCOLS	= "all";
	private static final String DEFAULT_PROTOCOL_VALUE = ALL_PROTOCOLS;
}
