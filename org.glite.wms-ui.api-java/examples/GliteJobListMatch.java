import org.glite.wmsui.apij.* ;
import org.glite.jdl.* ;
import java.io.File ;
import java.util.* ;

class GliteJobListMatch {
    public static void main(String[] args)   throws Exception {
        String help = "\nUsage:\nGliteJobListMatch    <Ns address>  <JDL file>" ;
	Url ns; 
	if ( args.length < 2 ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	try{
	   String nsHost  = args [0] ;
	   ns = new Url (nsHost);
	   String jdl = args[1] ;
	   JobAd jobad = new JobAd () ;
	   jobad.fromFile (jdl) ;
	   Job job = new Job ( jobad ) ;
	   Result res = job.listMatchingCE ( ns ) ;
	   System.out.println ( "\n" + res );
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
