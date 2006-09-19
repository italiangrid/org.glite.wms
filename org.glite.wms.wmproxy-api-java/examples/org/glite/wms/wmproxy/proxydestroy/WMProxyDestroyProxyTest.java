
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.proxydestroy;

import org.glite.wms.wmproxy.common.GetParameters;
import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.BaseException;
import org.apache.axis.AxisFault;
import org.w3c.dom.Element;

public class WMProxyDestroyProxyTest {

/*
	Test of  "destroyProxy" method in org.glite.wms.wmproxy.WMProxyAPI

*/
	WMProxyDestroyProxyTest ( ) { }
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
		String delegId = "";
		WMProxyAPI client = null;
		System.out.println ("TEST : destroyProxy");
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
		delegId = GetParameters.askForDelegationId( );
		System.out.print ("Calling proxyDestroy service ");
		if (delegId.length( ) >0) {
			System.out.println ("with delegationID = " + delegId);
		} else{
			System.out.println ("with default delegationID");
		}
		client.destroyProxy (delegId );
		System.out.println ("The proxy has been successfully destroyed\n");
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
