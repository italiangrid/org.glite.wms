#include "wmproxy_api.h"

using namespace std;
using namespace glite::wms::wmproxyapi;
int main()
{
	cout << "getVersion????" << getVersion()  << endl ;	
	string exe  , arg , req ,rank;
	exe = "exe" ;
	arg = "arg" ;
	req="req";
	rank="rank";
	//cout << "job Template" <<  getJobTemplate (1,exe , arg, req, rank) << endl ; 
	return 0;
}
