/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.jobperusal ;

import org.glite.wms.wmproxy.WMProxyAPI ;
import org.glite.wms.wmproxy.StringList ;
// INPUT OBJECTS
import java.io.InputStreamReader ;
import java.io.BufferedReader ;
// file classes
import java.io.File;
import java.io.FileNotFoundException;
// UI-API-JAVA
import org.glite.wmsui.apij.JobId ;
/*
	Test of  "enablePerusal" method in org.glite.wms.wmproxy.WMProxyAPI

*/

public class WMProxyEnablePerusalTest {

	public WMProxyEnablePerusalTest ( ) { }

	/*
	*	starts the test
	*	@param url service URL
	*  	@param jobID the id to identify the job
	*	@param proxyFile the path location of the user proxy file
	*	@param fileList the list of files
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String jobId , String proxyFile, StringList fileList, String certsPath   ) throws java.lang.Exception {
		WMProxyAPI client = null;
		String protocol = "";
		String id = "";
		String[] files;
		String warns = "";
		String filenames = "";
		int size = 0;
		int num = 0;
		// Prints out the input parameters
		System.out.println ("TEST : enablePerusal");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JOB-ID	 		= [" + jobId + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy	 		= [" + proxyFile + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		files = (String[]) fileList.getItem( );
		size = files.length;
		if (size > 0) {
			filenames = "filename(s)		= [" ;
			for (int i = 0; i < size; i++) {
				if (i>0 && i< size) { filenames += ", ";}
				if ( files[i] != null) {
					filenames += files[i] ;
				} else {
					warns = "- the file number" + (i+1) + "\n";
				}
			}
			System.out.println (filenames + "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		} else {
			String err = "error: Filename(s) to be enable for peeking not found.\n";
			err += "Some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile>  <File1> ......<FileN> [CAs paths (optional)] )";
			throw new Exception (err);
		}
		if (warns.length()>0) {
			System.out.println ("\nWARNING: invalid input argument(s) for :\n" + warns);
		}
		// CAs dir
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		}
		System.out.println ("Calling the service: enableFilePerusal ...");
		// Calling the WMProxy service
		client.enableFilePerusal(jobId, fileList);
		// prints the results
		System.out.println ("\nEnd of the test\n\nResult:");
		if (size == 1) {
			System.out.println ( "Your file has been successfully enabled for the job:");
		} else {
			System.out.println ( "Your files have been successfully enabled for the job:");
		}

		System.out.println (jobId + "\n");
	}

	public static void main(String[] args) throws Exception {

		String url = "" ;
		String jobId = "" ;
		String proxyFile = "";
		String certsPath = "";
		String[ ] files = null;
		StringList fileList = null;
		int k = 0;
		int n = 0;
		int argc = args.length;
		// input parameters
		if ((args == null) || (argc < 4)) {
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile>  <File1> ......<FileN> [CAs paths (optional)] )");
		}
		url = args[0];
		// checks the jobid
		JobId id = new JobId(args[1]);
		jobId = id.toString( );
		//proxyFile
		proxyFile = args[2];
		// List of files
		n = argc - 3 ;
		if (n > 0) {
			files = new String[n];
			fileList = new StringList( );
			for (int i = 3; i < argc; i++) {
				File f = new File(args[i]);
				if (f.isDirectory()){
					// certsPath has to be the last argument (if present)
					if (i == (n-1) ){
						certsPath = f.getAbsolutePath ( );
					} else {
						throw new FileNotFoundException ("not valid file; this path is a directory: " + args[i]);
					}
				} // If the object is not a valid directory has to be a file to be enabled for job peeking
				else {
					files[k++] = new String(args[i]);
				}
			}
			fileList.setItem(files);
			// Test
			runTest ( url, jobId, proxyFile, fileList, certsPath);
		} else {
			String err = "error: Filename(s) to be enable for peeking not found.\n";
			err += "Some mandatory input parameters are missing (<WebServices URL> <jobId> <proxyFile>  <File1> ......<FileN> [CAs paths (optional)] )";
			throw new Exception (err);
		}
	 }

 }
