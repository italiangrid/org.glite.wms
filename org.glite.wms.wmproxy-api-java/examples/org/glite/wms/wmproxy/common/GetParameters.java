package org.glite.wms.wmproxy.common ;

//import java.util.Vector ;

import org.glite.wms.wmproxy.JobType;
import org.glite.wms.wmproxy.JobTypeList;
import org.glite.wms.wmproxy.StringList;

import java.io.BufferedReader;
import java.io.FileReader ;


import java.util.Hashtable;
import java.lang.Integer ;

import java.io.IOException;
import java.lang.NoSuchFieldException ;
import java.lang.IllegalArgumentException;
import java.util.StringTokenizer ;



public class GetParameters {

	/*
		Default constructor
	*/
	public GetParameters (String file ) throws  java.io.IOException{

		this.configBuffer = readConfigFile (file );

		this.typeTable = new Hashtable ( );
		this.typeTable.put (this._NORMAL, JobType.NORMAL);
		this.typeTable.put (this._PARAMETRIC, JobType.PARAMETRIC);
		this.typeTable.put (this._INTERACTIVE, JobType.INTERACTIVE);
		this.typeTable.put (this._MPI,  JobType.MPI);
		this.typeTable.put (this._PARTITIONABLE, JobType.PARTITIONABLE);
		this.typeTable.put (this._CHECKPOINTABLE,  JobType.CHECKPOINTABLE);

	}

	// mapping of the JobType attributes
	public static final String _NORMAL 			=  JobType._NORMAL ;
	public static final String _PARAMETRIC 		= JobType._PARAMETRIC ;
	public static final String _INTERACTIVE 		= JobType._INTERACTIVE ;
	public static final String _MPI 				= JobType._MPI ;
	public static final String _PARTITIONABLE 	= JobType._PARTITIONABLE ;
	public static final String _CHECKPOINTABLE 	= JobType._CHECKPOINTABLE ;


	// configuration parameters
	public static final String JOBTYPE 		= "jobtype" ;
	public static final String  EXECUTABLE	= "executable" ;
	public static final String  ARGUMENTS 	= "arguments"  ;
	public static final String  REQUIREMENTS	= "requirements" ;
	public static final String  RANK			= "rank" ;
	public static final String JOBNUMBER		= "jobnumber" ;
	public static final String ATTRIBUTES		= "attributes" ;
	public static final String PARAMETERS	= "parameters";
	public static final String PARAM_NUMBER = "parameters_number";
	public static final String PARAM_START 	= "parameter_start";
	public static final String PARAM_STEP	= "parameter_step";


	// special tokens
	public static final String  SEPARATOR_TOKEN 		= "=" ;
	public static final String  START_LIST_TOKEN 		= "(" ;
	public static final String  END_LIST_TOKEN 		= ")" ;
	public static final String  SEPARATOR_LIST_TOKEN = "," ;


	// buffer for the configuration file
	private String configBuffer ;

	// structures
	private Hashtable typeTable = null ;
	private JobType[ ] typeList = null;
	

	private static String  readConfigFile ( String file ) throws java.io.IOException {

		String line = null;
		String config = null;

		try {
			BufferedReader buffer = new BufferedReader( new FileReader( file ) );

			while ((line = buffer.readLine()) != null) {
				line = line.trim( );
				if ( line.startsWith ( "#") == false)
					config += line  +"\n" ;
			}
			buffer.close();
		}catch (Exception ex) {
			throw new IOException( "Error in reading file (" +file + ") \n" + ex );
		}

		return config;
	}


	private String getParameter ( String parameter ) throws java.lang.NoSuchFieldException {

 		int start, end = 0;
		String line = "";

		start = this.configBuffer.indexOf ( parameter ) ;

		if ( start > 0 ){

			end = this.configBuffer.indexOf ("\n", start ) ;

			if ( end > start )
				line = this.configBuffer.substring ( start, end );
			else
				throw new NoSuchFieldException ("jobtype parameter: mismatch error in the configuration input file");
		}

		return line ;
	}

	public JobTypeList getJobType ( ) throws java.lang.NoSuchFieldException, java.lang.IllegalArgumentException {

		String line, label, value, list = "";
		JobType type = null;
		JobType[ ] typeArray = null;
		JobTypeList typeList = null;
		StringTokenizer tokens = null;
		int start, end, ntok = 0;

		String format = this.JOBTYPE + this.SEPARATOR_TOKEN + START_LIST_TOKEN + "element 1 " + SEPARATOR_LIST_TOKEN +
					"element 2" + SEPARATOR_LIST_TOKEN + "...." + END_LIST_TOKEN ;

		// Acquires the JobType line from the configuration file
		line = this.getParameter ( this.JOBTYPE ) ;
		System.out.println ( "jobtype 	= [" + line + "]");

		// Splits up label and the list of values basing on the Separator Token
		tokens = new StringTokenizer(line, this.SEPARATOR_TOKEN);
		if ( tokens.countTokens() != 2)
			throw new IllegalArgumentException( "bad format for job type (" + line + ")\n right format is" + format  );

		label = (String) tokens.nextElement ( );
		list =(String)  tokens.nextElement ( );

		if ( ( label == null ) || (list == null ) )
			throw new IllegalArgumentException( "bad format for job type - missing label or/and value (" + line + ")\n right format is " + format );

		// checks the correctness of the label
		label = label.trim( );
		if ( label.compareTo (this.JOBTYPE) != 0 )
			throw new IllegalArgumentException( "bad format for job type- wrong label (" + line + ")\n right format is" + format );

		// list of values: cancels the start and end-token
		list = list.trim( );
		start = list.indexOf ( START_LIST_TOKEN );
		end = list.indexOf ( END_LIST_TOKEN );

		if ( ( start < 0 ) || ( end < start ) )
			throw new IllegalArgumentException( "bad format for job type- wrong value (" + line + ")\n right format is" + format );
		list = list.substring ( start+1, end);

		// list of values: extraction of JobType values
		tokens = new StringTokenizer(list, this.SEPARATOR_LIST_TOKEN );
		ntok =tokens.countTokens() ;
		if ( ntok < 0 )
			throw new IllegalArgumentException( "bad format for job type - bad list value (" + line + ")\n right format is" + format  );

		// Builds the temporary array (used to create the JobType list to get back)
		typeArray = new JobType [ntok] ;
		for ( int i = 0; i < ntok ; i++) {
			value = (String) tokens.nextElement ( );
			//System.out.println ( "JobType single value=" + value);

			type =(JobType) this.typeTable.get ( value.trim( ) );

			if ( type == null )
				throw new IllegalArgumentException( "bad format for job type - wrong value in list  (" + line + ")" );

			typeArray[i] = type;

		}

		// builds the JobType list
		typeList = new JobTypeList ( );
		typeList.setJobType (typeArray);

		return  typeList ;
	}



