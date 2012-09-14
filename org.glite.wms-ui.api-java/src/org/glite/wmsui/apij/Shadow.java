/*
* Shadow.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*/
package org.glite.wmsui.apij;
import  org.glite.jdl.JobAd ;
import java.net.InetAddress ;
import java.util.StringTokenizer ;
import java.net.UnknownHostException ; //InetAddress Exc
import  java.io.BufferedReader ; //Stream of info gathering from pipes
import  java.io.FileReader;
import  java.io.FileWriter;
import  java.io.BufferedWriter; //Stream of info gathering from pipes
import java.io.File ;
import condor.classad.* ;
import java.io.IOException ;



/**
 * This class provides the core management for interactive jobs.
 * once the glite-wms-grid-console-shadow has started successfully and the job is running
 * the user should interact with the submitted job  (or might have attached to a previous job)
* At the end of the interaction the background bypass process should be
 * killed and the I/O pipes have to be removed. This is done automatically by the 'detach' method.
 * The shadow class must be used togheter with an implementation of the Listener interface, which actually performs the
 * final visual interactivity with the user.
 * @see Listener
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it> */
public class Shadow implements Runnable{
	/**The constructor*/
	Shadow(JobId jid , Listener ls ){  listener = ls ;  jobId  = jid ;  };
	/**Class Destructor*/
	public void finalize () {
		// System.out.println ( "\n\n\nShadow::Finalising!!\n\n\n"  );
		try{     detach () ;  }
		catch (  Exception exc ){  
			/** Do nothing*/
		}
	};
	/**
	* Run the listener
	*/
	public void run() {
		if (listener!=null)  listener.run( this) ;
		else    finalize() ;
	};
	/**
	* Write into the input stream pipeline
	@param str the string to be written
	@throws throws IOException If any error occurred while writing in the pipe
	@throws NoSuchFieldException if the inputPipe has not been created yet
	*/
	public void writeIn (String str )throws IOException, NoSuchFieldException {
			if ( barbaWriter ==null ) barbaWriter = new BufferedWriter ( new FileWriter ( getPipeIn()  )  ) ;
			BufferedWriter inputBuffer = new BufferedWriter ( new FileWriter ( getPipeIn()  )  ) ;
			// System.out.println ("Shadow::writeln-->writing: " + str ) ;
			inputBuffer.write( str );
			inputBuffer.close ();
	}
	/**
	* Read any possible message stored inside the output pipeline
	* @return the message read in the pipe (if any)
	* @throws throws IOException If any error occurred while reading in the pipe
	* @throws NoSuchFieldException if the outputPipe has not been created yet */
	public String emptyOut ( )throws IOException, NoSuchFieldException {
		if ( outBuf ==null)  outBuf = new BufferedReader ( new FileReader(  getPipeOut() )  ) ;
		int intbuf = outBuf.read()  ;
		if (  intbuf!=-1  ) {
			char charbuf =  (char) intbuf ;
			String tmpStr =   String.valueOf (  charbuf  );
			return tmpStr ;
		}else { try{ Thread.sleep (1000) ; } catch (Exception exc) {} ;   return "" ; }
		// return outBuf.readLine() ;
	}

	/**
	Finalize the Shadow instance.
	Kill the background shadow process and delete the named pipes from the hard disc
	@throws java.lang.NoSuchFieldException if the pipes have not been created yet
	*/
	public void detach() throws java.lang.NoSuchFieldException{
		// Dleting the files:
		new File ( getPipeOut() ).delete();
		new File ( getPipeIn() ).delete();
		new File ( pipeRoot ).delete();
		// Killing the shadow proces:
		String command = "kill -9 "+ pid ;
		if (pid!=0)
			Api.shadow( command );
		pipeRoot=null ;
	};


	/**
	* Retrieve the name of the pipe that listens for the standard input
	* @return the string representation of the input pipe name for the current job
	* @throws java.lang.NoSuchFieldException the pipe has not been created yet
	*/
	public String getPipeIn() throws NoSuchFieldException {
		if (pipeRoot!=null) return pipeRoot +".in" ;
		else  throw new NoSuchFieldException("Shadow named pipe not yet created. Unable to retrieve")  ;
	};
	/**
	* Retrieve the name of the pipe that listens for the standard output
	* @return the string representation of the output pipe name for the current job
	@throws java.lang.NoSuchFieldException the pipe has not been created yet
	*/
	public String getPipeOut()throws NoSuchFieldException {
		if (pipeRoot!=null)
		return pipeRoot +".out" ;
		else  throw new NoSuchFieldException("Shadow named pipe not yet created. Unable to retrieve")  ;
	};

