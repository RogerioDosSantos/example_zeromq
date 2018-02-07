
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "../../../../third-party/cppzmq/zmq.hpp"

bool RunServer(const char* port)
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    std::string uri = "tcp://*:";
    uri += port;
    socket.bind(uri.c_str());
    printf("\nServer\n\tURI: tcp://localhost:%s\n\n", port);

    std::string request;
    while (request.compare("exit"))
    {
        printf("\tWaiting for Client request ...");
        fflush(stdout);

        zmq::message_t request_message;
        socket.recv(&request_message);
        printf("done.\n");
        printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(request_message.size()), static_cast<const char*>(request_message.data()));
        request.assign(static_cast<const char*>(request_message.data()), request_message.size() - 1);

        printf("\tSending reply:\n");

        std::string server_reply("Server Reply to the request: ");
        server_reply += request;

        zmq::message_t reply_message((void*)server_reply.c_str(), server_reply.size() + 1);
        printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(reply_message.size()), static_cast<const char*>(reply_message.data()));
        socket.send(reply_message);
    }

    printf("Stopping Server.\n");
    return true;
}

bool RunClient(const char* uri)
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect(uri);
    printf("\nClient\n\tServer URI: %s\n\n", uri);

    std::string request;
    while (request.compare("exit"))
    {
        printf("\tSending request:\n");
        printf("\t\tEnter a request or 'exit' to stop both Server and Client: ");
        std::getline(std::cin, request);

        zmq::message_t request_message(request.c_str(), request.size() + 1);
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

bool RunPublisher(const char* port)
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);
    std::string uri = "tcp://*:";
    uri += port;
    socket.bind(uri.c_str());
    printf("\nPublisher\n\tURI: tcp://localhost:%s\n\n", port);

    std::string value_to_publish;
    while (value_to_publish.compare("exit"))
    {
        printf("\tPublishing Values:\n");
        printf("\t\tEnter item to be published or 'exit' to stop the publisher and the subscribers: ");
        std::getline(std::cin, value_to_publish);

        zmq::message_t message((void*)value_to_publish.c_str(), value_to_publish.size() + 1);
        printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(message.size()), static_cast<const char*>(message.data()));
        socket.send(message);
    }

    printf("Stopping publisher.\n");
    socket.close();
    return true;
}

bool RunSubscriber(const char* uri)
{
    printf("\nSubscriber\n\tConnecting to URI: %s\n\n", uri);

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect(uri);

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
        received_value.assign((const char*)message.data(), message.size() - 1);
    }

    printf("Stopping subscriber.\n");
    socket.close();
    return true;
}

bool RunNonWaitingSubscriber(const char* uri)
{
    printf("\nNon Waiting Subscriber\n\tConnecting to URI: %s\n\n", uri);

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect(uri);

    // std::string filter("M1");  // You can use empty string to receive all messages types.
    std::string filter("");  // You can use empty string to receive all messages types.
    socket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());

    std::string received_value;
    while (received_value.compare("exit"))
    {
        printf("\t Evaluating if message is received (filter: %s) from publisher ...", filter.c_str());
        fflush(stdout);

        zmq::message_t message;
        if (socket.recv(&message, ZMQ_DONTWAIT))
        {
            printf("done.\n");
            printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(message.size()), static_cast<const char*>(message.data()));
            received_value.assign((const char*)message.data(), message.size() - 1);
            continue;
        }

        printf("not done. Sleeping for 1 second\n");
        usleep(1 * 1000 * 1000);
    }

    printf("Stopping subscriber.\n");
    socket.close();
    return true;
}

bool RunServerPublisher(const char* server_port, const char* publisher_port)
{
    zmq::context_t context(1);
    zmq::socket_t server_socket(context, ZMQ_REP);
    zmq::socket_t publisher_socket(context, ZMQ_PUB);
    std::string server_uri("tcp://*:");
    std::string publisher_uri("tcp://*:");
    server_uri += server_port;
    publisher_uri += publisher_port;
    server_socket.bind(server_uri.c_str());
    publisher_socket.bind(publisher_uri.c_str());
    printf("\nServer Publisher\n\tServer URL: tcp://localhost:%s\n\tPublisher URL: tcp://localhost:%s\n\n", server_port, publisher_port);

    std::string value_to_publish("None - Send the Value using a Client");
    while (value_to_publish.compare("exit"))
    {
        printf("\tVerifying for Client request ...");
        fflush(stdout);

        zmq::message_t request_message;
        if (server_socket.recv(&request_message, ZMQ_DONTWAIT))
        {
            printf("done.\n");
            printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(request_message.size()), static_cast<const char*>(request_message.data()));
            value_to_publish.assign(static_cast<const char*>(request_message.data()), request_message.size() - 1);

            printf("\tSending reply:\n");

            std::string server_reply("Set new value to publish: ");
            server_reply += value_to_publish;

            zmq::message_t reply_message((void*)server_reply.c_str(), server_reply.size() + 1);
            printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(reply_message.size()), static_cast<const char*>(reply_message.data()));
            server_socket.send(reply_message);
        }
        else
        {
            printf("done. No new message. Sleeping for 1 second.\n");
            usleep(1 * 1000 * 1000);
        }

        printf("\tPublishing Value: %s\n", value_to_publish.c_str());

        zmq::message_t publisher_message((void*)value_to_publish.c_str(), value_to_publish.size() + 1);
        printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(publisher_message.size()), static_cast<const char*>(publisher_message.data()));
        publisher_socket.send(publisher_message);
    }

    printf("Stopping Server Publisher.\n");
    server_socket.close();
    publisher_socket.close();
    return true;
}

