/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
  * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */

 package org.glite.wms.wmproxy.collectiontemplate ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.common.GetParameters ;
import org.apache.log4j.Logger;

/*
	Test of "getCollectionTemplate" method in org.glite.wms.wmproxy.WMProxyAPI

	example of configuration file :
	jobnumber=3
	requirements=true
	rank=other.GlueCEStateEstimatedResponseTime


*/

public class WMProxyCollectionTemplateTest {

	public WMProxyCollectionTemplateTest  ( ) { }

	/*
		starts the test
		@param url service URL
		@param configFile the path location of the configuration file
		@param proxyFile the path location of the user proxy file
		@throws.Exception if any error occurs
	*/
	public static void runTest (String url, String configFile, String proxyFile ) throws java.lang.Exception {

		// input parameters for the service
		int jobNumber = 0;
		String rank = "";
		String requirements = "";

		// output result
		String result = "";

		// Prints out the input parameters
		System.out.println ("TEST : CollectionTemplate");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("config-File	 	= [" + configFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// Read configuration file ----------------------------
		System.out.println ("==================================================================================================");
		GetParameters parameters = new GetParameters(  configFile ) ;
		// job	number
		jobNumber = parameters.getIntValue( GetParameters.JOBNUMBER );
		System.out.println ( "jobnumber	= [" + jobNumber + "]");
		// rank
		rank =parameters.getStringValue ( GetParameters.RANK );
		System.out.println ( "rank		= [" + rank + "]");
		// requirements
		requirements = parameters.getStringValue ( GetParameters.REQUIREMENTS );
		System.out.println ( "requirements	= [" + requirements + "]");

		System.out.println ("==================================================================================================");
		// end reading configuration file ----------------------------
		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing....");
		result = client.getCollectionTemplate( jobNumber, requirements, rank);
		// result
		System.out.println ("RESULT");
		System.out.println ("=========================================================================================================================================================");
		System.out.println (result);
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("End of the test" );
	}
	/*
		main
	*/
	public static void main(String[] args) throws java.lang.Exception {
		// test input parameters
		String url = "" ;
		String jobId = "" ;
		String configFile = "" ;
		String proxyFile = "";
		// Reads the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> <config-File>)");
		url = args[0];
		proxyFile = args[1];
		configFile = args[2];
		runTest ( url, configFile, proxyFile);
 	 }
 }
