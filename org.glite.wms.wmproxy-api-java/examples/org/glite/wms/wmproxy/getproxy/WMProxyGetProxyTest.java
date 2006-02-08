
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.getproxy;

import org.glite.wms.wmproxy.WMProxyAPI;

public class WMProxyGetProxyTest {

/*
	Test of  "getProxy" method in org.glite.wms.wmproxy.WMProxyAPI

*/
	WMProxyGetProxyTest ( ) { }
	/*
	*	Starts the test
	*	@param url service URL
	*  	@param delegationID the id to identify the delegation
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String delegationId,String proxyFile, String certsPath ) throws java.lang.Exception {

		// proxies
		String certReq = "";
		String proxy = "" ;
		String result = "" ;
		WMProxyAPI client = null;

		System.out.println ("TEST : WMProxy getProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL			= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		 }
		System.out.println ("Testing ....");
		// Proxy Request
		System.out.println ("Performing Proxy-Request .....");
		certReq = client.getProxyReq ( delegationId );
		System.out.println ("getProxy result [\n." + certReq + "]" );
		// end
		System.out.println ("End of the test\n");
	}
	/**
	*	main
	*/
	public static void main(String[] args){
		String url = "" ;
		String delegationId = "";
		String proxyFile = "";
		String certsPath = "";
		try {
			// input parameters
			if ((args == null) || (args.length < 3)){
				throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <Delegation-ID> <proxyFile>[CAs paths (optional)] )");
			} else if (args.length > 4) {
			  throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <Delegation-ID> <proxyFile> [CAs paths (optional)] )");
			}
			url = args[0];
			delegationId = args[1];
			proxyFile = args[2];

			if (args.length == 4) {
			   certsPath = args[3];
			} else  {
			   certsPath = "";
			}
			// Launches the test
			runTest (url, delegationId, proxyFile, certsPath);
		} catch (Exception exc){
			System.out.println (exc.toString( ));
		}
	 }
 }