bool RunMultiSubscriber(const char* uri_1, const char* uri_2)
{
    printf("\nMulti-Subscriber\n\tConnecting to following servers:\n\t\t1: %s\n\t\t2: %s\n\n", uri_1, uri_2);

    zmq::context_t context(1);
    zmq::socket_t socket_1(context, ZMQ_SUB);
    socket_1.connect(uri_1);
    socket_1.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::socket_t socket_2(context, ZMQ_SUB);
    socket_2.connect(uri_2);
    socket_2.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::pollitem_t sockets[] = {{socket_1, 0, ZMQ_POLLIN, 0}, {socket_2, 0, ZMQ_POLLIN, 0}};

    std::string received_value;
    while (received_value.compare("exit"))
    {
        printf("\tWaiting messages from both servers ...");
        fflush(stdout);

        zmq::message_t message;
        zmq::poll(&sockets[0], 2, -1);
        printf("done.\n");
        if (sockets[0].revents & ZMQ_POLLIN)
        {
            printf("\t\tGetting message from Server: %s ...", uri_1);
            fflush(stdout);
            socket_1.recv(&message);
            printf("done.\n");
            printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(message.size()), static_cast<const char*>(message.data()));
            received_value.assign((const char*)message.data(), message.size() - 1);
            continue;
        }

        if (sockets[1].revents & ZMQ_POLLIN)
        {
            printf("\t\tGetting message from Server: %s ...", uri_2);
            fflush(stdout);
            socket_2.recv(&message);
            printf("done.\n");
            printf("\t\tMessage: Size: %d ; Value: %s\n", static_cast<int>(message.size()), static_cast<const char*>(message.data()));
            received_value.assign((const char*)message.data(), message.size() - 1);
            continue;
        }
    }

    printf("Stopping subscribers.\n");
    socket_1.close();
    socket_2.close();
    return true;
}

int main(int argc, char const* argv[])
{
    std::string arguments[3];
    switch (argc)
    {
    case 4:
        arguments[2] = argv[3];
    case 3:
        arguments[1] = argv[2];
    case 2:
        arguments[0] = argv[1];
        break;
    default:
        break;
    }

    if (!arguments[0].compare("server"))
        RunServer(arguments[1].c_str());
    else if (!arguments[0].compare("client"))
        RunClient(arguments[1].c_str());
    else if (!arguments[0].compare("publisher"))
        RunPublisher(arguments[1].c_str());
    else if (!arguments[0].compare("subscriber"))
        RunSubscriber(arguments[1].c_str());
    else if (!arguments[0].compare("non_waiting_subscriber"))
        RunNonWaitingSubscriber(arguments[1].c_str());
    else if (!arguments[0].compare("server_publisher"))
        RunServerPublisher(arguments[1].c_str(), arguments[2].c_str());
    else if (!arguments[0].compare("multi_subscriber"))
        RunMultiSubscriber(arguments[1].c_str(), arguments[2].c_str());
    else
        std::cout << "Invalid Command (" << argc << "):\n\t" << arguments[0] << "\nOptions:\n"
                  << "\tserver <server_port>\t\t\t\t\t-E.g.: server 9999\n"
                  << "\tclient <server_uri>\t\t\t\t\t-E.g.: client tcp://localhost:9999\n"
                  << "\tpublisher <publishe_port>\t\t\t\t-E.g.: publisher 9999\n"
                  << "\tsubscriber <publisher_uri>\t\t\t\t-E.g.: subscriber tcp://localhost:9999\n"
                  << "\tnon_waiting_subscriber <publisher_uri>\t\t\t-E.g.: non_waiting_subscriber tcp://localhost:9999\n"
                  << "\tserver_publisher <server_port> <publisher_port>\t\t-E.g.: server_publisher 9999 9998\n"
                  << "\tmulti_subscriber <server_uri1> <server_uri2>\t\t-E.g.: multi_subscriber tcp://localhost:9999 tcp://localhost:9998\n"
                  << "\n";

    return 0;
}
