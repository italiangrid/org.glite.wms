#include "glite/wmsutils/tls/socket++/GSISocketServer.h"
#include "glite/wmsutils/tls/socket++/GSISocketAgent.h"
#include "socket++/exceptions.h"

#include <iostream>
#include <string>

namespace socket_pp = glite::wmsutils::tls::socket_pp;

int main(int argc, char* argv[])
{
int res=0;
if ( argc < 1 )
{
	std::cerr << "You must specify teh listening port!" << std::endl;
	exit(4);
}
try {
	socket_pp::GSISocketServer srv(atoi(argv[1]));
	srv.Open();
	try {
		socket_pp::GSISocketAgent* a = srv.Listen();
		std::string message;
		a -> SetTimeout(5);
		a -> Receive(message);
		std::cout << message << std::endl;
		std::string answer;
		if ( message != "Hello World!" )
		{
			answer = "ko";
			res=1;
		}
		else
		{	
			answer = "ok";
		}
		std::cout << answer << std::endl;
			a -> Send(answer);
	}
	catch( std::exception& e) {
	        std::cout << e.what() << std::endl;
		return 1;
	}
} 
catch( std::exception& e) { 
	std::cout << e.what() << std::endl; 
	return 2;
} 
if ( res != 0 )	
{
 return 3;
}
else
{
	return 0;
}
}
