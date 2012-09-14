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

 package org.glite.wms.wmproxy.version ;

import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.BaseException;
/*

	Test of  "getVersion" method in org.glite.wms.wmproxy.WMProxyAPI

*/


public class WMProxyVersionTest {

	WMProxyVersionTest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile ) throws org.glite.wms.wmproxy.BaseException {

		String result = "";

		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		result = client.getVersion( ) ;
		System.out.println ("Version=[" +  result + "]" );
		System.out.println ("end of the test\n" );
	}

	public static void main(String[] args) throws Exception {

		String url = "";
		String proxyFile = "";
		try {
			// input parameters
			if ((args == null) || (args.length < 2))
				throw new BaseException ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile>)");
			url = args[0];
			proxyFile = args[1];

			System.out.println ("TEST : Version");
			System.out.println ("************************************************************************************************************************************");
			System.out.println ("WS URL	 	= [" + url + "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
			System.out.println ("proxyFile	 = [" + proxyFile + "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

			runTest ( url, proxyFile);
		} catch (BaseException exc) {
			System.out.println("\nException caught:\n");
			System.out.println("Message:\n----------\n"+ exc.getMessage() + "\n");
			System.err.println("Stack Trace:\n------------");
                        exc.printStackTrace();
		}

	 }

 }
