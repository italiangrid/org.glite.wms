
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 *
 * author : Marco Sottilaro (marco.sottilaro@datamat.it)
 * Please report any bug at:  egee@datamat.it
 *
 */

package org.glite.wms.wmproxy.jobregisterJSDL;

// JSDL imports:
import org.apache.axis.MessageContext;
import org.apache.axis.utils.XMLUtils;
import org.w3c.dom.Element;
import org.glite.wms.wmproxy.JobDefinitionHandler;
// import org.glite.wms.wmproxy.JSDLHandler;
import org.glite.wms.wmproxy.TestSer;
import java.io.BufferedReader;
import java.io.FileReader;


import org.glite.wms.wmproxy.WMProxyAPI;
import org.glite.wms.wmproxy.BaseException;
import org.glite.wms.wmproxy.JobIdStructType;
import org.glite.jdl.JobAd ;

/*
	Test of  "jobRegisterJSDL" method in org.glite.wms.wmproxy.WMProxyAPI

*/
public class WMProxyJobRegisterJSDLTest{
	/**
	* Default constructor
	*/
	public WMProxyJobRegisterJSDLTest ( ) { }
	/**
		 Prints the results
	*/
	private static void printResult (JobIdStructType entry) {
		JobIdStructType children[ ] = null;
		int size = 0;
		if ( entry != null ) {
			// id
			System.out.println ("jobID	= [" + entry.getId ( ) + "]" );
			// name
			System.out.println ("name	= [" +  entry.getName ( ) + "]" );

			// children
			children = (JobIdStructType[ ] ) entry.getChildrenJob ( );
			if ( children != null ) {
				size = children.length ;
				System.out.println ("number of children = [" + size + "]" );
				if ( size  > 0 ) {
					for (int i = 0; i < size ; i++){
						System.out.println ("child n. " + (i+1) );
						System.out.println ("--------------------------------------------");
						printResult (children [i] );
					}
				}
			} else
				System.out.println ("no children" );
		}
	}
	/*
	*	Starts the test
	*	@param url service URL
	*  	@param jsdlFile the path location of the JSDL file
	*  	@param delegationID the id to identify the delegation
	*	@param propFile the path location of the configuration file
	*	@param proxyFile the path location of the user proxy file
	*	@param certsPath the path location of the directory containing all the Certificate Authorities files
	*	@throws.Exception if any error occurs
	*/
	public static void runTest ( String url, String proxyFile, String delegationId, String jsdlFile, String certsPath   ) throws  org.glite.wms.wmproxy.BaseException {
		//jdl
		String jdlString = "";
		Element element  = null;
		JobDefinitionHandler  jsdl_h=null;
		String jsdlStr;
		try {
			// Read file
			BufferedReader in = new BufferedReader (new FileReader(jsdlFile));
			String tmp;
			jsdlStr= new String();
			while ((tmp=in.readLine())!=null){ jsdlStr+=tmp+"\n"; }
			in.close();
			jsdl_h= new JobDefinitionHandler( new TestSer().getMessageContext(),jsdlStr);
		} catch (Exception exc) {
			throw new BaseException("Unable to read/parse xml file "+ exc.getMessage());
		}
/* OLD  APPROACH

		try{
			// System.out.println("Context: " + context);
			element = XMLUtils.StringToElement("http://www.example.org","JobDefinition",jsdlStr);
			System.out.println("Element: " + element);
		} catch (Exception exc) {
			throw new BaseException("Unable to Create jsdl_h "+ exc.getMessage());
		}
*/
		// output results
		JobIdStructType result = null;
		WMProxyAPI client = null;
		// read jsdl

		// Prints out the input parameters
		System.out.println ("TEST : JobRegisterJSDL");
		System.out.println ("************************************************************************************************************************************");
		System.out.println ("WS URL	 		= [" + url + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("proxy			= [" + proxyFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");		System.out.println ("DELEGATION-ID		= [" +delegationId+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL-FILE		= [" + jsdlFile+ "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		System.out.println ("JDL			= [" + element + "]" );
		System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		if (certsPath.length()>0){
			System.out.println ("CAs path		= [" + certsPath+ "]" );
			System.out.println ("--------------------------------------------------------------------------------------------------------------------------------");
		  	client = new WMProxyAPI ( url, proxyFile, certsPath ) ;
		} else {
		 	 client = new WMProxyAPI ( url, proxyFile ) ;
		 }
		 // test
		System.out.println ("Testing ....");
		result= client.jobRegisterJSDL( jsdl_h, delegationId );
		// test results
		if ( result != null ) {
			System.out.println ("RESULT:");
			System.out.println ("=======================================================================");
			System.out.println ("Your job has been successfully registered:");
			printResult ( result );
			System.out.println("=======================================================================");
		}
		// end
		System.out.println ("End of the test");
	}

	public static void main(String[] args) throws Exception {
		String url = "" ;
		String jsdlFile = "" ;
		String proxyFile = "";
		String delegationID = "";
		String certsPath = "";
		// Reads the  input arguments
		if ((args == null) || (args.length < 4)) {
			throw new Exception ("error: some mandatory input parameters are missing (<WebServices URL> <delegationID> <proxyFile> <JSDL-FIlePath>  [CAs paths (optional)])");
		} else if (args.length > 5) {
			 throw new Exception ("error: too many parameters\nUsage: java <package>.<class> <WebServices URL> <delegationID> <proxyFile> <JSDL-FIlePath>  [CAs paths (optional)]");
		}
		url = args[0];
                delegationID = args[1];
		jsdlFile = args[2];
		proxyFile = args[3];
		if (args.length == 5) {
			certsPath = args[4];
		} else  {
			certsPath = "";
		}
		// Launches the test
		runTest ( url, jsdlFile, delegationID, proxyFile, certsPath);
	}
}
