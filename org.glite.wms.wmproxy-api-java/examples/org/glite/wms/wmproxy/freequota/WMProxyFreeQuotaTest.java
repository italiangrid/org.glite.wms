
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
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String certsPath ) throws  java.lang.Exception {
		WMProxyAPI client = null;
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
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		 }

		System.out.println ("Calling the getFreeQuota service");
		client.getFreeQuota(softLimit, hardLimit) ;
		// result
		System.out.println ("RESULT:");
		System.out.println ("===================================================");
		if (softLimit.value < 0) {
			System.out.println ("The Soft-Limit is not set");
		} else {
			System.out.println ("Soft-Limit	= [" + softLimit.value+ "]");
		}
		if (hardLimit.value < 0) {
			System.out.println ("The Hard-Limit is not set");
		} else {
			System.out.println ("Soft-Limit	= [" + hardLimit.value+ "]");
		}
		System.out.println ("===================================================");
		System.out.println ("End of the test" );
	 }

	public static void main(String[] args) throws java.lang.Exception {

		// input parameters
		String url = "" ;
		String proxyFile = "";
		String certsPath = "";
		try {
			// input parameters
			if ((args == null) || (args.length < 2)){
				throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> [CAs paths (optional)] )");
			} else if (args.length > 3) {
			 	 throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <proxyFile> [CAs paths (optional)] )");
			}
			url = args[0];
			proxyFile = args[1];
			if (args.length == 3) {
				certsPath = args[2];
			} else  {
				certsPath = "";
			}
			runTest ( url, proxyFile, certsPath);
		} catch (Exception exc){
			System.out.println (exc.toString( ));
		}
	 }

 }
