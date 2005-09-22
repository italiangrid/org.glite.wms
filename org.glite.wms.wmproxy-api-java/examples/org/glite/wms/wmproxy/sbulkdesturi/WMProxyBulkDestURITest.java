
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.sbulkdesturi ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.DestURIsStructType;

/*
	Test of  "getSandboxBulkDestURI" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyBulkDestURITest {

	public WMProxyBulkDestURITest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jobID the id to identify the job
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jobId , String proxyFile ) throws java.lang.Exception {
		DestURIsStructType result ;
		int size = 0;
		// Prints out the input parameters
		System.out.println ("TEST : SandboxBulkDestURI ");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID	 		= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing....\n");
		result = (DestURIsStructType) client.getSandboxBulkDestURI( jobId ) ;
		// string list
org.glite.wms.wmproxy.DestURIStructType[] list = (org.glite.wms.wmproxy.DestURIStructType[])result.getItem( );
		// prints the results
		System.out.println ("\nEnd of the test\n\nResult:");
		System.out.println ( "=====================================================================================================================================================");
		for (int i = 0; i < list.length ; i++){
			System.out.println ( "\n>> " + list[i].getId( ) );
			System.out.println ( "-------------------------------------------------------------- ");
			String[] uris = (String[] )list[i].getItem( );
			if (uris != null){
				size = uris.length ;
				for (int k=0; k < size ; k++){
					System.out.println ( "- " + uris[k]);
				}
			}
		}
		System.out.println ( "\n=====================================================================================================================================================");

	}

	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";


		// Reads the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <JobId> <proxyFile>)");
		url = args[0];
		jobId = args[1];
		proxyFile = args[2];

		runTest ( url, jobId, proxyFile );

	 }

 }
