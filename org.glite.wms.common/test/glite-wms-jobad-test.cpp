/* ************************************************
    Examples Methods
**************************************************/

#include "../src/utilities/Exceptions.h"
#include "../src/requestad/JobAd.h"

#include "../src/requestad/JdlAttributeList.h"
#include "../src/requestad/JDLAttributes.h"




using namespace edg::workload::common::utilities ;
using namespace edg::workload::common::requestad ;
using namespace std ;
using namespace classad ;


using namespace edg::workload::common::requestad ;
using namespace std ;

int EXjobad(const string& jobadfile )  {
try{
	cout << "\nLoading JobAd from File: " << jobadfile << endl ;
	JobAd jab;
	jab.fromFile(jobadfile);

	jab.setDefaultRank("5" );
	jab.setDefaultReq("True") ;

	cout << "\nDone.\nJOBAD  Lines Representation:->"<< flush << jab.toLines() << endl ;
	cout << "\nJOBAD  Once AGAIN!! Lines Representation:->"<< flush << jab.toLines() << endl ;
	cout << "\nJOBAD Submission String->" << flush << jab.toSubmissionString() << endl ;
	cout << "\nJOBAD  Once AGAIN!! Lines Representation:->"<< flush << jab.toLines() << endl ;
	cout << "\nJOBAD One more Submission String->"<< flush  << jab.toSubmissionString() << endl ;
} catch (Exception &exc) {
	cout  <<  "\nStack Trace:\n"  ;
	cout << exc.printStackTrace() ;
	cout  << "\nWhat:\n"  ;
	cout << exc.what() ;
	return 1;
} catch (exception &exc ){
	cout  << "\nstd exception found : What:\n"  ;
	cout << exc.what() ;
	return 2;
} catch (...) {
	cout  << "\n\n\n WARNING!!!!!\nUnkwnow exception, please contact the developer\n"  ;
	return 3;
}
	return 0;
}
int main(int argc,char *argv[]){
   if (argc < 2 ||  strcmp(argv[1],"--help") == 0)  {
     cout << "Usage : " << argv[0] << "  <JDL file>" << endl;
     return 1;
   }
   int ret = 0;
   ret =  EXjobad( argv[1]  );
   cout << "\n" ;
   return ret;
};

