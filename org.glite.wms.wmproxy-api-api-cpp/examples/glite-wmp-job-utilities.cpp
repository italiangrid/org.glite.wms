
#include "wmp_job_examples.h"

using namespace std;

namespace glite {
namespace wms {
namespace wmproxyapi {
namespace examples {
namespace utilities {

char* clean(char *str)
{
    int ii = 0;
    int len = 0;
    unsigned int p = 0;
    string *s = NULL;

   // erases white space at the beginning of the string
    for (ii = 0; str[ii] == ' ' || str[ii] == '\t'; ii++);
    str = &(str[ii]);

    len = strlen (str);

    // erases white space at the end of the string
    if (len > 0) {
        for (ii = len - 1; (str[ii] == ' ' || str[ii] == '\t'); ii--);
        str[ii + 1] = '\0';
    }

    s = new string(str);

    p = s->find("\n");
    if ( p != string::npos ){
	*s = s->substr (0, ii-1);
    }

    return ((char*)s->c_str()) ;
}


string* jobidFromFile (const string &path)
{
	ostringstream bstream;
	string *j_id = NULL;
	char ch ;

         ifstream file(path.c_str());
         if (file.is_open())   {
         	while( ! file.eof() )
                {
                        file.get(ch);
                        bstream << ch;
                }
               file.close();

		j_id = new string( bstream.str() );
		*j_id = clean ((char*)j_id->c_str());
        }

	return j_id;
}

string handle_exception (const BaseException &b_ex )
{
	string meth = b_ex.methodName.c_str();
	string *errcode =b_ex.ErrorCode ;
	string *description = b_ex.Description ;

	string excmsg = "";

	if (meth.size() > 0){
		excmsg += "method: " +meth + "\n";
	}

	if (errcode && errcode->size() > 0 ){
			excmsg += "error code:\t" + *errcode + "\n";
	}
	if (description && description->size() > 0 ){
			excmsg += "description:\t" + *description + "\n";
	}

	return excmsg;
}

int saveToFile (const string &path, const string &bfr)
{
	int result = 0;
	char date[1024] = "";
	time_t now = time(NULL);
	struct tm *tn = localtime(&now);

 	 sprintf (date, "%s-%d-%d-%d%d%d%.2d%.2d%.2d-%s :\n\n",
       				 tn->tm_year, tn->tm_mon, tn->tm_mday, tn->tm_hour, tn->tm_min, tn->tm_sec );

	ofstream outputstream(path.c_str(), ios::app);
	if (outputstream.is_open() )
	{
		outputstream << date << ends;
		outputstream << bfr << ends;
		outputstream.close();
	} else {
		result  = -1;
	}

	return result ;
}

} //glite
}//wms
}//wmproxyapi
}//examples
}//utilities
