import org.glite.wmsui.apij.* ;
import org.glite.wms.jdlj.* ;
import java.io.File ;
import java.util.* ;

class GliteJobSubmit {
    public static void main(String[] args)   throws Exception {


	System.out.println ("Creating credential....");
	UserCredential pippo = new UserCredential() ;
	System.out.println ("DONE");






        String help = "\nUsage:\nGliteJobSubmit   <Ns Address>  <Lb address>  <JDL file> [ns logger level]" ;
	Url ns , lb ;
	if ( args.length < 3 ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	try{
	   String nsHost  = args [0] ;
	   String lbHost  = args [1] ;
	   ns = new Url (nsHost);
	   lb = new Url (lbHost) ;
	   String jdl = args[2] ;
	   JobAd jobad = new JobAd () ;
	   jobad.fromFile (jdl) ;
	   // Special features
	   Listener ls = null ;
	   JobState state = null ;
	   //if (jobad.hasAttribute (Jdl.JOBTYPE) )  if (  jobad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_INTERACTIVE ) ) ls = new ListenerConsole () ;
	   Job job = new Job ( jobad ) ;
	   //job.setCredPath ( new File ( "/tmp/x509up_u502") ) ;
	   //System.out.println( "Submitting the following JDL:\n " + jobad.toLines() +"\nSubmitting to NS:  " +ns  +"\nSubmitting to LB:  " + lb    );
	   if (args.length==4)  job.setLoggerLevel(    java.lang.Integer.parseInt( args[3] )    ) ;
	   Result res = job.submit (ns , lb , null  , ls , state) ;
	   System.out.println(res.getResult() ) ;
	}catch (Exception exc){
	   System.out.println ("\nError: "+ help );
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
