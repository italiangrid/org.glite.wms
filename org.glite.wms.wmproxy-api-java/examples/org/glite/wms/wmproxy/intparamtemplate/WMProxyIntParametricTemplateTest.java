
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.intparamtemplate ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList ;

import org.glite.wms.wmproxy.common.GetParameters ;

/*
	Test of  "getIntParametricJobTemplate" method in org.glite.wms.wmproxy.WMProxyAPI

	example of configuration file :

		attributes=(StdInput, StdOutput)
		parameters_number=10
		parameter_start=4
		parameter_step=2
		requirements=true
		rank=other.GlueCEStateEstimatedResponseTime


*/

public class WMProxyIntParametricTemplateTest {

	public WMProxyIntParametricTemplateTest  ( ) { }

	/*
		starts the test
		@param url service URL
		@param configFile the path location of the configuration file
		@param proxyFile the path location of the user proxy file
		@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String configFile, String proxyFile ) throws java.lang.Exception {

		// service input parameters
		StringList attributeList = null ;
		int paramNumber = 0;
		int parameterStart, parameterStep = 0;
		String rank = "";
		String requirements = "";

		// output result
		String result = "";
		// Prints out the input parameters
		System.out.println ("TEST : IntParametricTemplate");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("config-File	 	= [" + configFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");

		// Reads configuration file ----------------------------
		System.out.println ("==================================================================================================");
		GetParameters parameters = new GetParameters(  configFile ) ;

		// attribute list
		attributeList = (StringList) parameters.getStringList ( GetParameters.ATTRIBUTES );

		// number of parameters
		paramNumber = parameters.getIntValue( GetParameters.PARAM_NUMBER );
		System.out.println ( "paramNumber	= [" + paramNumber + "]");

		// parameter start
		parameterStart = parameters.getIntValue( GetParameters.PARAM_START);
		System.out.println ( "paramNumber	= [" + parameterStart + "]");

		// parameter step
		parameterStep = parameters.getIntValue( GetParameters.PARAM_STEP);
		System.out.println ( "paramStep	= [" + parameterStep + "]");

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
		System.out.println ("testing....");
		result = client.getIntParametricJobTemplate( attributeList, paramNumber, parameterStart, parameterStep, requirements, rank);

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


		// Reads the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <config-File> <proxyFile>)");
		url = args[0];
		configFile = args[1];
		proxyFile = args[2];

		runTest ( url, configFile, proxyFile);

	 }

 }
