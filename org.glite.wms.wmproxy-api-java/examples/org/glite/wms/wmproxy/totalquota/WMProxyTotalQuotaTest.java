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

package org.glite.wms.wmproxy.totalquota ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import javax.xml.rpc.holders.LongHolder ;

/*

	Test of  "getTotalQuota" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyTotalQuotaTest {

	public WMProxyTotalQuotaTest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile ) throws java.lang.Exception {

		// output results
		LongHolder softLimit = new LongHolder( ) ;
		LongHolder hardLimit = new LongHolder( ) ;

		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("testing....");
		client.getTotalQuota(softLimit, hardLimit) ;

		// result
		System.out.println ("RESULT:");
		System.out.println ("===================================================");
		System.out.println ("softLimit=[" + softLimit.value+ "]");
		System.out.println ("hardLimit=[" + hardLimit.value + "]");
		System.out.println ("===================================================");
		System.out.println ("end of the test" );
	}

	public static void main(String[] args) throws Exception {

		// input parameters
		String url = "" ;
		String proxyFile = "";


		// Read the input arguments
		if ((args == null) || (args.length < 2))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile>)");
		url = args[0];
		proxyFile = args[1];

		// Print out the input parameters
		System.out.println ("TEST : TotalQuota");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		runTest( url, proxyFile );
	 }

 }
