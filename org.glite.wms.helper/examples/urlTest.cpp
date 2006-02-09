/***************************************************************************
 *  Filename  : urlTest.cpp
 *  Authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
                Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright : (C) 2001 by INFN
 ***************************************************************************/

#include <iostream>
#include "jobadapter/URL.h"

using std::cout;
using std::cerr;
using std::endl;

using namespace glite::wms::helper::jobadapter::url;

int
main(void)
{

  {
    URL* url;
    
    cout << "Testing class URL..." << '\n';
    cout << "\nLet's start with an easy one: http://www.google.com\n";
    try {
      url = new URL("http://www.google.com");
      cout << "protocol = " << url->protocol() << '\n'
	      << "host =     " << url->host()     << '\n'
	      << "port =     " << url->port()     << '\n'
	      << "path =     " << url->path()     << '\n';
      cout << url->as_string() << '\n';
    } catch (ExInvalidURL& ex) {
      cerr << ex.parameter() << '\n';
    } catch (...) {
      cerr << "Caught uknown exception" << endl;
    }

    delete url;

    cout << "Testing an empty URL..." << endl;
    try {
      URL url("");
    } catch (ExInvalidURL& ex) {
      cerr << "Empty URL cause an exception, uhm ...right\n\n";
    }
  }

  {
    cout << "Some example..." << endl;

    const std::string url("http://www.ics.uci.edu:34535/a/b/eefef/qff%20df");
    const std::string wrong_prot_url("h ttp://www.ics.uci.edu");
    const std::string wrong_host_url("http://www.ic s.uci.edu");
    const std::string url_path("http://www.ics.uci.edu:8080/pub/ietf/uri/");
    const std::string url_path_pct("http://www.ics.uci.edu:8080/pub/ietf/u%20ri/");
    const std::string url_path_wrong_pct("http://www.ics.uci.edu:8080/pub/ietf/u%2ri/");
    const std::string url_path2("http://www.ics.uci.edu:8080/pub/ietf/uri");
    const std::string wrong_url_path("http://www.ics.uci.edu:8080/pub/i:etf/uri");

    try {
      URL url_(url);
      cout << url << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << url << " = not valid URL\n";
    }
    try {
      URL wrong_prot_url_(wrong_prot_url);
      cout << wrong_prot_url << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << wrong_prot_url << " = not valid URL\n";
    }
    try {
      URL wrong_host_url_(wrong_host_url);
      cout << wrong_host_url << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << wrong_host_url << " = not valid URL\n";
    }
    try {
      URL url_path_(url_path);
      cout << url_path << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << url_path << " = not valid URL\n";
    }
    try {
      URL url_path_pct_(url_path_pct);
      cout << url_path_pct << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << url_path_pct << " = not valid URL\n";
    }
    try {
      URL url_path_wrong_pct_(url_path_wrong_pct);
      cout << url_path_wrong_pct << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << url_path_wrong_pct << " = not valid URL\n";
    }
    try {
      URL url_path2_(url_path2);
      cout << url_path2 << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << url_path2 << " = not valid URL\n";
    }
    try {
      URL wrong_url_path_(wrong_url_path);
      cout << wrong_url_path << " = valid URL\n";
    } catch (ExInvalidURL& ex) {
      cout << wrong_url_path << " = not valid URL\n";
    }
  }

  {
    cout << "\nTesting copy constructor..." << endl;
    URL* url1;
    
    try {
      url1 = new URL("http://www.cnaf.infn.it/giaco");
    } catch (ExInvalidURL& ex) {
      cerr << "Invalid URL" << ex.parameter() << endl;
    } catch (...) {
      cerr << "Caught uknown exception" << endl;
    }
    
    {
      URL url2(*url1);
      
      cout << "url1 = " << url1->as_string() << endl
	   << "url2 = " << url2.as_string() << endl << endl;

      cout << "url1 protocol = " << url1->protocol() << endl
	   << "url2 protocol = " << url2.protocol() << endl << endl;
      cout << "url1 host = " << url1->host() << endl
	   << "url2 host = " << url2.host() << endl << endl;
      cout << "url1 port = " << url1->port() << endl
	   << "url2 port = " << url2.port() << endl << endl;
      cout << "url1 path = " << url1->path() << endl
	   << "url2 path = " << url2.path() << endl << endl;
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
    } catch (...) {
      cerr << "Caught uknown exception" << endl;
    }
    
    {
      URL url2 = *url1;

      cout << "url1 = " << url1->as_string() << endl
	   << "url2 = " << url2.as_string() << endl << endl;

      cout << "url1 protocol = " << url1->protocol() << endl
	   << "url2 protocol = " << url2.protocol() << endl << endl;
      cout << "url1 host = " << url1->host() << endl
	   << "url2 host = " << url2.host() << endl << endl;
      cout << "url1 port = " << url1->port() << endl
	   << "url2 port = " << url2.port() << endl << endl;
      cout << "url1 path = " << url1->path() << endl
	   << "url2 path = " << url2.path() << endl << endl;
    }

    delete url1;
  }

  return 0;
}
