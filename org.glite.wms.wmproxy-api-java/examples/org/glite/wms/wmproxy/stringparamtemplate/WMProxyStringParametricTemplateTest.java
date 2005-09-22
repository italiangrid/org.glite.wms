
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 */

package org.glite.wms.wmproxy.stringparamtemplate ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList ;

import org.glite.wms.wmproxy.common.GetParameters ;
/*
	Test of  "getStringParametricJobTemplate" method in org.glite.wms.wmproxy.WMProxyAPI

	example of configuration file :

		attributes=(Executable, StdOutput)
		parameters=(a, b)
		requirements=true
		rank=other.GlueCEStateEstimatedResponseTime
*/

public class WMProxyStringParametricTemplateTest {

	public WMProxyStringParametricTemplateTest  ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*  	@param configFile the path location of the configuration file
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String configFile ) throws java.lang.Exception {
		// service input parameters
		StringList attributeList, paramList = null ;
		String rank = "";
		String requirements = "";
		// output result
		String result = "";
		// Prints out the input parameters
		System.out.println ("TEST : StringParametricTemplate");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("config-File	 	= [" + configFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// Reads the configuration file ----------------------------
		System.out.println ("==================================================================================================");
		GetParameters parameters = new GetParameters(  configFile ) ;
		// attributes list
		attributeList = (StringList) parameters.getStringList ( GetParameters.ATTRIBUTES );
		String[]  list = (String[] ) attributeList.getItem( );
		if (list.length >0){
			System.out.print ( "attributes	= [");
			for (int i=0 ; i < list.length   ; i++) {
				if (i > 0){ System.out.print (", ");}
				System.out.print (list[i]);
			}
			System.out.println("]");
		}
		// parameters list
		paramList = (StringList) parameters.getStringList ( GetParameters.PARAMETERS );
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

		result = client.getStringParametricJobTemplate( attributeList, paramList, requirements, rank);
	System.out.println ("Result:");
		// result
		System.out.println ("=========================================================================================================================================================");
		System.out.println (result);
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("End of the test" );
	 }
	public static void main(String[] args) throws Exception {
		// input parameters
		String url = "" ;
		String jobId = "" ;
		String configFile = "" ;
		String proxyFile = "";
		// Read the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> <config-File>)");
		url = args[0];
		proxyFile = args[1];
		configFile = args[2];
		runTest ( url,proxyFile, configFile);

	 }

 }
