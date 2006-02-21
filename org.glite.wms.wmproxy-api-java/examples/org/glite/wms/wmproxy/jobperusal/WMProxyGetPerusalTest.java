
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.jobperusal ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList ;
// UI-API-JAVA
import org.glite.wmsui.apij.JobId ;
// Retrieves the information on server available protocols
import org.glite.wms.wmproxy.common.GetProtocols;
/*
	Test of  "getPerusalFiles" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyGetPerusalTest {

	public  WMProxyGetPerusalTest( ) { }


	/**
	*	Gets the string with the result of the operations:
	*	either
	*	> a message saying that there are no files to be retrieved
	*	or
	*	> the list of URIs related to the files to be retrieved
	*/
	public static String getStringResult (StringList fileList ) {
		String result = "";
		String[ ] files = (String[]) fileList.getItem( );
		if (files != null) {
			int size = files.length ;
			if (size > 0) {
				result = "List of files to be retrieved:\n";
				result += "----------------------------------------------------------------------------------------------------------\n";
				for (int i = 0; i < size; i++) {
					result += " - " + files[i] + "\n" ;
				}
			} else {
				result = "No files to be retrieved\n";
			}
		} else {
				result = "No files to be retrieved\n";
		}
		return result;
	}

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jobID the id to identify the job
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jobId , String fileName, String proxyFile, String certsPath   ) throws java.lang.Exception {
		WMProxyAPI client = null;
		StringList fileList = null;
		StringList result = null;
		String protocol = "";
		int size = 0;
		int num = 0;
		// Prints out the input parameters
		System.out.println ("TEST : getPerusalFiles");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID	 		= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("filename	 	= [" + fileName + "]" );
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
			System.out.println ("\nPROTOCOL			= [ALL PROTOCOLS]\n" );
		 } else {
			System.out.println ("\nPROTOCOL			= [" + protocol + "]\n" );
		 }
		 // Calling the WMProxy service with allchunck = false
		System.out.println ("Test 1: allchunck = false");
		System.out.println ("--------------------------------");
		System.out.println ("Calling the service: getPerusalFiles ...");
		result =  client.getPerusalFiles (jobId, fileName, false,protocol) ;
		System.out.println ("\nEnd of the test\n\nResult:");
		System.out.println(getStringResult(result));
		System.out.println ( "==================================================================================================================\n");
		// Calling the WMProxy service with allchunck = true
		System.out.println ("Test 2: allchunck = true");
		System.out.println ("--------------------------------");
		System.out.println ("Calling the service: getPerusalFiles ...");
		result =  client.getPerusalFiles (jobId, fileName,true, protocol) ;
		System.out.println ("\nEnd of the test\n\nResult:");
		System.out.println(getStringResult(result));
	}

	public static void main(String[] args) throws Exception {
		String url = "" ;
		String jobId = "" ;
		String fileName = "";
		String proxyFile = "";
		String certsPath = "";
		// input parameters
		if ((args == null) || (args.length < 4)){
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <jobId> <filename> <proxyFile> [CAs paths (optional)] )");
		} else if (args.length > 5) {
			throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL>  <jobId> <filename> <proxyFile> [CAs paths (optional)] )");
		}
		url = args[0];
		// checks the jobid
		JobId id = new JobId(args[1]);
		jobId = id.toString( );
		// filename
		fileName = args[2];
		// proxyFile
		proxyFile = args[3];
		if (args.length == 5) {
			certsPath = args[4];
		} else  {
			certsPath = "";
		}
		// Test
		runTest ( url, jobId, fileName, proxyFile, certsPath);
	 }

 }
