
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.proxyinfo ;

// WMP-API-JAVA
import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.ProxyInfoStructType;
// UI-API-JAVA
import org.glite.wmsui.apij.JobId ;

/*
*
*	Test of  "getJobProxyInfo" method in org.glite.wms.wmproxy.WMProxyAPI
*
*/

public class WMProxyJobProxyInfoTest {

	public WMProxyJobProxyInfoTest  ( ) { }

	/*
	*	starts the test
	*	@param url the service URL
	*	@param jobId the job identifier
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jobId, String proxyFile, String certsPath  ) throws  java.lang.Exception {
		String jdl = "";
		WMProxyAPI client = null;
		ProxyInfoStructType result = null;
		ProxyInfo proxyInfo = null;
		// Prints out the input parameters
		System.out.println ("TEST : JobProxyInfo");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOBID	 		= [" +jobId + "]" );
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
		result = client.getJobProxyInfo(jobId);
		// result
		System.out.println ("RESULT - PROXY-INFO:");
		System.out.println ("======================================================================================================");
		if (result != null) {
			proxyInfo = new ProxyInfo( );
			System.out.println (proxyInfo.getProxyInfo(result));
		 } else {
			System.out.println ("Sorry!! No information available for this proxy!");
		 }
		System.out.println ("======================================================================================================");
		System.out.println ("End of the test" );
	 }

	public static void main(String[] args) throws java.lang.Exception {
		// test input parameters
		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		String certsPath = "";
		try {
			// input parameters
			if ((args == null) || (args.length < 3)){
				throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile> [CAs paths (optional)] )");
			} else if (args.length > 4) {
			 	 throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <jobId> <proxyFile> [CAs paths (optional)] )");
			}
			url = args[0];
			jobId = args[1];
			proxyFile = args[2];
			if (args.length == 4) {
				certsPath = args[3];
			} else  {
				certsPath = "";
			}
			// checks the jobid
			JobId id = new JobId(jobId);
			jobId = id.toString( );
			runTest ( url, jobId, proxyFile, certsPath);
		} catch (Exception exc){
			System.out.println (exc.toString( ));
		}
	 }

 }
