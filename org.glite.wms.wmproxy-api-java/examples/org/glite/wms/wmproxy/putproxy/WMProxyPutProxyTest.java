
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.putproxy;

import org.glite.wms.wmproxy.WMProxyAPI;


public class WMProxyPutProxyTest {

/*
	Test of  "putProxy" method in org.glite.wms.wmproxy.WMProxyAPI

*/
	WMProxyPutProxyTest ( ) { }
	/*
	*	starts the test
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

		System.out.println ("TEST : PutProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL			= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("PROPERTIES-FILE		= [" + propFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");


		// test
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");

		// Proxy Request
		System.out.println ("performing Proxy-Request .....");
		certReq = client.getProxyReq ( delegationId );
		System.out.println ("getProxy result [\n." + certReq + "]" );

		// Create proxy from "ProxyRequest" result
		System.out.println ("creating Proxy  .....");
		proxy = client.createProxyfromCertReq ( certReq, delegationId, propFile );
		System.out.println ("proxy = [\n." + proxy + "]" );

		System.out.println ("putting Proxy .....");
		// Put prxoy
		client.putProxy (delegationId, proxy);

		// end
		System.out.println ("end of the test\n");

	}

	public static void main(String[] args) throws Exception {

		// input parameters
		String url = "" ;
		String delegationId = "";
		String propFile = "";
		String proxyFile = "";



		// input parameters
		if ((args == null) || (args.length < 4))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <Delegation-ID> <Properties-FIle> <proxyFile>)");
		url = args[0];
		delegationId = args[1];
		propFile = args[2];
		proxyFile = args[3];

		runTest (url, delegationId, propFile, proxyFile);


	 }

 }
