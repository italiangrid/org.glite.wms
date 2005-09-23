import org.glite.wmsui.apij.* ;


public class GliteJobAttach {
    public static void main(String[] args)   throws Exception {
        String help = "\nUsage:\nGliteJobAttach  <jobid> [port]" ;
	Url ns , lb ;
	if (   (args.length <1 ) || ( args.length >2 ) ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	int port = 0;
	try{
		if  (args.length ==2 )
			port =   java.lang.Integer.parseInt(   args[1]  );
		Job job = new Job (new JobId ( args[0] ) ) ;
		// job.attach ( new ListenerFrame ( ) , port ) ;
		job.attach ( new ListenerConsole ( ) , port ) ;
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
