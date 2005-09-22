
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 *
 * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */

package org.glite.wms.wmproxy.jobpurge ;

import org.glite.wms.wmproxy.WMProxyAPI ;
/*
	Test of  "jobPurge" method in org.glite.wms.wmproxy.WMProxyAPI

*/
public class WMProxyJobPurgeTest {
	/**
	* Default constructor
	*/
	public WMProxyJobPurgeTest ( ) { }
	/**
	*	Starts the test
	*	@param url service URL
	*  	@param jobId the id to identify the job
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String jobId ) throws java.lang.Exception {
		// Prints out the input parameters
		System.out.println ("TEST : JobPurge");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID		= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");			// Testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		client.jobPurge( jobId );
		System.out.println ("End of the test" );

	}
	/*
	* main
	*/
	public static void main(String[] args) throws Exception {
		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		// Reads the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> <JobId>)");
		url = args[0];
		proxyFile = args[1];
		jobId = args[2];
		runTest ( url, proxyFile, jobId);
	 }

 }
