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

void EXjobad(const string& jobadfile )  {
   try{
	JobAd jab;
	jab.fromFile(jobadfile);
	cout << "\nJOBAD  Lines Representation:->" << jab.toLines() << flush;
	cout << "\nJOBAD Submission String->" << jab.toSubmissionString() << flush;

   } catch (Exception &exc) {
	cout << "\n\nException found!!!.\nHere follows the error description:" ;
	cout  <<  "\nStack Trace:\n"  ;
	cout << exc.printStackTrace() ;
	cout  << "\nWhat:\n"  ;
	cout << exc.what() ;
   } catch (exception &exc ){
	cout  << "\nstd exception found : What:\n"  ;
	cout << exc.what() ;
   } catch (...) {
	cout  << "\n\n\n WARNING!!!!!\nUnkwnow exception, please contact the developer\n"  ;
   }
}

int main(int argc,char *argv[]){
   if (argc < 2 ||  strcmp(argv[1],"--help") == 0)  {
     cout << "Usage : " << argv[0] << "  <JDL file>" << endl;
     return 1;
   }

   EXjobad( argv[1]  );
   cout << "\n" ;
   return 0;
};

