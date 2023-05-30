#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


void broadcast (char *message, int message_length, int sender, fd_set *master, int max_socket)
{
	std::cout << message << std::endl;
	for (int i = 1; i <= max_socket; ++i)
	{
		if (FD_ISSET(i, master))
		{
			if (i == sender)
				continue;
			else
				send(i, message, message_length, 0);
		}
	}
}

int main (int ac, char **av) 
{
	if (ac < 2 )
	{
		std::cerr << "Usage: " << av[0] << " port" << std::endl;
		return 1;
	}
	std::cout << "configuring local address..." << std::endl;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // listen on socket
	
	struct addrinfo *bind_address;
	getaddrinfo(0, av[1], &hints, &bind_address);

 	std::cout << "creating socket..." << std::endl;
 	int socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (socket_listen < 0) 
	{
		std::cerr << "socket() failed. (" << errno << ")" << std::endl;
		return 1;
	}


	std::cout << "binding socket to local address..." << std::endl;
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) 
	{
		std::cerr << "bind() failed. (" << errno << ")" << std::endl;
		return 1;
	}
	freeaddrinfo(bind_address);

	std::cout << "listening..." << std::endl;
	if (listen(socket_listen, 10) < 0) 
	{
		std::cerr << "listen() failed. (" << errno << ")" << std::endl;
		return 1;
	}

	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	int max_socket = socket_listen;

	std::cout << "waiting for connections..." << std::endl;
	while (true)
	{
		fd_set reads;
		fd_set writes;
		reads = master;
		writes = master;
		if (select(max_socket+1, &reads, &writes, 0, 0) < 0) 
		{
			std::cerr << "select() failed. (" << errno << ")" << std::endl;
			return 1;
		}
		for (int i = 1; i <= max_socket; ++i)
		{
			if (FD_ISSET(i, &reads))
			{
				if (i == socket_listen)
				{
					struct sockaddr_storage client_address;
					socklen_t client_len = sizeof(client_address);
					int socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
					if (socket_client < 0) 
					{
						std::cerr << "accept() failed. (" << errno << ")" << std::endl;
						return 1;
					}
					FD_SET(socket_client, &master);
					if (socket_client > max_socket)
						max_socket = socket_client;
					char address_buffer[100];
					getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
					std::cout << "new connection from " << address_buffer << std::endl;
				}
				else
				{
					char message[1024];
					int bytes_received = recv(i, message, 1024, 0);
					if (bytes_received < 1)
					{
						FD_CLR(i, &master);
						close(i);
						continue;
					}
					broadcast(message, bytes_received, i, &master, max_socket);
					for (int j = 1; j <= max_socket; ++j)
					{
						if (FD_ISSET(j, &master))
						{
							if (j == socket_listen || j == i)
								continue;
							else
								send(j, message, bytes_received, 0);
						}
					}
				}
			}
		}
		// for (int i = 1; i <= max_socket; ++i)
		// {
		// 	if (FD_ISSET(i, &writes))
		// 	{
		// 		if (i == socket_listen)
		// 			continue;
		// 		else
		// 		{
		// 			char message[1024];
		// 			std::cin.getline(message, 1024);
		// 			int bytes_sent = send(i, message, strlen(message), 0);
		// 			if (bytes_sent < 1)
		// 			{
		// 				FD_CLR(i, &master);
		// 				close(i);
		// 				continue;
		// 			}
		// 		}
		// 	}
		// }
	}
	std::cout << "closing listening socket..." << std::endl;
	close(socket_listen);
	std::cout << "finished." << std::endl;
	return 0;
}

