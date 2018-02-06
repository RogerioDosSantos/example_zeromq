
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "../../../../third-party/cppzmq/zmq.hpp"

bool RunServer()
{
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	std::string uri = "tcp://*:9999";
	socket.bind(uri.c_str());
	printf("\nServer\n\tURL: tcp://localhost:9999\n\n");

	std::string request;
	while (request.compare("exit"))
	{
		printf("\tWaiting for Client request ...");
    fflush(stdout);

		zmq::message_t request_message;
		socket.recv(&request_message);
		printf("done.\n");
		printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(request_message.size()), static_cast<const char*>(request_message.data()));
		request.assign(static_cast<const char*>(request_message.data()), request_message.size()-1);

		printf("\tSending reply:\n");

		std::string server_reply("Server Reply to the request: ");
		server_reply += request;

		zmq::message_t reply_message((void*)server_reply.c_str(), server_reply.size()+1);
		printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(reply_message.size()), static_cast<const char*>(reply_message.data()));
		socket.send(reply_message);
	}

	printf("Stopping Server.\n");
	return true;
}

bool RunClient()
{
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	std::string uri = "tcp://localhost:9999";
	socket.connect(uri.c_str());
	printf("\nClient\n\tURL: tcp://localhost:9999\n\n");

	std::string request;
	while (request.compare("exit"))
	{
		printf("\tSending request:\n");
		printf("\t\tEnter a request or 'exit' to stop both Server and Client: ");
    std::getline(std::cin, request);

		zmq::message_t request_message(request.c_str(), request.size()+1);
    printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(request_message.size()), static_cast<const char*>(request_message.data()));
		socket.send(request_message);

		printf("\tWaiting for Server reply ...");
    fflush(stdout);

		zmq::message_t reply_message;
    socket.recv(&reply_message);
		printf("done.\n");
    printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(reply_message.size()), static_cast<const char*>(reply_message.data()));
	}

	printf("Stopping Client.\n");
  socket.close();
	return true;
}

bool RunPublisher()
{
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PUB);
	std::string uri = "tcp://*:9998";
	socket.bind(uri.c_str());
	printf("\nPublisher\n\tURL: tcp://localhost:9998\n\n");

	std::string value_to_publish;
	while (value_to_publish.compare("exit"))
	{
		printf("\tPublishing Values:\n");
		printf("\t\tEnter item to be published or 'exit' to stop the publisher and the subscribers: ");
    std::getline(std::cin, value_to_publish);

		zmq::message_t message((void*)value_to_publish.c_str(), value_to_publish.size()+1);
    printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(message.size()), static_cast<const char*>(message.data()));
		socket.send(message);
	}

	printf("Stopping publisher.\n");
	socket.close();
	return true;
}

bool RunSubscriber()
{
	std::string uri = "tcp://localhost:9998";
	printf("\nSubscriber\n\tConnecting to URL: tcp://localhost:9998\n\n");

	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_SUB);
	socket.connect(uri.c_str());

	// std::string filter("M1");  // You can use empty string to receive all messages types.
	std::string filter("");  // You can use empty string to receive all messages types.
	socket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());

	std::string received_value;
	while (received_value.compare("exit"))
	{
		printf("\tWaiting to receive message (filter: %s) from publisher ...", filter.c_str());
    fflush(stdout);
		zmq::message_t message;
		socket.recv(&message, 0);
		printf("done.\n");
    printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(message.size()), static_cast<const char*>(message.data()));
		received_value.assign((const char*)message.data(), message.size()-1);
	}

	printf("Stopping subscriber.\n");
	socket.close();
	return true;
}

int main(int argc, char const* argv[])
{
	std::string argument;
	switch (argc)
	{
	case 2:
		argument = argv[1];
		break;
	default:
		break;
	}

	if (!argument.compare("server"))
    RunServer();
	else if (!argument.compare("client"))
    RunClient();
	else if (!argument.compare("publisher"))
		RunPublisher();
	else if (!argument.compare("subscriber"))
		RunSubscriber();
	else
		std::cout << "Invalid Command (" << argc << "):\n\t" << argument << "\n Options:\n\tpublisher\n\tsubscriber\n";

	return 0;
}
