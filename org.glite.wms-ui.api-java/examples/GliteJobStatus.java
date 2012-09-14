import org.glite.wmsui.apij.* ;

public class GliteJobStatus {
    public static void main(String[] args)   throws Exception {
        String help = "\nUsage:\n GliteJobStatus <jobid> [verbosity]" ;
	Url ns , lb ;
	if (   (args.length <1 ) || ( args.length >2 ) ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	int level = 0;
	try{
		if  (args.length ==2 )
			level = java.lang.Integer.parseInt( args[1] ) ;
		Job job = new Job (new JobId ( args[0] ) ) ;
		System.out.println ( job.getStatus ().toString (level) ) ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
