import org.glite.wmsui.apij.* ;
import org.globus.io.urlcopy.UrlCopy ;
import org.globus.util.* ;   // GlobusUrl class

public class GliteJobGetLogInfo {
    public static void main(String[] args)   throws Exception {



/*
String source_path =  "gsiftp://ibm139.cnaf.infn.it//tmp/caccapuzzosa" ;
String dest_path = "file:////tmp/caccapuzzosa" ;
UrlCopy uCopy = new UrlCopy();
uCopy.setSourceUrl ( new GlobusURL(  source_path  )  ) ;
uCopy.setDestinationUrl (new GlobusURL ( dest_path  ) );
uCopy.copy() ;
*/



        String help = "\nUsage:\nGliteJobGetLogInfo  <jobid> [level]" ;
	Url ns , lb ;
	if (   (args.length <1 ) || ( args.length >2 ) ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	int level = 0 ;
	try{
		if  (args.length ==2 )
			level = java.lang.Integer.parseInt( args[1] ) ;
		Job job = new Job (new JobId ( args[0] ) ) ;
		System.out.println ( job.getLogInfo().toString (level) ) ;
		// job.getStatus() ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
