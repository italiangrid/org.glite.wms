package org.glite.wms.wmproxy.jobstart;

import org.glite.wms.wmproxy.WMProxyAPI;


public class WMProxyJobStartTest {

	public WMProxyJobStartTest ( ) { }

	public static void runTest ( String url, String jobId, String proxyFile ) throws java.lang.Exception {
		// Prints the input parameters
		System.out.println ("TEST : JobStart");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 	= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID		= [" +jobId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxyFile	= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// testing
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("testing ....");
		client.jobStart( jobId );
		System.out.println ("end of the test");
	}


	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";

		// Reads the input parameters
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <JobId> <proxyFile>)");
		url = args[0];
		jobId = args[1] ;
		proxyFile = args[2] ;

		runTest (url, jobId, proxyFile );
	 }

 }
