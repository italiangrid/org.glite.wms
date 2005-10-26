
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

public class WMProxyGrstGetProxyTest {

/*
	Test of  "grstGetProxy" method in org.glite.wms.wmproxy.WMProxyAPI

*/
	WMProxyGrstGetProxyTest ( ) { }
	/*
	*	Starts the test
	*	@param url service URL
	*  	@param delegationID the id to identify the delegation
	*  	@param propFile the path location of the user properties file
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String delegationId, String propFile, String proxyFile ) throws org.glite.wms.wmproxy.BaseException  {
		// proxies
		String certReq = "";
		String proxy = "" ;
		String result = "" ;

		System.out.println ("TEST : GridSite getProxy");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL			= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// Test
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing ....");
		// Proxy Request
		System.out.println ("Performing Proxy-Request .....");
		certReq = client.grstGetProxyReq ( delegationId );
		System.out.println ("getProxy result [\n." + certReq + "]" );
		// end
		System.out.println ("End of the test\n");
	}
	/**
	*	main
	*/
	public static void main(String[] args)  {
		String url = "" ;
		String delegationId = "";
		String propFile = "";
		String proxyFile = "";
		try  {
			// input parameters
			if ((args == null) || (args.length < 3))
				throw new IllegalArgumentException ("error: some mandatory input parameters are missing (<WebServices URL> <Delegation-ID> <proxyFile>)");
			url = args[0];
			delegationId = args[1];
			proxyFile = args[2];

			// Launches the test
			runTest (url, delegationId, propFile, proxyFile);
		} catch (BaseException exc) {
			System.out.println("\nException caught:\n");
			System.out.println("Message:\n----------\n"+ exc.getMessage() + "\n");
			System.err.println("Stack Trace:\n------------");
                        exc.printStackTrace();
		}
	/*	} catch (Exception e) {
			System.out.println("Exception caught!!");
                        System.err.println("Stack Trace:");
                        e.printStackTrace();
                        System.err.println("\ngetMessage: " + e.getMessage());


                        // lettura del fault
                        AxisFault fault = AxisFault.makeFault(e);
                        System.out.println("\nfaultString: \n" + fault.getFaultString());
                        Element element[] = fault.getFaultDetails();
                        System.out.println("Description: \n" + element[0]);
                        System.out.println("Description: \n"
                                        + element[0].getElementsByTagName("Description").item(0));
		}*/
/*
 		} catch ( org.gridsite.www.namespaces.delegation_1.DelegationExceptionType exc)  {
			System.err.println ( "DelegationException caught ");
			System.err.println (  "Message: " + exc.getMessage1() );
			// Print the stack trace to a byte array
			java.io.ByteArrayOutputStream out = new java.io.ByteArrayOutputStream();
			java.io.PrintStream ps = new java.io.PrintStream(out);
			exc.printStackTrace(ps);
		} catch (java.rmi.RemoteException exc)  {
System.err.println ( "----------------\n" + exc.toString() +  "----------------\n");
			System.err.println ( "RemoteException caught ");
			System.err.println (  "Message: " + exc.getMessage() );
			System.err.println (   "Cause: " + exc.getCause() );
			// Print the stack trace to a byte array
			java.io.ByteArrayOutputStream out = new java.io.ByteArrayOutputStream();
			java.io.PrintStream ps = new java.io.PrintStream(out);
			exc.printStackTrace(ps);

			 System.err.println (  "Stack Trace:\n" + ps.toString() );
 		} catch (Exception exc )  {
System.err.println ( "----------------\n" + exc.toString() +  "----------------\n");
			System.err.println ( "Generic Exception caught ");
			System.err.println (  "Message: " + exc.getMessage() );

		}*/

	 }

 }
