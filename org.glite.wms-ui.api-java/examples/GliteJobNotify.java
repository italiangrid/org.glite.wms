import org.glite.wmsui.apij.* ;

public class GliteJobNotify {
    public static void main(String[] args)   throws Exception {
        String help = "\nUsage:\n GliteJobNotify <jobid> <timeout> <state> [<state>] [<state>] [<state>]  [<state>] " ;
	help +="\nStates values: SUBMITTED=1\tWAITING=2\nREADY=3\tSCHEDULED=4\tRUNNING=5\nDONE=6\tCLEARED=7\tABORTED=8\nCANCELLED=9\tPURGED=10" ;
	Url ns , lb ;
	if (   args.length <3 ) {
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}try{
		int [] states = new int [args.length-2]  ;
		int seconds =   java.lang.Integer.parseInt(  args[1] ) ;
		Job job = new Job (new JobId ( args[0] ) ) ;
		for (int i = 2; i< args.length ; i++) {
			states[i-2] =  java.lang.Integer.parseInt(  args[i] ) ;
			System.out.println("Waiting notification for.... "  +JobStatus.code[ states[i-2]]  );
		}
		System.out.println("Waiting " + seconds+" seconds for Job Notification...");
		System.out.println ( job.notify( states , seconds ).toString(3) ) ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
