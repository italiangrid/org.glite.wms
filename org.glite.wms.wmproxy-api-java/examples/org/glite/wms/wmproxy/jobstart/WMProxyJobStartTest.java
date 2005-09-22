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


public class WMProxyJobStartTest {

	public WMProxyJobStartTest ( ) { }

	public static void runTest ( String url, String proxyFile, String jobId ) throws java.lang.Exception {
		// Prints the input parameters
		System.out.println ("TEST : JobStart");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 	= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");		System.out.println ("proxyFile	= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID		= [" +jobId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// testing
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		client.jobStart( jobId );
		System.out.println ("The job has been successfully started.\nEnd of the test");
	}

	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		// Reads the input parameters
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> <JobId>)");
		url = args[0];
		proxyFile = args[1] ;
		jobId = args[2] ;
		runTest (url, proxyFile, jobId );
	 }

 }
