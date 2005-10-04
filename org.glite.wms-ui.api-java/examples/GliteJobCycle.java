import org.glite.wmsui.apij.* ;
import org.glite.jdl.* ;
import java.io.File ;
import java.util.* ;





class GliteJobCycle {

static final int MAX_WAITING_TIME= 10000000 ;




static void retrieveOutput (Job job, String path)  throws Exception{
	System.out.println("Retrieving output...");
	System.out.println(   job.getOutput(path)   );
}

static void waitForDone (Job job, int mseconds )  throws Exception{
        boolean done = false;
        Result result ;
	System.out.println("JobId.... " + job.getJobId()  ) ;
        while (done==false) {
                if (mseconds > MAX_WAITING_TIME)
                        throw new UnsupportedOperationException ("warning Timeout reached!");
		System.out.println("Waiting " + mseconds +" milli seconds...." );
        	Thread.sleep(mseconds);
		result = job.getStatus() ;
                if (result.getCode()!=0)
                        throw new UnsupportedOperationException ("Unable to retrieve JobStatus: " + result.getCode()+ Result.ACCEPTED  );
                JobStatus status =(JobStatus)(result.getResult());
		System.out.println("Status is " + status.name () );
		System.out.println("Warning:DONE CODE IS " + status.getValInt(JobStatus.DONE_CODE) );
		switch (status.code()){
			case JobStatus.DONE:
				if (status.getValInt(JobStatus.DONE_CODE) == 0){
				 	// Job Ended successfully, ready for file retrieving
				        System.out.println("OK! DONE CODE IS 0");
				 	return ;
				} else System.out.println("Warning:DONE CODE IS " + status.getValInt(JobStatus.DONE_CODE) );
			case  JobStatus.CLEARED:
			case JobStatus.ABORTED: 
			case JobStatus.CANCELLED:
			case  JobStatus.UNKNOWN:
			case JobStatus.PURGED:
			case JobStatus.UNDEF:
				throw new UnsupportedOperationException ("Unable to continue, Job status is " + status.name() );
			default:
				break;

		}
		mseconds = mseconds*(mseconds/20) ;

        }
}


    public static void main(String[] args)   throws Exception {
/*
	System.out.println ("Creating credential....");
	UserCredential pippo = new UserCredential() ;
	System.out.println ("DONE");
*/


        String help = "\nUsage:\nGliteJobCycle <Ns Address> <Lb address> <JDL file> [outputPath]" ;
	Url ns , lb ;
	if ( args.length < 3 ){
	   System.out.println ("\nSyntax Error: "+ help );
	   System.exit (1);
	}
	try{
	   String nsHost  = args [0] ;
	   String lbHost  = args [1] ;
	   ns = new Url (nsHost);
	   lb = new Url (lbHost) ;
	   String jdl = args[2] ;
	   JobAd jobad = new JobAd () ;
	   jobad.fromFile (jdl) ;
	   // Job CREATION
	   // Job job = new Job ( jobad ) ;
	   String path = "/tmp" ;
	   if (args.length==4)  path =args[3] ;
	   // Proper Submission
	   // Result res = job.submit (ns , lb , null  , null , null) ;
	   System.out.println("res.getResult() point, now waiting..." ) ;
	   Job job = new Job  (new JobId (path)  );
	   Thread.sleep(1000);
	   waitForDone (job , 100) ;
	   retrieveOutput (job,  "/tmp") ;
	
	}catch (Exception exc){
	   System.out.println ("\nError: "+ help );
	   exc.printStackTrace() ;
	   System.exit (1);
	}
   }
}
