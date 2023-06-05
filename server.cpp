//  _________ .__            __
//  \_   ___ \|  |__ _____ _/  |_ __ ________
//  /    \  \/|  |  \\__  \\   __\  |  \____ \ 
//  \     \___|   Y  \/ __ \|  | |  |  /  |_> >
//   \______  /___|  (____  /__| |____/|   __/
//          \/     \/     \/           |__|
//

#include "classes.hpp"

Server::Server(std::string name) : _name(name), _db("ChatUp.db") {}

Server::~Server() {}

void Server::setupSocket(char *port) {
	// configuring local address
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		 // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;	 // listen on socket

	struct addrinfo *bind_address;
	int silence = getaddrinfo(0, port, &hints, &bind_address); // just to silence the unused warning

	// creatng socket
	_socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (_socket_listen < 0)
		throw std::runtime_error("socket() failed");
	int optval = 1;
	setsockopt(_socket_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	// binding socket to local bind_address
	if (bind(_socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
		throw std::runtime_error("bind() failed");
	freeaddrinfo(bind_address);

	// listening
	if (listen(_socket_listen, 10) < 0)
		throw std::runtime_error("listen() failed");

	FD_ZERO(&_sets[MASTER]);
	FD_SET(_socket_listen, &_sets[MASTER]);
	FD_SET(0, &_sets[MASTER]);
	_max_socket = _socket_listen;
}

void Server::getReadyDescriptors(int timeout_sec, int timeout_usec) {
	_sets[READ] = _sets[MASTER];
	_sets[WRITE] = _sets[MASTER];
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = timeout_usec;

	if (select(_max_socket + 1, &_sets[READ], NULL, 0, NULL) < 0)
		throw std::runtime_error("select() failed");
}

const int &Server::getSocketListen() const { return _socket_listen; }

const int &Server::getMaxSocket() const { return _max_socket; }

const fd_set &Server::getSets(int i) const { return _sets[i]; }

const std::string &Server::getName() const { return _name; }

std::map<int, Client> &Server::getClients() { return _clients; }

void Server::sendWelcomeMessage(int client) {
	std::string header = HEADER;
	header.insert(0, "\033[38;5;201m"); // blue
	header.append("\033[0m\n");
	send(client, header.c_str(), header.size() + 1, 0);
	std::string menu =
		"Welcome to ChatUp!\n"
		"To log in with your 42 account, please go to this url:\n"
		"\033[38;5;44m"
		"https://api.intra.42.fr/oauth/"
		"authorize?client_id=u-s4t2ud-4abbfeac4233b7388035477a7812d40f6e2b6765d3ce76263aa10532caec27d0&redirect_"
		"uri=http%3A%2F%2F10.12.12.1%3A8080%2Fauth&response_type=code&state=" +
		std::to_string(client) + "\n"
		"\033[0m"
		"To log in with a ChatUp account choose an option:\n"
		"1. Login\n"
		"2. Register\n"
		"3. Exit\n"
		"Enter the option number: \n";
	send(client, menu.c_str(), menu.size() + 1, 0);
}

void Server::acceptConnection() {
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	int socket_client = accept(_socket_listen, (struct sockaddr *)&client_address, &client_len);
	if (socket_client < 0)
		throw std::runtime_error("accept() failed");
	FD_SET(socket_client, &_sets[MASTER]);
	if (socket_client > _max_socket)
		_max_socket = socket_client;
	char address_buffer[100];
	getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0,
				NI_NUMERICHOST);
	std::cout << "New connection from " << address_buffer << std::endl;
	Client new_client(socket_client);
	_clients[socket_client] = new_client;
	sendWelcomeMessage(socket_client);
}

void Server::receiveMessage(int sender) {
	char read[1024];
	int bytes_received = recv(sender, read, 1024, 0);
	if (bytes_received < 1) {
		close(sender);
		FD_CLR(sender, &_sets[MASTER]);
		_clients.erase(sender);
		return;
	}
	if (bytes_received == 1 && read[0] == '\n')
		return;
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	read[bytes_received] = '\0';
	if (bytes_received > 0 && read[bytes_received - 1] == '\n')
		read[bytes_received - 1] = '\0';
	std::cout << _clients[sender]._color << _clients[sender].getName() << ": " << read << "\033[0m" << std::endl;
	broadcastMessage(sender, read, bytes_received);
}

void Server::broadcastMessage(int sender, char *read, int bytes_received) {
	char message[2048];
	sprintf(message, "%s%s: %s\033[0m\n", _clients[sender]._color.c_str(), _clients[sender].getName().c_str(), read);
	for (int j = 1; j <= _max_socket; ++j) {
		if (FD_ISSET(j, &_sets[MASTER])) {
			if (_clients[j].getStatus() == CONNECTED && j != _socket_listen && j != sender)
				send(j, message, strlen(message), 0);
		}
	}
}

void Server::addClient(int socket) {
	FD_SET(socket, &_sets[MASTER]);
	if (socket > _max_socket)
		_max_socket = socket;
}

void Server::removeClient(int socket) { FD_CLR(socket, &_sets[MASTER]); }

void Server::sendMessage() {
	char message[1024];
	std::cin.getline(message, 1024);
	char formatted_message[2048];
	sprintf(formatted_message, "%s: %s\n", _name.c_str(), message);
	for (int j = 1; j <= _max_socket; ++j) {
		if (FD_ISSET(j, &_sets[WRITE]))
			if (j != _socket_listen && _clients[j].getStatus() == CONNECTED) {
				int begin = 0;
				int bytes_sent = 0;
				while (bytes_sent < strlen(formatted_message)) {
					bytes_sent = send(j, formatted_message + begin, strlen(formatted_message) - begin, 0);
					begin += bytes_sent;
				}
			}
	}
}

void Server::announce(std::string msg, int exclude) {
	char message[2048];
	sprintf(message, "\033[38;5;147m%s\033[0m\n", msg.c_str());
	for (int j = 1; j <= _max_socket; ++j) {
		if (FD_ISSET(j, &_sets[WRITE]))
			if (j != _socket_listen && j != exclude && _clients[j].getStatus() == CONNECTED) {
				int begin = 0;
				int bytes_sent = 0;
				while (bytes_sent < strlen(message)) {
					bytes_sent = send(j, message + begin, strlen(message) - begin, 0);
					begin += bytes_sent;
				}
			}
	}
}

void Server::parseName(std::string name, bool _new) {
	if (name.size() == 0)
		throw "\033[31mUsername cannot be empty :( try Again\n\033[0mUsername:";
	if (name.size() < 3 || name.size() > 20)
		throw "\033[31mUsername must be between 3 and 20 characters :( try "
			  "Again\n\033[0mUsername:";
	for (int i = 0; i < name.size(); ++i)
		if (!isalnum(name[i]))
			throw "\033[31mUsername must be alphanumeric :( try "
				  "Again\n\033[0mUsername:";
	if (name == _name)
		throw "\033[31mUsername already taken :( try Again\n\033[0mUserName:";
	if (_new && _db.userExists(name))
		throw "\033[31mUsername already taken :( try "
			  "Again\n\033[0mUserName:";
	else if (!_new && !_db.userExists(name))
		throw "\033[31mUsername does not exist :( try "
			  "Again\n\033[0mUserName:";
}

int Server::get_client_new_name(int client) {
	char name[1024];
	int bytes_received = recv(client, name, 1024, 0);
	if (bytes_received < 1) {
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
		return 1;
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	name[bytes_received] = '\0';
	if (bytes_received > 0 && name[bytes_received - 1] == '\n')
		name[bytes_received - 1] = '\0';
	try {
		parseName(name, true);
	} catch (const char *error) {
		send(client, error, strlen(error), 0);
		return 1;
	}
	_clients[client].setName(name);
	send(client, "Password:", 10, 0);
	return 0;
}

int Server::get_client_new_password(int client) {
	char password[1024];
	int bytes_received = recv(client, password, 1024, 0);
	if (bytes_received < 1) {
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
		return 1;
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	password[bytes_received] = '\0';
	if (bytes_received > 0 && password[bytes_received - 1] == '\n')
		password[bytes_received - 1] = '\0';
	if (strlen(password) < 6 || strlen(password) > 20) {
		send(client,
			 "\033[31mPassword must be between 6 and 20 characters :( try "
			 "Again\n\033[0mPassword:",
			 77, 0);
		return 1;
	}
	_db.addUser(_clients[client].getName(), password);
	send(client, "\033[32mAccount created successfully :)\nYou can chat now!\n\033[0m", 60, 0);
	announce(_clients[client].getName() + " joined the chat", client);
	return 0;
}

int Server::get_client_login_name(int client) {
	char name[1024];
	int bytes_received = recv(client, name, 1024, 0);
	if (bytes_received < 1) {
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
		return 1;
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	name[bytes_received] = '\0';
	if (bytes_received > 0 && name[bytes_received - 1] == '\n')
		name[bytes_received - 1] = '\0';
	try {
		parseName(name, false);
	} catch (const char *error) {
		send(client, error, strlen(error), 0);
		return 1;
	}
	_clients[client].setName(name);
	send(client, "Password:", 10, 0);
	return 0;
}

int Server::get_client_password(int client) {
	char password[1024];
	int bytes_received = recv(client, password, 1024, 0);
	if (bytes_received < 1) {
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
		return 1;
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	password[bytes_received] = '\0';
	if (bytes_received > 0 && password[bytes_received - 1] == '\n')
		password[bytes_received - 1] = '\0';
	if (!_db.verifyPassword(_clients[client].getName(), password)) {
		send(client, "\033[31mIncorrect password :( try Again\n\033[0mPassword:", 51, 0);
		return 1;
	}
	send(client, "\033[32mLogged in successfully :)\nYou can chat now!\n\033[0m", 54, 0);
	announce(_clients[client].getName() + " has joined the chat", client);
	return 0;
}

void Server::get_client_option(int client) {
	char option[1024];
	int bytes_received = recv(client, option, 1024, 0);
	if (bytes_received < 1) {
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
	}
	bytes_received = bytes_received > 1024 ? 1024 : bytes_received;
	option[bytes_received] = '\0';
	if (bytes_received > 0 && option[bytes_received - 1] == '\n')
		option[bytes_received - 1] = '\0';
	if (strcmp(option, "1") == 0) {
		_clients[client].setStatus(EXPECTING_LOGIN_NAME);
		send(client, "Username:", 9, 0);
	} else if (strcmp(option, "2") == 0) {
		_clients[client].setStatus(EXPECTING_NEW_NAME);
		send(client, "Username:", 9, 0);
	} else if (strcmp(option, "3") == 0) {
		close(client);
		FD_CLR(client, &_sets[MASTER]);
		_clients.erase(client);
	} else {
		send(client, "Invalid option :( try Again\nOption:", 35, 0);
	}
}

void Server::handleClient(int client) {
	switch (_clients[client].getStatus()) {
	case CONNECTED:
		receiveMessage(client);
		break;
	case IN_MENU:
		get_client_option(client);
		break;
	case EXPECTING_LOGIN_NAME:
		if (!get_client_login_name(client))
			_clients[client].setStatus(EXPECTING_PASSWORD);
		break;
	case EXPECTING_PASSWORD:
		if (!get_client_password(client))
			_clients[client].setStatus(CONNECTED);
		break;
	case EXPECTING_NEW_NAME:
		if (!get_client_new_name(client))
			_clients[client].setStatus(EXPECTING_NEW_PASSWORD);
		break;
	case EXPECTING_NEW_PASSWORD:
		if (!get_client_new_password(client))
			_clients[client].setStatus(CONNECTED);
		break;
	}
}
