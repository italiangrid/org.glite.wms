/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.createproxy;

import org.glite.wms.wmproxy.WMProxyAPI;

/*
	Test of  "createProxyfromCertReq" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyCreateProxyTest {

	WMProxyCreateProxyTest ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*  	@param delegationID the id to identify the delegation
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest(String url, String delegationId, String proxyFile)
					throws java.lang.Exception  {

		String certReq = "";
		String result = "" ;

		System.out.println ("TEST : CreateProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL			= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("PROXY-FILE		= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// client object
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;

		// Starting test
		System.out.println ("Testing ....");

		// Proxy Request
		System.out.println ("ProxyRequest .....");
		certReq = client.getProxyReq ( delegationId );
		System.out.println ("getProxy result [\n." + certReq + "]" );

		// Create Proxy from "ProxyRequest" result
		System.out.println ("createProxy calling .....");
		result = client.createProxyfromCertReq ( certReq );

		// end: Display result
		System.out.println ("end of the test\nresult = [\n" + result + "]" );

	 }

	public static void main(String[] args) throws java.lang.Exception {

		String url = "" ;
		String delegationId = "";
		String proxyFile = "";

		// input parameters
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <Delegation-ID>  <ProxyFile>)");
		url = args[0];
		delegationId = args[1];
		proxyFile = args[2];

		runTest(url, delegationId, proxyFile);

 	}
 }
