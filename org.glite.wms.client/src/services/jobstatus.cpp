

#include "jobstatus.h"


using namespace std ;

namespace glite {
namespace wms{
namespace client {
namespace services {


JobStatus::JobStatus(){
};

void JobStatus::readOptions (int argc,char **argv){
	/*
        jobIds = wmcOpts->getJobIds ( );
        if ( jobIds.empty( ) )
        	throw WmsClientException(__FILE__,__LINE__,
			"readOptions",DEFAULT_ERR_CODE,
			"JobId Error", "no valid jobid(s)");
	}
        jobIds = wmcUtils->checkJobIds( jobIds, wrongids);
        */
};

void JobStatus::getStatus ( ){

};

}}}} // ending namespaces
