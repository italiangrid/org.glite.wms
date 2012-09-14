import org.glite.wmsui.apij.* ;

public class GliteJobGetState {
    public static void main(String[] args)   throws Exception {
        String help = "\nUsage:\nGliteJobGetState  <jobid> [step]" ;
	Url ns , lb ;
	if (   (args.length <1 ) || ( args.length >2 ) ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	int step = 0;
	try{
		if  (args.length ==2 )
			step =   java.lang.Integer.parseInt(   args[1]  );
		Job job = new Job (new JobId ( args[0] ) ) ;
		System.out.println ( job.getState(step).toString(true, true) ) ;
		// job.getStatus() ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
