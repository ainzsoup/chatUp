#include "server.hpp"

Client::Client(char *ip, char *port) : _ip(ip), _port(port)
{
}

Client::Client()
{
}

Client::~Client()
{
}

void Client::connectToServer()
{
	// configuring remote address
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	struct addrinfo *server_address;
	if (getaddrinfo(_ip, _port, &hints, &server_address)) 
		throw std::runtime_error("getaddrinfo() failed");
	// creating scoket
	_socket = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol);
	if (_socket < 0) 
		throw std::runtime_error("socket() failed");
	// connecting to server
	if (connect(_socket, server_address->ai_addr, server_address->ai_addrlen)) 
	{
		throw std::runtime_error("connect() failed");
	}
 	freeaddrinfo(server_address);
	std::cout << "\033[32m Connection established\nYou're in :) \033[0m" << std::endl;
	
	FD_ZERO(&_sets[READ]);
 	FD_SET(_socket, &_sets[READ]);
	FD_SET(0, &_sets[READ]);

}

void Client::getReadyDescriptors(int timeout_sec, int timeout_usec)
{
	struct timeval timeout;
	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = timeout_usec;

	if (select(_socket + 1, &_sets[READ], 0, 0, &timeout) < 0) 
		throw std::runtime_error("select() failed");
}

const int &Client::getSocket() const
{
	return _socket;
}

const fd_set &Client::getSets(int set) const
{
	return _sets[set];
}

void Client::sendMessage()
{
	std::string message;
	std::getline(std::cin, message);
	if (send(_socket, message.c_str(), message.size() + 1, 0) < 0)
		throw std::runtime_error("send() failed");
}

void Client::receiveMessage()
{
	char message[4096];
	memset(message, 0, 4096);
	int bytesReceived = recv(_socket, message, 4096, 0);
	if (bytesReceived < 0)
		throw std::runtime_error("recv() failed");
	std::cout << "\033[32m" << message << "\033[0m" << std::endl;
}

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: " << av[0] << "<remote address> <port>" << std::endl; 
		return 1;
	}
	Client client(av[1], av[2]);
	try 
	{
		client.connectToServer();
		while(true)
		{
			if (FD_ISSET(client.getSocket(), &client.getSets(READ)))
			{
				
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
