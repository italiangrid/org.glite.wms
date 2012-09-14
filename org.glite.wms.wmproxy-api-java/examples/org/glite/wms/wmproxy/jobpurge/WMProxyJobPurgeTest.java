/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 *
 * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */

package org.glite.wms.wmproxy.jobpurge ;

import org.glite.wms.wmproxy.WMProxyAPI ;
// UI-API-JAVA
import org.glite.wmsui.apij.JobId ;
/*
	Test of  "jobPurge" method in org.glite.wms.wmproxy.WMProxyAPI

*/
public class WMProxyJobPurgeTest {
	/**
	* Default constructor
	*/
	public WMProxyJobPurgeTest ( ) { }
	/*
	*	Starts the test
	*	@param url service URL
	*	@param proxyFile the path location of the user proxy file
	*  	@param jobID the id to identify the job
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url,  String proxyFile, String jobId, String certsPath) throws java.lang.Exception {
		WMProxyAPI client = null;
		// Prints  the input parameters
		System.out.println ("TEST : JobPurge");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxyFile		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID			= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		}
		// Testing .....
		System.out.println ("Calling the jobPurge service.....");
		client.jobPurge( jobId );
		System.out.println ("Your request of purging has been successfully sent");
		System.out.println ("End of the test" );
	}
	public static void main(String[] args) throws Exception  {
		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		String certsPath = "";
		// input parameters
		if ((args == null) || (args.length < 3)){
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile> [CAs paths (optional)] )");
		} else if (args.length > 4) {
			throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL>  <jobId> <proxyFile> [CAs paths (optional)] )");
		}
		url = args[0];
		// checks the jobid
		JobId id = new JobId(args[1]);
		jobId = id.toString( );
		// proxyFile
		proxyFile = args[2];
		if (args.length == 4) {
			certsPath = args[3];
		} else  {
			certsPath = "";
		}
		// test
		runTest ( url, proxyFile, jobId, certsPath);
	 }
 }
