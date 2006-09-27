
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.sbulkdesturi ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.DestURIsStructType;
import org.glite.wms.wmproxy.DestURIStructType;
import org.glite.wms.wmproxy.StringList ;
// Retrieves the information on server available protocols
import org.glite.wms.wmproxy.common.GetParameters;
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
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jobId , String proxyFile, String certsPath   ) throws java.lang.Exception {
		DestURIsStructType result ;
		WMProxyAPI client = null;
		String protocol = "";
		String id = "";
		int size = 0;
		int num = 0;
		// Prints out the input parameters
		System.out.println ("TEST : SandboxBulkDestURI ");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID	 		= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
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
			System.out.println ("\nPROTOCOL			= [DEFAULT]\n" );
		 } else {
			System.out.println ("\nPROTOCOL			= [" + protocol + "]\n" );
		 }
		System.out.println ("Testing....\n");
		result = (DestURIsStructType) client.getSandboxBulkDestURI(jobId,protocol ) ;
		// string list
		DestURIStructType[] list = (DestURIStructType[])result.getItem( );
		// prints the results
		System.out.println ("\nEnd of the test\n\nResult:");
		System.out.println ( "=====================================================================================================================================================");
		num = list.length;
		for (int i = 0; i < num ; i++){
			if ( i> 1){
				System.out.println ( "-------------------------------------------------------------------------------------------------------------- ");
			}
			id = list[i].getId( ) ;
			if ( id.equals(jobId) ) {
				System.out.println ( "\n> ROOT: " + id );
			} else {
				System.out.println ( "\n> child node: " + id );
			}
			String[] uris = (String[] )list[i].getItem( );
			if (uris != null){
				size = uris.length ;
				for (int k=0; k < size ; k++){
					System.out.println ( "    - " + uris[k]);
				}
			}
		}
		System.out.println ( "\n=====================================================================================================================================================");

	}

	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		String certsPath = "";
		// input parameters
		if ((args == null) || (args.length < 3)){
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile> [CAs paths (optional)] )");
		} else if (args.length > 4) {
			 	 throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <jobId> <proxyFile> [CAs paths (optional)] )");
		}
		url = args[0];
		proxyFile = args[1];
		jobId = args[2];
		if (args.length == 4) {
			certsPath = args[3];
		} else  {
			certsPath = "";
		}
		// Test
		runTest ( url, jobId, proxyFile, certsPath);
	 }
 }