	/**
	* Retrieve the process id corresponding to the shadow background process
	* @return the id of the process related to the running shadow
	* @throws java.lang.NoSuchFieldException the process has not been launched yet */
	public int getPid() throws NoSuchFieldException , IOException  {
		if (pipeRoot!=null)
		return pid ;
		else  throw new NoSuchFieldException("Shadow named pipe not yet created. Unable to retrieve process Id")  ;
	}
	/**
	* Retrieve the Name of the host 
	* @return the string representation of the current host
	* @throws UnknownHostException the system is unable to retrieve host information */
	static public String getHost ()  throws UnknownHostException{
			String hostname = "" ;
			String ipAddr = "";
			InetAddress addr = InetAddress.getLocalHost();
			// System.out.println ( " addr = [" + addr.toString()+"]" );
			StringTokenizer tks = new StringTokenizer ( addr.toString() , "/" );
			if ( tks.countTokens() == 2 ){
				hostname = tks.nextToken ( );
				// ipAddr = tks.nextToken ( );
				return tks.nextToken ( );
			} else throw new UnknownHostException ("Unable to parse the Inet ip address") ;
	}
	/**
	* Retrieve the shadow port
	* @return the port where the shadow proces is listening to
	* @throws java.lang.NoSuchFieldException the process has not been launched yet */
	public int getPort()throws NoSuchFieldException  {
		if (pipeRoot!=null)   return port ;
		else  throw new NoSuchFieldException("Shadow named pipe not yet created. Unable to retrieve process Id")  ;
	}
	/**
	* Retrieve the id of the Job for the current Shadow
	* @return the JobId instance linked to the Shadow
	*/
	public JobId getJobId ()    {  return jobId ; }



	void start()   {  new Thread(  this ).start();  /* launch the listener */ };
	/**  Perform a grid-console-shadow  */
	int console( ) { return console (0 ); }
	/**  Perform a grid-console-shadow
	* @param port the port where the listener listen to from the agent   */
	int console( int port )   {
		pipeRoot = "/tmp/listener-" + jobId.unique;
		// Launch the grid-console-shadow with --logfile (background process)
		String shPath = Api.getEnv (GLITE_WMS_LOCATION) ;
		if (  shPath== null  )
			shPath = OPT_GLITE ;
		shPath = shPath +SEP +BIN +SEP + "glite-wms-grid-console-shadow"  ;
		File sh = new File ( shPath ) ;
		if (  !sh.isFile () )
			throw new RuntimeException ("Unable to find listener shadow executable: " +shPath ) ;
		String command = shPath ;
		String gtp_env  = Api.getEnv ("GLOBUS_TCP_PORT_RANGE") ;
		int fromP =0, toP =0;
		if (gtp_env!= null){
			// environment variable found
			String sep[] = {" " , ":" , "-" , "," } ;
			for (   int i = 0 ; i< sep.length ; i++  ){
				if ( gtp_env.indexOf( sep[i] ) !=-1  ){
					fromP =  java.lang.Integer.parseInt(   gtp_env.substring(0 , gtp_env.indexOf( sep[i] ) ).trim()  ) ;
					toP    =  java.lang.Integer.parseInt(   gtp_env.substring(gtp_env.indexOf( sep[i] )    ).trim()    );
					break;
				}
			}
		}
		if (port !=0){
			if (    (port < fromP  ) || (port > toP)    )
				throw new   RuntimeException( "Unable to perform attachement: port exceeds firewall range [" + gtp_env +"]");
			command += " -port " + port ;
		}
		command += " -log-to-file " + pipeRoot +"&";
		if ( Api.shadow( command  )  != 0 )
			throw new RuntimeException ("Unable to perform the listener shadow executable" ) ;
		/* Open the Pipe */
		int timeout = 0;
		BufferedReader pipe =null;
		boolean toSleep = false ;
		while ( timeout< 10 ){
			try {  if (toSleep){Thread.sleep ( 1000  ) ; toSleep=false ; } pipe = new BufferedReader ( new FileReader ( pipeRoot ) ) ; break ;
			}catch (Exception exc){  timeout++ ;  toSleep= true ; }
		}
		if (timeout ==10)  throw new RuntimeException ("Unable to open listener shadow info Stream:\n" + pipeRoot ) ;
		// get the pid and the port:
		String ad = new String();
		char[] buffer = new char[1];
		timeout=0 ;
		while (  timeout< 10  ){
			try {
			if (toSleep)
				{Thread.sleep ( 1000  ) ; toSleep=false ; }
			ad += String.valueOf (   (char) pipe.read()  ) ;
			if (  ad.trim().endsWith("]") )
				break ;
			}catch (  Exception exc){  timeout++ ;  toSleep= true ; }
		}
		if (timeout ==10)  throw new RuntimeException ("Unable to properly read the listener shadow info Stream:\n" + pipeRoot ) ;
		ClassAdParser cp = new ClassAdParser( ad  )  ;
		RecordExpr re = (RecordExpr) cp.parse() ;
		Constant co = (Constant) re.lookup("PORT") ;
		this.port = co.intValue() ;
		co = (Constant) re.lookup("PID") ;
		pid =co.intValue() ;
		if    (    fromP>0    &&   ( (this.port < fromP  ) || (this.port > toP))    ){
			// try { detach(); } catch (NoSuchFieldException exc) {}
			finalize();
			throw new   RuntimeException( "Unable to perform attachement: port exceeds firewall range [" + gtp_env +"]");
		}
		System.out.println ("Shadow::console-->Console Started Successffully") ;
		return 0 ;
	};
	/******
	*  private / packages Members:
	*******/
	int port ;	
	private int pid =0;
	private String host;
	private Listener listener ;
	private String pipeRoot = null;
	private BufferedReader outBuf , errBuf ;
	private BufferedWriter barbaWriter = null ;
	private JobId jobId;
	private static final String SEP = "/" ;
	private static final String GLITE_WMS_LOCATION = "GLITE_WMS_LOCATION";
	private static final String OPT_GLITE = SEP + "opt"+ SEP +  "glite";
	private static final String BIN = SEP + "bin" ;
}; //end Shaodw class

