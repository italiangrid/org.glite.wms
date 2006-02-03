
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.protocols ;

// WMP-API-JAVA
import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList;

/*
*
*	Test of  "getProtocols" method in org.glite.wms.wmproxy.WMProxyAPI
*
*/

public class WMProxyGetProtocolsTest {

	public WMProxyGetProtocolsTest  ( ) { }

	/*
	*	Starts the test
	*	@param url the service URL
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String certsPath  ) throws  java.lang.Exception {
		String jdl = "";
		WMProxyAPI client = null;
		StringList protoList = null ;
		String[]  list = null;
		int len = 0;
		// Prints out the input parameters
		System.out.println ("TEST : GetProtocols");
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
		System.out.println ("Testing....");
		protoList = client.getTransferProtocols( );
		if (protoList != null) {
			// result
			System.out.println ("RESULT - PROTOCOLS:");
			System.out.println ("===================================================");
			list = (String[] ) protoList.getItem( );
			len = list.length;
			if ( len == 0) {
				System.out.print ("Sorry, but he received list is empty !!");
			} else {
				for (int i=0 ; i < len  ; i++) {
					System.out.println  ( "" + (i+1) + " - " + list[i]);
				}
			}
			System.out.println ("===================================================");
		} else {
			System.out.println ("No information available: a <Null Pointer> has been received by the server");
		}
		System.out.println ("End of the test" );
	 }
	public static void main(String[] args) throws java.lang.Exception {
		// test input parameters
		String url = "" ;
		String delId = "" ;
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
