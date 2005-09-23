import org.glite.wmsui.apij.* ;
import org.globus.io.urlcopy.UrlCopy ;
import org.globus.util.* ;   // GlobusUrl class
import java.io.File  ;

public class GliteJobGetOutput {
    public static void main(String[] args)   throws Exception {



/*
String source_path =  "gsiftp://ibm139.cnaf.infn.it//tmp/caccapuzzosa" ;
String dest_path = "file:////tmp/caccapuzzosa" ;
UrlCopy uCopy = new UrlCopy();
uCopy.setSourceUrl ( new GlobusURL(  source_path  )  ) ;
uCopy.setDestinationUrl (new GlobusURL ( dest_path  ) );
uCopy.copy() ;
*/



        String help = "\nUsage:\nGliteJobGetOutput   <jobid> [dir]" ;
	Url ns , lb ;
	if (   (args.length <1 ) || ( args.length >2 ) ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	String dirPath = "/tmp";
	try{
		JobId jid = new JobId ( args[0] ) ;
		if  (args.length ==2 )
			dirPath = args[1]  ;
		dirPath = dirPath + "/"  + System.getProperty( "user.name" ) +  "_" +    jid.getUnique ()  ;

		File path = new File (dirPath);
		/* Do nothing, The path already exist and it is a directory: OK */
		if ( path.isDirectory() ) {/** OK */}
		// The path exists but it is not a directory
		else  if ( path.exists() )
			throw new UnsupportedOperationException ("The path " +dirPath + " is already existing but it is not a directory" );
		// The path does not exist. Try to create the directory
		else
			if  (!path.mkdir() )
				throw new UnsupportedOperationException ("Unable to create the path: " + dirPath );

		Job job = new Job ( jid  ) ;
		System.out.println ( job.getOutput( dirPath  ) ) ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
