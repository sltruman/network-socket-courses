// signaling-client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//// client-test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "signaling_client.h"

#include <fstream>


int main()
{
	ofstream log;
	log.open("../x64/Release/signaling-client.log", ios::out);
	
	ostream out(cout.rdbuf(log.rdbuf()));
	
	signaling_client io;
	string command;

	do {
		getline(cin,command);
		if (command == "getRouterRtpCapabilities") {
			out << io.get_router_rtp_capabilities() << endl;
		}
		else if (command == "createProducerTransport") {
			string device_rtp_capabilities;
			getline(cin, device_rtp_capabilities);
			out << io.create_producer_transport(device_rtp_capabilities) << endl;
		}
		else if (command == "connectProducerTransport") {
			string dtls_parameters;
			getline(cin, dtls_parameters);
			io.connect_producer_transport(dtls_parameters);
			out << "true" << endl;
		}
		else if (command == "produce") {
			string args;
			getline(cin, args);
			out << io.produce(args) << endl;
		}
		else if (command == "createConsumerTransport") {
			string args;
			getline(cin, args);
			out << io.create_consumer_transport(args) << endl;
		}
		else if (command == "connectConsumerTransport") {
			string args;
			getline(cin, args);
			io.connect_consumer_transport(args);
			out << "true" << endl;
		}
		else if (command == "consume") {
			string args;
			getline(cin, args);
			out << io.consume(args) << endl;
		}
		else if (command == "resume") {
			io.resume();
			out << "true" << endl;
		}
	} while (command != "exit");

	log.close();

	return 0;
}
