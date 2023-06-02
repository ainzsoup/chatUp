//  _________ .__            __
//  \_   ___ \|  |__ _____ _/  |_ __ ________
//  /    \  \/|  |  \\__  \\   __\  |  \____ \ 
//  \     \___|   Y  \/ __ \|  | |  |  /  |_> >
//   \______  /___|  (____  /__| |____/|   __/
//          \/     \/     \/           |__|
//

#include "server.hpp"
#include <unistd.h>

Server::Server(std::string name) : _name(name) {}

Server::~Server() {}

void Server::setupSocket(char *port) {
    // configuring local address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // listen on socket

    struct addrinfo *bind_address;
    int slience =
        getaddrinfo(0, port, &hints,
                    &bind_address); // just to silence unused variable warning

    // creatng socket
    _socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                            bind_address->ai_protocol);
    if (_socket_listen < 0)
        throw std::runtime_error("socket() failed");

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

    if (select(_max_socket + 1, &_sets[READ], &_sets[WRITE], 0, &tv) < 0)
        throw std::runtime_error("select() failed");
}

const int &Server::getSocketListen() const { return _socket_listen; }

const int &Server::getMaxSocket() const { return _max_socket; }

const fd_set &Server::getSets(int i) const { return _sets[i]; }

const std::string &Server::getName() const { return _name; }

void Server::sendWelcomeMessage(int i) {
    std::string header = HEADER;
    header.insert(0, "\033[34m"); // blue
    header.append("\033[0m\n");
    send(i, header.c_str(), header.size() + 1, 0);
    std::string msg =
        "Connection established.\nPlease enter your desired username:";
    send(i, msg.c_str(), msg.size() + 1, 0);
}

void Server::acceptConnection() {
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int socket_client =
        accept(_socket_listen, (struct sockaddr *)&client_address, &client_len);
    if (socket_client < 0)
        throw std::runtime_error("accept() failed");
    FD_SET(socket_client, &_sets[MASTER]);
    if (socket_client > _max_socket)
        _max_socket = socket_client;
    char address_buffer[100];
    getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer,
                sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
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
	std::cout << _clients[sender]._color << _clients[sender].getName()
			  << ": " << read << "\033[0m" << std::endl;
    broadcastMessage(sender, read, bytes_received);
}

void Server::broadcastMessage(int sender, char *read, int bytes_received) {
    char message[2048];
    sprintf(message, "%s%s: %s\033[0m\n", _clients[sender]._color.c_str(),
            _clients[sender].getName().c_str(), read);
    for (int j = 1; j <= _max_socket; ++j) {
        if (FD_ISSET(j, &_sets[MASTER])) {
            if (j == _socket_listen || j == sender ||
                _clients[j].getStatus() ==
                    EXPECTING_NAME) // don't send to listener or sender or to
                                    // client that hasn't entered name yet
                continue;
            else
                send(j, message, strlen(message), 0);
        }
    }
}

void Server::sendMessage() {
    char message[1024];
    std::cin.getline(message, 1024);
    char formatted_message[2048];
    sprintf(formatted_message, "%s: %s\n", _name.c_str(), message);
    for (int j = 1; j <= _max_socket; ++j) {
        if (FD_ISSET(j, &_sets[WRITE]))
            if (j != _socket_listen) {
                int begin = 0;
                int bytes_sent = 0;
                while (bytes_sent < strlen(formatted_message)) {
                    bytes_sent = send(j, formatted_message + begin,
                                      strlen(formatted_message) - begin, 0);
                    begin += bytes_sent;
                }
            }
    }
}

void Server::announce(std::string msg, int exclude) {
    char message[2048];
    sprintf(message, "\033[30;100m%s\033[0m\n", msg.c_str());
    for (int j = 1; j <= _max_socket; ++j) {
        if (FD_ISSET(j, &_sets[WRITE]))
            if (j != _socket_listen && j != exclude &&
                _clients[j].getStatus() == CONNECTED) {
                int begin = 0;
                int bytes_sent = 0;
                while (bytes_sent < strlen(message)) {
                    bytes_sent =
                        send(j, message + begin, strlen(message) - begin, 0);
                    begin += bytes_sent;
                }
            }
    }
}

void Server::parseName(std::string name) {
    if (name.size() == 0)
        throw "\033[31mUsername cannot be empty :( try Again\n\033[0mUsername:";
    if (name.size() < 1 || name.size() > 20)
        throw "\033[31mUsername must be between 1 and 20 characters :( try "
              "Again\n\033[0mUsername:";
    for (int i = 0; i < name.size(); ++i)
        if (!isalnum(name[i]))
            throw "\033[31mUsername must be alphanumeric :( try "
                  "Again\n\033[0mUsername:";
    if (name == _name)
        throw "\033[31mUsername already taken :( try Again\n\033[0mUserName:";
    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
        if (it->second.getName() == name)
            throw "\033[31mUsername already taken :( try "
                  "Again\n\033[0mUserName:";
}

int Server::getClientName(int client) {
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
        parseName(name);
    } catch (const char *error) {
        send(client, error, strlen(error), 0);
        return 1;
    }
    _clients[client].setName(name);
    announce(_clients[client].getName() + " has joined the chat!", client);
    return 0;
}

void Server::handleClient(int client) {
    switch (_clients[client].getStatus()) {
    case CONNECTED:
        receiveMessage(client);
        break;
    case EXPECTING_NAME:
        if (!getClientName(client)) {
            _clients[client].setStatus(CONNECTED);
            send(client, "Welcome to the chat!\n", 21, 0);
        }
        break;
    }
}



