import org.glite.wmsui.apij.* ;
import java.util.Vector;

public class GliteDagsSubmit {
    public static void main(String[] args)   throws Exception {
	String help = "\nUsage:\nGliteDagsSubmit  <NS address>  <LB address>  <DagAd file(s)>" ;
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
	for (int i = 2 ; i< args.length ; i++)
		try{
			collect.insert (  new Job ( args[i] ) );
			//collect.setMaxThreadNumber(5);
		}catch (Exception exc){ System.out.println( exc.getMessage() );  }
	try{  System.out.println (   collect.submit ( ns , lbs , null  )    ) ;    }
	catch (Exception exc){   exc.printStackTrace() ;    System.exit (1); }
   }
}
