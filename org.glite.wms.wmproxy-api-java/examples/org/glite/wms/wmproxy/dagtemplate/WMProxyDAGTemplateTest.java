
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.dagtemplate ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.GraphStructType;

import org.glite.wms.wmproxy.common.GetParameters ;

/*
*
*	Test of  "getDAGTemplate" method in org.glite.wms.wmproxy.WMProxyAPI
*
*/

public class WMProxyDAGTemplateTest {

	public WMProxyDAGTemplateTest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile ) throws java.lang.Exception {

		// service input parameters
		GraphStructType graphDependencies = null;
		String rank = "";
		String requirements = "";

		// output result
		String result = "";

		// Print out the input parameters
		System.out.println ("TEST : DAGTemplate");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// Read configuration file
		System.out.println ("==================================================================================================");

		// rank
		rank = "other.GlueCEStateEstimatedResponseTime";
		System.out.println ( "rank		= [" + rank + "]");

		// requirements
		requirements = "true";
		System.out.println ( "requirements	= [" + requirements + "]");
		System.out.println ("==================================================================================================");
		// end reading configuration file

		GraphStructType[ ]  nodes = new GraphStructType[2];
		nodes[0] = new GraphStructType() ;
		nodes[0].setName("B");
		nodes[1] = new GraphStructType() ;
		nodes[1].setName("C");

		graphDependencies = new GraphStructType( );
		graphDependencies.setChildrenJob( nodes );


		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("testing....");

		result = client.getDAGTemplate(graphDependencies, requirements, rank);

		// result
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("result=[" + result+ "]");
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("end of the test" );

	}

	public static void main(String[] args) throws java.lang.Exception {

		// input parameters
		String url = "" ;
		String jobId = "" ;
		String configFile = "" ;
		String proxyFile = "";



		// Read the input arguments
		if ((args == null) || (args.length < 2))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile>)");
		url = args[0];
		proxyFile = args[1];

		runTest(url,  proxyFile);

	 }

 }
