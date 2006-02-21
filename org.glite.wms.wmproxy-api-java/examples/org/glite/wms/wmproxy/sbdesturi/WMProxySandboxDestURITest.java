
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.sbdesturi ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList ;
// INPUT OBJECTS
import java.io.InputStreamReader ;
import java.io.BufferedReader ;

/*
	Test of  "getSandboxDestURI" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxySandboxDestURITest {

	public WMProxySandboxDestURITest  ( ) { }

	/**
	* Returns the chosen protocol by user among the available ones
	* or empty string (it means: all protocols)
	*/
	 public static String askForProtocol (WMProxyAPI client ) {
		boolean retrieve = false;
		InputStreamReader isr = null ;
		BufferedReader stdin = null ;
		String input = "";
		String numbers = "";
		String protocol = "";
		Integer conv = null;
		String[] protocols = null;
		StringList protoList = null ;
		String[]  list = null;
		int value = 0;
		int size = 0;
		// question to users
		String question = "\nDo you want to retrieve the list of WMProxy available protocols [y|n] ?";
		System.out.println (question);
		System.out.println ("y = contacting the server to retrieve the list");
		System.out.println ("n = retrieve the list of DestinationURI for all available protocols");
		isr = new InputStreamReader( System.in );
		stdin = new BufferedReader( isr );
		while (true) {
			try {
				input = stdin.readLine();
			} catch (java.io.IOException exc){
				continue;
			}
			input = input.trim();
			if (input.equals("y")) {
				retrieve = true;
				break;
			} else if (input.equals("n")) {
				retrieve = false;
				break;
			} else {
				System.out.println (question);
			}
		}
		if (retrieve) {
			// Retrieving the protocol list ....
			System.out.println( "Getting the list of available protocols from the endpoint " + client.getEndPoint());
			try {
				protoList = client.getTransferProtocols( );
				if (protoList == null) {
					System.out.println("You have received an empty list (Null Pointer received)");
					protocol = "";
				}

			} catch (Exception exc) {
				System.out.println ("\nWARNING Unable to retrieve the list of protocols;");
				System.out.println ("exception caught (" + exc.getClass().getName() + ") - " + exc.getMessage( ) + "\n");
				protocol = "";
			}
			if (protoList != null) {
				list = (String[])protoList.getItem( );
				size = list.length ;
				if (size==0) {
					// The WMProxy doesn't have the list
					System.out.println("You have received an empty list.");
				} else if (size==1) {
					// list with only one element
					protocol = list[0];
					System.out.println("The only available protocol is: " + protocol);
				} else {
					// choosing a protocol .....
					System.out.println( "List of WMProxy available protocols:" );
					System.out.println ("===========================================");
					for (int i = 0; i < size; i++) {
						System.out.println ("" + (i+1) + " - " + list[i]);
						if ( i>0 && i<size){ numbers += " | ";}
						numbers += "" + (i+1);
					}
					System.out.println ("a - all protocols in the list above");
					while (true) {
						System.out.println ("---------------------------");
						System.out.println ("Choose one protocol in the list [" + numbers + "]");
						System.out.println ("a = all protocols");
						try {
							input = stdin.readLine();
						} catch (java.io.IOException exc){
							continue;
						}
						// removes white spaces (if there is any)
						input = input.trim();
						if (input.equals("a") ) {
							protocol = "all";
							break;
						} else {
							try {
								conv = new Integer(input.trim());
								value = conv.intValue();
								if (value > 1 || value < size){
									protocol = list[value-1];
									break;
								}
							} catch ( NumberFormatException exc){
								// invalid value: repeats this loop
							}
						}
					}
				}
			}

		} else {
			// empty string = all protcols
			protocol = "all";
		}
		return protocol;
    	}
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
		 protocol = askForProtocol(client);
		System.out.println ("\nPROTOCOL			= [" + protocol + "]\n" );

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
