/***************************************************************************
 *  filename  : urlTest.cpp
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *  copyright : (C) 2001 by INFN
 ***************************************************************************/

#include <iostream>

#include "jobadapter/url/URL.h"

using std::cout;
using std::cerr;
using std::endl;

using namespace glite::wms::helper::jobadapter::url;

int
main(int argc, char* argv[])
{
  if (argc == 2) {
    cout << "Testing URL with argv[1]..." << endl;
      
    URL* url;
    
    try {
      url = new URL(argv[1]);
    } catch (ExInvalidURL& ex) {
      cerr << "Invalid URL" << ex.parameter() << endl;
      return -1;
    } catch (...) {
      cerr << "Caught uknown exception" << endl;
      return -2;
    }

    cout << "protocol = " << url->protocol() << endl
	 << "host =     " << url->host()     << endl
	 << "port =     " << url->port()     << endl
	 << "path =     " << url->path()     << endl;

    cout << url->as_string() << endl << endl;
    
    delete url;
  }
  
  {
    cout << "Testing an empty URL..." << endl;
    
    URL url;

    cout << "URL is empty = " << url.is_empty() << endl;
    cout << "protocol = " << url.protocol() << endl
	 << "host =     " << url.host()     << endl
	 << "port =     " << url.port()     << endl
	 << "path =     " << url.path()     << endl;

    cout << endl;
  }

  {
    cout << "Testing copy constructor..." << endl;
    URL* url1;
    
    try {
      url1 = new URL("http://www.cnaf.infn.it/giaco");
    } catch (ExInvalidURL& ex) {
      cerr << "Invalid URL" << ex.parameter() << endl;
      return -1;
    } catch (...) {
      cerr << "Caught uknown exception" << endl;
      return -2;
    }
    
    {
      URL url2(*url1);
      
      cout << "url1 = " << url1->as_string() << endl
	   << "url2 = " << url2.as_string() << endl << endl;
    }
    
    {
      URL empty_url;
      URL url2(empty_url);
      
      cout << "empty_url = "
	   << (empty_url.is_empty() ? "empty" : empty_url.as_string())
	   << endl
	   << "url2      = "
	   << (url2.is_empty() ? "empty" : url2.as_string())
	   << endl << endl;
    }
    
    delete url1;
  }

  {
    cout << "Testing assignment operator..." << endl;
    URL* url1;

    try {
      url1 = new URL("http://www.cnaf.infn.it/giaco");
    } catch (ExInvalidURL& ex) {
      cerr << "Invalid URL" << ex.parameter() << endl;
      return -1;
    } catch (...) {
      cerr << "Caught uknown exception" << endl;
      return -2;
    }
    
    {
      URL url2 = *url1;

      cout << "url1 = " << url1->as_string() << endl
	   << "url2 = " << url2.as_string() << endl << endl;
    }

    {
      URL empty_url;
      URL url2(empty_url);
      
      cout << "empty_url = "
	   << (empty_url.is_empty() ? "empty" : empty_url.as_string())
	   << endl
	   << "url2      = "
	   << (url2.is_empty() ? "empty" : url2.as_string())
	   << endl << endl;
    }
    
    delete url1;
  }

  return 0;
}
