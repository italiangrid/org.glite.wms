
import org.glite.wmsui.apij.* ;
import org.glite.wms.jdlj.* ;
import java.io.File ;
import java.util.* ;

class GliteDagSubmit {
	public static void main(String[] args)   throws Exception {
		String help = "\nUsage:\nGliteDagSubmit   <Ns Address>  <Lb address>  <JDL file> [ns logger level]" ;
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
			// Interactive Jobs management
			if (jobad.hasAttribute (Jdl.JOBTYPE) )  if (  jobad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_INTERACTIVE ) ) ls = new ListenerConsole () ;
			Job job = new Job ( jobad ) ;
			if (args.length==4){
				job.setLoggerLevel(    java.lang.Integer.parseInt( args[3] )    ) ;
			}
			Result res = job.submit (ns , lb , null  , ls , state) ;
			System.out.println(res.getResult() ) ;
		}catch(javax.naming.directory.InvalidAttributeValueException exc){
			System.out.println ("Error: JobAd invalid attribute caught:\n");
			exc.printStackTrace() ;
			System.exit (1);
		}catch(java.net.UnknownHostException exc){
			System.out.println ("Error: UInable to contact host:\n");
			exc.printStackTrace() ;
			System.exit (1);
		}catch(JobAdException exc){
			System.out.println ("Error: JobAd attribute semantic:\n");
			exc.printStackTrace() ;
			System.exit (1);
		}catch(NoSuchFieldException exc){
			System.out.println ("Error: Mandatory attribute not found:\n");
			exc.printStackTrace() ;
			System.exit (1);
		}catch(UnsupportedOperationException exc){
			System.out.println ("Error: Submission Not allowed\n");
			exc.printStackTrace() ;
			System.exit (1); // Not allowed
		}catch(java.io.FileNotFoundException exc){
			System.out.println ("Error:Unable to find credential certificate:\n");
			exc.printStackTrace() ;
			System.exit (1); // Proxy
		}catch(org.globus.gsi.GlobusCredentialException exc){
			System.out.println ("Error: Wrong credential certificate properties:\n");
			exc.printStackTrace() ;
			System.exit (1); // Proxy
		}catch(Exception exc){
			System.out.println ("\n Unexpected Exception\n") ;
			exc.printStackTrace() ;
			System.exit (1);
		}
	}
}