	public StringList getStringList (String parameter) throws java.lang.NoSuchFieldException, java.lang.IllegalArgumentException {

		String line, label, value, list  = "";
		String elem = null;
		String [ ] strArray = null;
		StringList strList = null;
		StringTokenizer tokens = null;
		int start, end, ntok = 0;

		String format = parameter + this.SEPARATOR_TOKEN + START_LIST_TOKEN + "element 1 " + SEPARATOR_LIST_TOKEN +
					"element 2" + SEPARATOR_LIST_TOKEN + "...." + END_LIST_TOKEN ;

		// Acquires the list- line from the configuration file
		line = this.getParameter (parameter) ;
		//System.out.println ( "line 	= [" + line + "]");

		// Splits up label and the list of values basing on the Separator Token
		tokens = new StringTokenizer(line, this.SEPARATOR_TOKEN);

		if ( tokens.countTokens() != 2)
			throw new IllegalArgumentException( "bad format for "+ parameter +" (" + line + ")\n right format is " + format  );

		label = (String) tokens.nextElement ( );
		list =(String)  tokens.nextElement ( );

		if ( ( label == null ) || (list == null ) )
			throw new IllegalArgumentException( "bad format for  "+ parameter +" - missing label or/and value (" + line + ")\n right format is " + format );

		label = label.trim( );

		// checks the correctness of the label
		if ( label.compareTo (parameter) != 0 )
			throw new IllegalArgumentException( "bad format for "+ parameter +" - wrong label (" + line + ")\n right format is " + format );

		// list of values: cancels the start and end-token
		list = list.trim( );
		start = list.indexOf ( START_LIST_TOKEN );
		end = list.indexOf ( END_LIST_TOKEN );

		if ( ( start < 0 ) || ( end < start ) )
			throw new IllegalArgumentException( "bad format for "+ parameter +"  - wrong value (" + line + ")\n right format is" + format );

		list = list.substring ( start+1, end);

		// list of values: extraction of JobType values
		tokens = new StringTokenizer(list, this.SEPARATOR_LIST_TOKEN );

		ntok =tokens.countTokens() ;
		if ( ntok < 0 )
			throw new IllegalArgumentException( "bad format for  "+ parameter +" - bad list value (" + line + ")\n right format is" + format  );

		// Builds the temporary array (used to create the String-list to get back)
		strArray = new String[ntok];

		for ( int i = 0; i < ntok ; i++) {
			value = (String) tokens.nextElement ( );
			//System.out.println ( "value=[" + value + "]");
			strArray[i] =value;
		}

		// Builds the String-list
		strList = new StringList ( );
		strList.setItem ( strArray );

		return  strList ;
	}

	public int getIntValue ( String parameter) throws java.lang.NoSuchFieldException, java.lang.IllegalArgumentException {

		String line = "";
		String value = "";
		Integer integerValue = null;
		StringTokenizer tokens = null ;

		// Acquires the parameter line from the configuration file
		line =this.getParameter ( parameter ) ;
		//System.out.println ( "line = [" + line + "]");

		if (line == null )
			throw new NoSuchFieldException ( "parameter not found (" + parameter + ")" );

		tokens = new StringTokenizer (line, this.SEPARATOR_TOKEN);

		// Splits up label and the list of values basing on the Separator Token
		if  ( tokens.countTokens( ) != 2)
			throw new IllegalArgumentException( "bad format for " + parameter );

		tokens.nextElement();

		// int-value
		value = (String) tokens.nextElement( );
		integerValue = new Integer (value);

		return integerValue.intValue( ) ;
	}

	public String getStringValue ( String parameter) throws java.lang.NoSuchFieldException, java.lang.IllegalArgumentException {

		String line = "";
		String value = "";
		StringTokenizer tokens = null ;

		// Acquires the parameter line from the configuration file
		line =this.getParameter ( parameter ) ;
		//System.out.println ( "line = [" + line + "]");

		if (line == null )
			throw new NoSuchFieldException ( "parameter not found (" + parameter + ")" );

		// Splits up label and the list of values basing on the Separator Token
		tokens = new StringTokenizer (line, this.SEPARATOR_TOKEN);

		if (tokens.countTokens( ) != 2)
			throw new IllegalArgumentException( "bad format for " + parameter );

		// String value
		tokens.nextElement();
		value = (String) tokens.nextElement( );

		return value ;
	}

}
