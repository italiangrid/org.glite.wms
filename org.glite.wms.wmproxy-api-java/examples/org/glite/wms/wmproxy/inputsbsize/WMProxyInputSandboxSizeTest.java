/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


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
