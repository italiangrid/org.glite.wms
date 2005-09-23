import org.glite.wmsui.apij.* ;
import org.glite.wms.jdlj.* ;
import java.util.Vector;

public class GliteJobsSubmit {
    public static void main(String[] args)   throws Exception {
	String help = "\nUsage:\nGliteJobsSubmit  <NS address>  <LB address>  <JDL file(s)>" ;
	if (args.length <1 ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	String nsHost  = args [0] ;
	String lbHost  = args [1] ;
	Url ns = new Url (nsHost);
	Vector lbs = new Vector ();
	lbs.add ( new Url (lbHost)  ) ;
	JobCollection collect = new JobCollection () ;
	JobAd jobad  ;
	for (int i = 2 ; i< args.length ; i++)
		try{
			jobad =  new JobAd () ;
			jobad.fromFile (args[i]) ;
			collect.insertAd (  jobad) ;
			// collect.setMaxThreadNumber(1);
			collect.setLoggerLevel(6) ;
		}catch (Exception exc){ System.out.println( exc.getMessage() );  }
	try{  System.out.println (   collect.submit ( ns , lbs , null  )    ) ;    }
	catch (Exception exc){   exc.printStackTrace() ;    System.exit (1); }
   }
}
