
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.freequota ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import javax.xml.rpc.holders.LongHolder ;

/*
*
*	Test of  "getFreeQuota" method in org.glite.wms.wmproxy.WMProxyAPI
*
*/

public class WMProxyFreeQuotaTest {

	public WMProxyFreeQuotaTest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile ) throws  java.lang.Exception {

		// output results
		LongHolder softLimit = new LongHolder( ) ;
		LongHolder hardLimit = new LongHolder( ) ;


		// Prints out the input parameters
		System.out.println ("TEST : FreeQuota");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("testing....");
		client.getFreeQuota(softLimit, hardLimit) ;

		// result
		System.out.println ("RESULT:");
		System.out.println ("===================================================");
		System.out.println ("softLimit	= [" + softLimit.value+ "]");
		System.out.println ("hardLimit	= [" + hardLimit.value + "]");
		System.out.println ("===================================================");
		System.out.println ("end of the test" );
	 }

	public static void main(String[] args) throws java.lang.Exception {

		// input parameters
		String url = "" ;
		String proxyFile = "";


		// Read the input arguments
		if ((args == null) || (args.length < 2))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile>)");
		url = args[0];
		proxyFile = args[1];

		runTest ( url, proxyFile );

	 }

 }
