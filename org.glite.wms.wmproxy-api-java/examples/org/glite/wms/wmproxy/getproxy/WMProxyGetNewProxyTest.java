
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.getproxy;

import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.BaseException;
import org.apache.axis.AxisFault;
import org.w3c.dom.Element;
import org.gridsite.www.namespaces.delegation_1.NewProxyReq;
public class WMProxyGetNewProxyTest {

/*
	Test of  "grstGetProxy" method in org.glite.wms.wmproxy.WMProxyAPI

*/
	WMProxyGetNewProxyTest ( ) { }
	/*
	*	Starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String certsPath ) throws org.glite.wms.wmproxy.BaseException  {
		// proxies
		String proxy = "" ;
		WMProxyAPI client = null;
		NewProxyReq result = null;
		System.out.println ("TEST : newGetProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL			= [" + url + "]" );
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
		result = client.getNewProxyReq ( );
		System.out.println ("RESULT:\n----------------------------------------------------------------\n");
		System.out.println ("Delagation ID =[." + result.getDelegationID()+ "]" );
		System.out.println ("Proxy\n========================================\n" +
			result.getProxyRequest( ) + "\n========================================" );
		// end
		System.out.println ("End of the test\n");
	}
	/**
	*	main
	*/
	public static void main(String[] args)  {
		String url = "" ;
		String proxyFile = "";
		String certsPath = "";		
		try {
			// input parameters
			if ((args == null) || (args.length < 2)){
				throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile>)");
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
			// Launches the test
			runTest (url, proxyFile, certsPath);
		} catch (Exception exc){
			System.out.println (exc.toString( ));
		}
	}
 }
