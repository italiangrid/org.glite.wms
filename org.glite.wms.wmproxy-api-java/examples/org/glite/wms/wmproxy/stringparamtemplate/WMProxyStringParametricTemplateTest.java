
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
	/**
		Starts the test
		@param url service URL
		@param configFile the path location of the configuration file
		@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
		@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String configFile, String proxyFile, String certsPath ) throws java.lang.Exception {
		WMProxyAPI client = null;
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
		System.out.println ("config-File	 	= [" + configFile + "]" );
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
		client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Calling the getStringParametricJobTemplate service");
		result = client.getStringParametricJobTemplate( attributeList, paramList, requirements, rank);
	System.out.println ("Result:");
		// result
		System.out.println ("=========================================================================================================================================================");
		System.out.println (result);
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("End of the test" );
	 }
	public static void main(String[] args) throws Exception {

		// test input parameters
		String url = "" ;
		String configFile = "" ;
		String proxyFile = "";
		String certsPath = "";
		// input parameters
		if ((args == null) || (args.length < 3)){
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <configFile> <proxyFile> [CAs paths (optional)] )");
		} else if (args.length > 4) {
			throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <configFile> <proxyFile> [CAs paths (optional)] )");
		}
		url = args[0];
		configFile = args[1];
		proxyFile = args[2];
		if (args.length == 4) {
			certsPath = args[3];
		} else  {
			certsPath = "";
		}
		runTest ( url, configFile, proxyFile, certsPath);
	 }

 }
