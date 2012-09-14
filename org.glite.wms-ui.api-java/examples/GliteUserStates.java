import org.glite.wmsui.apij.* ;
import org.glite.jdl.* ;
import java.util.* ;

class GliteUserStates {
public static void main(String[] args)   throws Exception {
	String help = "\nUsage:\nGliteUserStates <LB host> <LB port> [verbosity]" ;
	if ( args.length != 2 ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	Url  lb ;
	try{
	   String lbHost  = args [0] ;
	   int lbPort  =  java.lang.Integer.parseInt( args[1]) ;
	   lb = new Url (lbHost, lbPort) ;
	   UserJobs uj = new UserJobs () ;
	   System.out.println("Retrieving user jobs from Lb server, please wait....") ;
	   Vector jobs = uj.getStates(  lb  ) ;
	   for (int i = 0 ; i< jobs.size() ; i++ ) System.out.print ( jobs.get(i) + "\n" );
	}catch (Exception exc){
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
