#include "../src/socket++/socketClient_cu_suite.h"

void socketClient_test::setUp()
{
	srvListeningPort = 55557;
	return;
}

void socketClient_test::tearDown()
{
	//if ( c ) delete c;
	return;
}

void socketClient_test::SendReceive_case()
{
	srv = new socket_pp::SocketServer(srvListeningPort);
	srv->Open();
	pthread_create(&thr,NULL,&thrLoop,srv);
	socket_pp::SocketClient c("localhost",srvListeningPort);
	CPPUNIT_ASSERT(c.Open());
	string msg = "Test 1";
	CPPUNIT_ASSERT(c.Send(msg));
	string answer;
	CPPUNIT_ASSERT(c.Receive(answer));
	CPPUNIT_ASSERT_MESSAGE("Failure testing 'string' case", 
			answer == "Test 1");
	
	int msgInt = INT_MAX;
	CPPUNIT_ASSERT(c.Send(msgInt));
	int answerInt;
	CPPUNIT_ASSERT(c.Receive(answerInt));
	CPPUNIT_ASSERT_MESSAGE("Failure testing 'int' case", 
			answerInt == INT_MAX);
	
	long msgLong = LONG_MAX;
	CPPUNIT_ASSERT(c.Send(msgLong));
	long answerLong;
	CPPUNIT_ASSERT(c.Receive(answerLong));
	CPPUNIT_ASSERT_MESSAGE("Failure testing 'long' case", 
			answerLong == LONG_MAX);

	CPPUNIT_ASSERT(c.Close());
	pthread_join (thr,NULL);
	return;
}

void socketClient_test::OpenFailure_case()
{
	 	CPPUNIT_ASSERT_NO_THROW(
				c = new socket_pp::SocketClient("NotAValidHost",
					srvListeningPort)
				);
		CPPUNIT_ASSERT_THROW(
				c->Open(),
				socket_pp::IOException); //This Throws
}


void* thrLoop(void* param)
{
	try{
	socket_pp::SocketAgent* a;
	a = ((socket_pp::SocketServer *)param)->Listen();
	a -> SetTimeout(5);
	std::string message;
	msgSendReceive(message,a);
	int messageInt;
	msgSendReceive(messageInt,a);
	long messageLong;
	msgSendReceive(messageLong,a);
	((socket_pp::SocketServer *)param)->KillAgent(a);
	((socket_pp::SocketServer *)param)->Close();
	}
	catch( std::exception& e) {
		std::cout << e.what() << std::endl;
	}	
	return NULL;
}

