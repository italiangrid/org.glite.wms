
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */

package org.glite.wms.wmproxy.jobtemplate ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.JobType;
import org.glite.wms.wmproxy.JobTypeList ;
import org.glite.wms.wmproxy.common.GetParameters ;

/*
	Test of  "jobTemplate" method in org.glite.wms.wmproxy.WMProxyAPI

	example of configuration file :

		jobtype=(INTERACTIVE,MPI)
		executable=/bin/ls
		arguments=-la
		requirements=true
		rank=other.GlueCEStateEstimatedResponseTime

*/

public class WMProxyJobTemplateTest {

	public WMProxyJobTemplateTest  ( ) { }

	/**
		Starts the test
		@param url service URL
		@param proxyFile the path location of the user proxy file
		@param configFile the path location of the configuration file
		@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String configFile ) throws java.lang.Exception {
		// service input parameters
		JobTypeList typeList = null ;
		String executable = "";
		String arguments = "";
		String rank = "";
		String requirements = "";
		// output result
		String result = "";
		// Reads the  configuration file
		System.out.println ("==================================================================================================");
		GetParameters parameters = new GetParameters(  configFile ) ;
		// jobtype list
		typeList =parameters.getJobType( );
		// executable
		executable = parameters.getStringValue( GetParameters.EXECUTABLE );
		System.out.println ( "executable	= [" + executable + "]");
		// arguments
		arguments = parameters.getStringValue( GetParameters.ARGUMENTS );
		System.out.println ( "arguments	= [" +  arguments + "]");
		// rank
		rank =parameters.getStringValue ( GetParameters.RANK );
		System.out.println ( "rank		= [" + rank + "]");
		// requirements
		requirements = parameters.getStringValue ( GetParameters.REQUIREMENTS );
		System.out.println ( "requirements	= [" + requirements + "]");

		System.out.println ("==================================================================================================");
		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("Testing...");
		result = client.getJobTemplate( typeList, executable, arguments,requirements, rank);
		// Result
		System.out.println ("Result:");
		System.out.println ("=========================================================================================================================================================");
		System.out.println ( result);
		System.out.println ("=========================================================================================================================================================");
		System.out.println ("End of the test" );
	}
	/**
	* main
	*/
	public static void main(String[] args) throws Exception {
		// input parameters
		String url = "" ;
		String jobId = "" ;
		String configFile = "" ;
		String proxyFile = "";
		// Reads the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <proxyFile> <config-File> )");
		url = args[0];
		proxyFile = args[1];
		configFile = args[2];
		// Prints out the input parameters
		System.out.println ("TEST : JobTemplate");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("config-File	 	= [" + configFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");			runTest ( url, proxyFile, configFile );
	 }

 }
