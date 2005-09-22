
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.inputsbsize ;

import org.glite.wms.wmproxy.WMProxyAPI ;

/*
*
*	Test of  "getMaxInputSandboxSize" method in org.glite.wms.wmproxy.WMProxyAPI
*
*/

public class WMProxyInputSandboxSizeTest {

	public WMProxyInputSandboxSizeTest ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile ) throws java.lang.Exception {

		double result = 0;

		// Prints out the input parameters
		System.out.println ("TEST : MAX-InputSandboxSize");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....\n");
		result = client.getMaxInputSandboxSize();
		System.out.println ("\nEnd of the test. Result:");
		// prints the result +  end
		System.out.println ("MAX InputSandbox size = [" + result + "]");
	 }

	public static void main(String[] args) throws java.lang.Exception {

		String url = "" ;
		String proxyFile = "" ;


		// Reads the input arguments
		if ((args == null) || (args.length < 2))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile>)");
		url = args[0];
		proxyFile = args[1];

		runTest ( url, proxyFile);

	 }

 }
