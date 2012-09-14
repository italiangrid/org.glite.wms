/* ********************************************************************
* File name: Shadow.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
*********************************************************************/
#include "glite/wmsui/api/Shadow.h"
#include "glite/wmsui/api/JobExceptions.h"
#include "glite/jdl/Ad.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRINGSTREAM
#include <sstream>
#else
#include <strstream>
#endif

using namespace glite::wmsutils::jobid ; //JobId
using namespace std ;

namespace glite {
namespace wmsui {
namespace api {

	Shadow::Shadow (JobId jid , Listener *ls) {set(jid , ls);}
	Shadow::Shadow (){};
	Shadow::~Shadow(){};
	void Shadow::set(JobId jid ,   Listener* ls) {  jobId = jid ; listener = ls ; } ;
	void Shadow::attach(int port){
		// Query to LB: host & port
		// if Query--> Error :  Do nothing, proceed anyway
		// If old Shadow is up  ---> kill process
		// start the new process (the named pipeline are the same) (if error -> EXC)
		console (port) ;
		// Log new host / port to LB (if error -> EXC)
		// Run listener
	} ;

	/**  Read from the specified named pipe and return the string */
	string Shadow::empty(string bufferName){
		string result="" ,outMsg;
		filebuf fb;
		fb.open (bufferName.c_str() , ios::in);
		istream jdl_in(&fb);
#ifdef HAVE_STRINGSTREAM
		stringstream buffer ;
#else
		strstream buffer;
#endif
		int leng = 2048;
		char tmp[leng];
		while (  jdl_in.getline(tmp,leng,'\n')  ){
		result +="\n" + string(tmp) ;
		}
		fb.close() ;
		return result ;
	} ;
	string Shadow::getPipeErr(){        return pipeRoot +".err" ;    };
	string Shadow::getPipeIn(){       return pipeRoot +".in" ;    };
	string Shadow::getPipeOut(){       return pipeRoot +".out" ;    };
	string Shadow::getHost(){       return "localHost" ; /*TBD ! ! */   };
	int Shadow::getPid(){       return pid ;    };
	int Shadow::getPort(){       return port ;    };
	/*********************
	* kill
	*********************/
	void Shadow::kill (){
		if (pid != 0 ){
			char tmp [2048];
			sprintf(tmp , "%d", pid) ;
			string command = "kill -9 " + string(tmp);
			system(command.c_str());
		}
	}
	/*********************
	* Detach
	*********************/
	void Shadow::detach(){
		kill() ;
		remove(   getPipeErr().c_str()    );
		remove(   getPipeIn().c_str()    );
		remove(   getPipeOut().c_str()    );
	};
	/*********************
	* Start
	*********************/
	void Shadow::start(){
		listener->run( this) ;
	};

	/** Launch the console shadow and read pid and port from the named written pipe   */
	void Shadow::console( int port){
		GLITE_STACK_TRY("Shadow::console()") ;
		// Kill the grid-console-shadow, if still running:
		kill () ;
		pipeRoot = "/tmp/listener-" + jobId.getUnique();
		// Launch the grid-console-shadow with --logfile (background process)
		string arguments = " -log-to-file" +pipeRoot ;
		int fromP =0, toP =0 ;
		char* gl_env  =getenv ("GLOBUS_TCP_PORT_RANGE") ;
		if (gl_env!= NULL){
			// environment variable found
			string gtp_env = string( gl_env  );
			string sep[] = {" " , ":" , "-" , "," } ;
			for (unsigned int i = 0 ; i< sep->length() ; i++){
				//cout << "Looking for '" << sep[i] <<"' in " << gtp_env << flush ;
				if ( gtp_env.find( sep[i] ) <  gtp_env.size()  ){
					fromP = atoi (  gtp_env.substr(0 , gtp_env.find( sep[i] ) ).c_str()   ) ;
					toP    = atoi (  gtp_env.substr(gtp_env.find( sep[i] )  ).c_str()   ) ;
						break;
				}
			}
		}
		if (port !=0){
			if (    (port < fromP  ) || (port > toP)    )
				throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Unable to perform attachement: port exceeds firewall range"  ) ;
			arguments += "-port " ; //TBD + repr (port)
		}
		char* shPath_ch = getenv ("GLITE_WMS_LOCATION") ;
		string shPath  ;
		if (  shPath == NULL)
			shPath = "/opt/glite" ;
		shPath = string(shPath_ch) + "/bin/glite-wms-grid-console-shadow"  ;
		string command = shPath  +arguments +" &";
		if (  system(command.c_str())   )
			throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Unable to launch the Shadow listaner executable: "+ shPath  ) ;
		// get the pid and the port:
		string adStr ;
		int timeout= 0 ;
		while (timeout<10){
			try{
				adStr += empty( pipeRoot   );
			}catch (exception &exc){
				sleep(1) ;
				timeout++ ;
			}
			if ( adStr.find_last_of( "]")<adStr.length() )
			break ;
		}
		if (timeout==10)
			throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Unable to read listener named pipe streams" );
	
		glite::jdl::Ad ad ( adStr );
		this->port = ad.getIntValue("PORT")[0];
		pid  = ad.getIntValue("PID")[0];
		if  (   fromP>0 &&   (   (this->port < fromP  ) || (this->port > toP)    )   ){
			detach();
			throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Unable to perform attachement: port exceeds firewall range" );
		}
		GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
	};

} // api
} // wmsui
} // glite
