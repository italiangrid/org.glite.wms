
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

 package org.glite.wms.wmproxy.jobsubmit;

import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.JobIdStructType;

import org.glite.wms.jdlj.JobAd ;


/*
	Test of  "jobSubmit" method in org.glite.wms.wmproxy.WMProxyAPI

*/



public class WMProxyJobSubmitTest {

	WMProxyJobSubmitTest ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jdlFile the path location of the JDL file
	*  	@param delegationID the id to identify the delegation
	*	@param propFile the path location of the configuration file
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jdlFile, String delegationId, String proxyFile ) throws java.lang.Exception {

		JobIdStructType jobId = null ;
		String jdlString = "";
		String result = "" ;

		// reads the jdl
		JobAd jad = new JobAd ( );
		jad.fromFile ( jdlFile );
		jdlString = jad.toString ( );

		System.out.println ("TEST : JobSubmit");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID	= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL-FILE	= [" + jdlFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL		= [" + jdlString + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxyFile	= [" +proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// testing
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		jobId = client.jobSubmit( jdlString, delegationId );
		System.out.println ("end of the test\nresult = [" + result + "]" );
	}

	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jdlFile = "" ;
		String delegationId = "";
		String proxyFile = "";

		// input parameters
		if ((args == null) || (args.length < 4))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <JDL-FIlePath> <proxyFile>)");
		url = args[0];
		jdlFile = args[1];
		delegationId = args[2];
		proxyFile = args[3];

		runTest ( url, jdlFile, delegationId, proxyFile );

	 }

 }
