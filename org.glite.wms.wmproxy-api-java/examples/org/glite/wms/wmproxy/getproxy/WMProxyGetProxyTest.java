
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.getproxy ;

import org.glite.wms.wmproxy.WMProxyAPI;

/*
*
*	Test of  "getProxyReq" method in org.glite.wms.wmproxy.WMProxyAPI
*
*/


public class WMProxyGetProxyTest {

	WMProxyGetProxyTest ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*  	@param delegationID the id to identify the delegation
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String delegationID, String proxyFile ) throws java.lang.Exception {
		// result
		String result = "" ;

		// Prints the input parameters
		System.out.println ("TEST : GetProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID		= [" + delegationID + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxyFile		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// testing ....
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		result = client.getProxyReq( delegationID );

		// Display the result
		System.out.println ("=======================================================================");
		System.out.println ("result = [\n" + result + "]" );
		System.out.println ("=======================================================================");
	}

	public static void main(String[] args) throws java.lang.Exception {

		// input parameters
		String url = "" ;
		String delegationID = "" ;
		String proxyFile = "";

		// Reads the input parameters
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <DelegationID> <proxy-File>)");
		url = args[0];
		delegationID = args[1];
		proxyFile = args[2];

		runTest ( url, delegationID, proxyFile);



	 }

 }
