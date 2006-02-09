
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
  * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
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
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String certsPath  ) throws java.lang.Exception {

		// service input parameters
		GraphStructType graphDependencies = null;
		String rank = "";
		String requirements = "";
		WMProxyAPI client = null;
		// output result
		String result = "";

		// Print out the input parameters
		System.out.println ("TEST : DAGTemplate");
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
		System.out.println ("JDL input attributes:");
		// rank
		rank = "other.GlueCEStateEstimatedResponseTime";
		System.out.println ( "rank		= [" + rank + "]");
		// requirements
		requirements = "true";
		System.out.println ( "requirements	= [" + requirements + "]");
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// GRAPH
		GraphStructType[ ]  nodes = new GraphStructType[2];
		nodes[0] = new GraphStructType() ;
		nodes[0].setName("B");
		nodes[1] = new GraphStructType() ;
		nodes[1].setName("C");
		graphDependencies = new GraphStructType( );
		graphDependencies.setChildrenJob( nodes );
		// testing ...
		System.out.println ("Testing....\n");
		result = client.getDAGTemplate(graphDependencies, requirements, rank);
		System.out.println ("\nEnd of the test\nResult:");
		// result
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("result=[" + result+ "]");

		System.out.println ("=========================================================================================================================================================");
	}

	public static void main(String[] args)  throws Exception  {
		// test input parameters
		String url = "" ;
		String proxyFile = "";
		String certsPath = "";
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
 	 }
 }
