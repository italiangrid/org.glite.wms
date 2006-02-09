/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
  * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */
package org.glite.wms.wmproxy.jobstart;

import org.glite.wms.wmproxy.WMProxyAPI;
// UI-API-JAVA
import org.glite.wmsui.apij.JobId ;

public class WMProxyJobStartTest {

	public WMProxyJobStartTest ( ) { }
	/*
	*	Starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*  	@param jobID the id to identify the job
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url,  String proxyFile, String jobId, String certsPath) throws java.lang.Exception {
		WMProxyAPI client = null;
		// Prints the input parameters
		System.out.println ("TEST : JobStart");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 	= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");		System.out.println ("proxyFile	= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID		= [" +jobId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		}
		// testing
		System.out.println ("Testing ....");
		client.jobStart( jobId );
		System.out.println ("The job has been successfully started.\nEnd of the test");
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

 }
