#include <iostream>
#include <string>
#include <pthread.h>
#include <cppunit/extensions/HelperMacros.h>

#include "glite/wms/tls/socket++/SocketServer.h" 
#include "glite/wms/tls/socket++/SocketClient.h"
#include "glite/wms/tls/socket++/SocketAgent.h"
#include "socket++/exceptions.h"

namespace socket_pp = glite::wms::tls::socket_pp;

using namespace std;

class socketClient_test : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(socketClient_test);
	//CPPUNIT_TEST(constructor_case); 
	CPPUNIT_TEST(SendReceive_case); 
	CPPUNIT_TEST(OpenFailure_case); 
	//CPPUNIT_TEST(SendString_case); 
	//CPPUNIT_TEST(ReceiveInt_case); 
	//CPPUNIT_TEST(ReceiveLong_case); 
	//CPPUNIT_TEST(ReceiveString_case); 
	//CPPUNIT_TEST(Open_case); 
	//CPPUNIT_TEST(Close_case); 
	//CPPUNIT_TEST(SetTimeout_case); 
	//CPPUNIT_TEST(Host_case); 
	CPPUNIT_TEST_SUITE_END();
private:
	int srvListeningPort;
	pthread_t thr;
	socket_pp::SocketServer *srv;
	socket_pp::SocketClient *c;
public:

	void setUp();
	void tearDown();
	void constructor_case();
	void SendReceive_case();
	void OpenFailure_case();
	void SendReceiveFailure_case();
	void SendLong_case();
	void SendString_case();
	void ReceiveInt_case();
	void ReceiveLong_case();
	void ReceiveString_case();
	void Open_case();
	void Close_case();
	void SetTimeout_case();
	void Host_case();

	friend void* thrLoop(void *);
	

};

template <class Type>
        void msgSendReceive ( Type message, socket_pp::SocketAgent* a)
{
	a -> Receive(message);
        a -> Send(message);
 	return;	 
}

