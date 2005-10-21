
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.getproxy;

import org.glite.wms.wmproxy.WMProxyAPI;

public class WMProxyGrstGetProxyTest {

/*
	Test of  "grstGetProxy" method in org.glite.wms.wmproxy.WMProxyAPI

*/
	WMProxyGrstGetProxyTest ( ) { }
	/*
	*	Starts the test
	*	@param url service URL
	*  	@param delegationID the id to identify the delegation
	*  	@param propFile the path location of the user properties file
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String delegationId, String propFile, String proxyFile ) throws java.lang.Exception {
		// proxies
		String certReq = "";
		String proxy = "" ;
		String result = "" ;

		System.out.println ("TEST : GridSite getProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL			= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// Test
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		// Proxy Request
		System.out.println ("Performing Proxy-Request .....");
		certReq = client.grstGetProxyReq ( delegationId );
		System.out.println ("getProxy result [\n." + certReq + "]" );
		// end
		System.out.println ("End of the test\n");
	}
	/**
	*	main
	*/
	public static void main(String[] args) throws Exception {
		String url = "" ;
		String delegationId = "";
		String propFile = "";
		String proxyFile = "";
		// input parameters
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <Delegation-ID> <proxyFile>)");
		url = args[0];
		delegationId = args[1];
		proxyFile = args[2];
		// Launches the test
		runTest (url, delegationId, propFile, proxyFile);


	 }

 }
