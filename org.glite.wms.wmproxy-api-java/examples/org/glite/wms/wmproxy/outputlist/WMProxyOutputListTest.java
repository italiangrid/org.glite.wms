
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.outputlist ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringAndLongList ;
import org.glite.wms.wmproxy.StringAndLongType ;


/*
	Test of  "getOutputFileList" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyOutputListTest {

	public WMProxyOutputListTest ( ) { }

	private static void printResult (StringAndLongList list) {

	StringAndLongType vect [ ] = null ;
		StringAndLongType file = null;
		int size = 0;

		if ( list!= null ) {

			vect = (StringAndLongType[ ] ) list.getFile ( );

			if ( vect != null ){
				size = vect.length ;
				System.out.println ("number of files = [" + size + "]" );
				if ( size  > 0 ) {
					for (int i = 0; i < size ; i++){
						file = vect [i];
						System.out.println ("file n. " + (i+1) );
						System.out.println ("--------------------------------------------");
						System.out.println ("name		= [" + file.getName ( ) + "]" );
						System.out.println ("size		= [" + file.getSize ( ) + "]" );

					}
				}
			}
		}


/*

		//vect = ( StringAndLongType[ ] ) list.getFile ( );
		System.out.println ("===\n"+ list.toString( ) +"===\n" );
					for (int i = 0; i < 2 ; i++){
						file = list.getFile(i);
						System.out.println ("file n. " + (i+1) );
						System.out.println ("--------------------------------------------");
						System.out.println ("name		= [" + file.getName ( ) + "]" );
						System.out.println ("size		= [" + file.getSize ( ) + "]" );

					}

*/


	}

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jobID the id to identify the job
	*	@param proxyFile  the path location of the user proxy file
	*	@throws.Exception if any error occurs
	*/
	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		StringAndLongList result = null;

		// Read the input arguments
		if ((args == null) || (args.length < 3))
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <JobId> <proxyFile>)");
		url = args[0];
		jobId = args[1];
		proxyFile = args[2];

		// Print out the input parameters
		System.out.println ("TEST : OutputList");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID		= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		// testing ...
		WMProxyAPI client = new WMProxyAPI ( url, proxyFile ) ;
		System.out.println ("testing ....");
		result = client.getOutputFileList(jobId);
		// test results
		if ( result != null ) {
			System.out.println ("RESULT:");
			System.out.println ("=======================================================================");
			printResult ( result );
			System.out.println("=======================================================================");
		}

		System.out.println ("end of the test" );

	 }

 }
