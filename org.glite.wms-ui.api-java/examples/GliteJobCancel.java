import org.glite.wmsui.apij.* ;

public class GliteJobCancel {
    public static void main(String[] args)   throws Exception {
        String help = "\nUsage:\n  GliteJobCancel <jobid> " ;
	if ( args.length !=1 ) {
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	try{
		Job job = new Job (new JobId ( args[0] ) ) ;
		System.out.println ( job.cancel() ) ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
