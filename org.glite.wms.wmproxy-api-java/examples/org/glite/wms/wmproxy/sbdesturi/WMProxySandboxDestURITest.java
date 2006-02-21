
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.sbdesturi ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList ;
// Retrieves the information on server available protocols
import org.glite.wms.wmproxy.common.GetProtocols
 ;
/*
	Test of  "getSandboxDestURI" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxySandboxDestURITest {

	public WMProxySandboxDestURITest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jobID the id to identify the job
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jobId , String proxyFile , String certsPath  ) throws java.lang.Exception {
		WMProxyAPI client = null;
		StringList result = null;
		String protocol = null;
		int size = 0;
		// Prints out the input parameters
		System.out.println ("TEST : SandboxDestURI ");
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
		 protocol = GetProtocols.askForProtocol(client);
		 if (protocol.compareTo(GetProtocols.DEFAULT_PROTOCOL_VALUE)==0) {
			System.out.println ("\nPROTOCOL			= [DEFAULT]\n" );
		 } else {
			System.out.println ("\nPROTOCOL			= [" + protocol + "]\n" );
		 }
		 // testing ...
		System.out.println ("Testing....");
		result = (StringList) client.getSandboxDestURI( jobId, protocol ) ;
		// string list
		String[] uris = (String[] )result.getItem( );
		// prints the results
		System.out.println ("\nend of the test\nresult:");
		System.out.println ( "=====================================================================================================================================================\n");
		if (uris != null){
			size = uris.length ;
			for (int i=0; i < size ; i++) {
				System.out.println ( "- " + uris[i] + "\n");
			}
		}
		System.out.println ( "=====================================================================================================================================================");
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
			throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL>  <jobId> <proxyFile> [CAs paths (optional)] )");
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
