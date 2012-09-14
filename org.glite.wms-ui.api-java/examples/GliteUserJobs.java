import org.glite.wmsui.apij.* ;
import org.glite.jdl.* ;
import java.text.DateFormat ;
import java.util.* ;

class GliteUserJobs {
	public static void main(String[] args)   throws Exception {
		String help = "\nUsage:\nGliteUserJobs <LB host> <LB port>  \n\t[-all] \n\t[ -from=<yyyyMMddhhmmss>]  \n\t[-to=<yyyyMMddhhmmss>]  "+
		"\n\t[-utags=<tag name>=<value>]\n\t[-s=<status value>]\n\t[-e=<status value>]\n"  ;
		Url  lb ;
		if ( args.length <3 ) {
			System.out.println ("\nSyntax Error: "+ help );
			System.exit (1);
		}
		try{
			Vector jobs ;
			String lbHost  = args [0] ;
			int lbPort  =java.lang.Integer.parseInt( args[1]) ;
			lb = new Url (lbHost, lbPort) ;
			UserJobs uj = new UserJobs () ;
			boolean all = false ;
			if ( args.length >2   ){
				Query query = new Query () ;
				GregorianCalendar from , to ;
				java.text.SimpleDateFormat df = new java.text.SimpleDateFormat ("yyyyMMddhhmmss") ;
				for (int i = 2 ; i< args.length ; i++){
					if (args[i].startsWith("-from=") ){
						from = new GregorianCalendar () ;
						from.setTime(  df.parse ( args[i].substring(6 )   ) ) ;
						query.setTimeFrom ( from ) ;
					}else if (args[i].startsWith("-to=")  ){
						to = new GregorianCalendar () ;
						to.setTime(  df.parse ( args[i].substring(4)   ) ) ;
						query.setTimeTo ( to ) ;
					}else if (args[i].startsWith("-utags=") ){
						String [] ut= args[i].substring(7).split("=") ;
						if ( ut.length!=2 ) throw new RuntimeException("Unable to parse the value: " + args[i] );
						query.setUserTag ( ut[0] , ut[1] ) ;
					}else if (args[i].startsWith("-all") ){
						query.setOwned () ;
					}else if (args[i].startsWith( "-s=" ) ){
						query.setInclude ( java.lang.Integer.parseInt(args[i].substring(3))   ) ;
					}else if (args[i].startsWith( "-e=" ) ){
						query.setExclude ( java.lang.Integer.parseInt(args[i].substring(3))   ) ;
					}else throw new RuntimeException("Unable to parse the value: " + args[i] );
				}
				// PERFORM THE QUERY
				System.out.println ( "Performing query:\n***\n" +query  +"\n***\nPlease Wait....." ) ;
				jobs = uj.getJobs(  lb ,query ) ;
			}else jobs = uj.getJobs(  lb  ) ;
			for (int i = 0 ; i< jobs.size() ; i++ ) System.out.print ( jobs.get(i) + "\n" );
			if (jobs.size()==0) System.out.println("Unable to find any job matching your query, please try again modifying it") ;
		}catch (Exception exc){
			exc.printStackTrace() ;
			System.exit (1);
		}
	}
}
