#include "server.hpp"
#include <unistd.h>


Server::Server()
{
}

Server::~Server()
{
}

void Server::setupSocket(char *port)
{
	//configuring local address
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // listen on socket
	
	struct addrinfo *bind_address;
	int slience = getaddrinfo(0, port, &hints, &bind_address); // just to silence unused variable warning

	//creatng socket
 	_socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (_socket_listen < 0) 
		throw std::runtime_error("socket() failed");

	//binding socket to local bind_address
	if (bind(_socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) 
		throw std::runtime_error("bind() failed");
	freeaddrinfo(bind_address);

	//listening
	if (listen(_socket_listen, 10) < 0) 
		throw std::runtime_error("listen() failed");

	FD_ZERO(&_sets[MASTER]);
	FD_SET(_socket_listen, &_sets[MASTER]);
	FD_SET(0, &_sets[MASTER]);
	_max_socket = _socket_listen;
}

void Server::getReadyDescriptors(int timeout_sec, int timeout_usec)
{
	_sets[READ] = _sets[MASTER];
	_sets[WRITE] = _sets[MASTER];
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = timeout_usec;

	if (select(_max_socket + 1, &_sets[READ], &_sets[WRITE], 0, &tv) < 0) 
		throw std::runtime_error("select() failed");
}

const int &Server::getSocketListen() const
{
	return _socket_listen;
}

const int &Server::getMaxSocket() const
{
	return _max_socket;
} 

const fd_set &Server::getSets(int i) const
{
	return _sets[i];
}

void Server::sendWelcomeMessage(int i)
{
	std::string msg = "Welcome to the chat!\n Please enter your name:";
	send(i, msg.c_str(), msg.size() + 1, 0);
}


void Server::acceptConnection()
{
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	int socket_client = accept(_socket_listen, (struct sockaddr*) &client_address, &client_len);
	if (socket_client < 0) 
		throw std::runtime_error("accept() failed");
	FD_SET(socket_client, &_sets[MASTER]);
	if (socket_client > _max_socket)
		_max_socket = socket_client;
	char address_buffer[100];
	getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	std::cout << "New connection from " << address_buffer << std::endl;
	Client new_client(socket_client);
	_clients[socket_client] = new_client;
	sendWelcomeMessage(socket_client);
}

void Server::receiveMessage(int sender)
{
	char read[1024];
	int bytes_received = recv(sender, read, 1024, 0);
	if (bytes_received < 1)
	{
		close(sender);
		FD_CLR(sender, &_sets[MASTER]);
		_clients.erase(sender);
		return;
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	read[bytes_received] = '\0';
	if (bytes_received > 0 && read[bytes_received - 1] == '\n')
			read[bytes_received - 1] = '\0';
	std::cout << "\033[32m" << _clients[sender].getName() << ": " << read << "\033[0m" << std::endl;
	broadcastMessage(sender, read, bytes_received);
}

void Server::broadcastMessage(int sender, char *read, int bytes_received)
{
	char message[2048];
    sprintf(message, "\033[32m%s: %s\033[0m\n", _clients[sender].getName().c_str(), read);
	for (int j = 1; j <= _max_socket; ++j)
	{
		if (FD_ISSET(j, &_sets[MASTER]))
		{
			if (j == _socket_listen || j == sender) // don't send to listener and sender
				continue;
			else
				send(j, message, strlen(message), 0);
		}
	}
}

void Server::sendMessage(char *name)
{
	char message[1024];
	std::cin.getline(message, 1024);
	char formatted_message[2048];
	sprintf(formatted_message, "%s: %s\n", name, message);
	for (int j = 1; j <= _max_socket; ++j)
	{
		if (FD_ISSET(j, &_sets[WRITE]))
			if (j != _socket_listen)
			{
				int begin = 0;
				int bytes_sent = 0;
				while (bytes_sent < strlen(formatted_message))
				{
					bytes_sent = send(j, formatted_message + begin, strlen(formatted_message) - begin, 0);
					begin += bytes_sent;
				}
			}
	}
}

void Server::announce(std::string msg)
{
	char message[2048];
	sprintf(message, "\033[37m%s\033[0m\n", msg.c_str()); //gray 
	for (int j = 1; j <= _max_socket; ++j)
	{
		if (FD_ISSET(j, &_sets[WRITE]))
			if (j != _socket_listen)
			{
				int begin = 0;
				int bytes_sent = 0;
				while (bytes_sent < strlen(message))
				{
					bytes_sent = send(j, message + begin, strlen(message) - begin, 0);
					begin += bytes_sent;
				}
			}
	}
}

int Server::getClientName(int client)
{
	char name[1024];
	int bytes_received = recv(client, name, 1024, 0);
	if (bytes_received < 1)
	{
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
		return 0;
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	name[bytes_received] = '\0';
	if (bytes_received > 0 && name[bytes_received - 1] == '\n')
			name[bytes_received - 1] = '\0';
	_clients[client].setName(name);
	std::cout << "\033[32m" << _clients[client].getName() << " has joined the chat!\033[0m" << std::endl;
	return 1;
}

void Server::handleClient(int client)
{
	switch (_clients[client].getStatus())
	{
		case CONNECTED:
			receiveMessage(client);
			break;
		case EXPECTING_NAME:
			if (getClientName(client))
			{
				_clients[client].setStatus(CONNECTED);
				send(client, "Welcome to the chat!\n", 21, 0);
				

			}

			break;
	}
}

int main(int ac, char **av)
{
	if (ac < 3)
	{
		std::cerr << "Usage: " << av[0] << " port name" << std::endl;
		return 1;
	}
	Server server;
	try
	{
		server.setupSocket(av[1]);
		std::cout << "waiting for connections..." << std::endl;	
		while(true)
		{
			usleep(100);
			server.getReadyDescriptors(0, 0);
			for (int i = 0; i <= server.getMaxSocket(); ++i)
			{
				if (FD_ISSET(i, &server.getSets(READ)))
				{
					if (i == server.getSocketListen()) // new connection
						server.acceptConnection();
					else if (i == STDIN_FILENO) // server input
						server.sendMessage(av[2]);
					else // client message
						server.handleClient(i);
				}
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "quitting..." << std::endl;
	close(server.getSocketListen());
	return 0;
}


