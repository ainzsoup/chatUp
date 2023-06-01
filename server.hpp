#pragma once
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_timeval.h>
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
#include <cstring>
#include <map>
#include <vector>
#include <sqlite3.h>


enum 
{
	MASTER,
	READ,
	WRITE,
};

class Server
{
public:
	Server();
	~Server();
	void setupSocket(char *port);
	void run(int ac, char **av);
	void getReadyDescriptors(int timeout_sec, int timeout_usec);
	void acceptConnection();
	void receiveMessage(int i);
	void sendMessage(char *name);
	void broadcastMessage(int sender , char *msg, int bytes_received);
	void sendWelcomeMessage(int i);
	void getClientName(int i);
	const int &getSocketListen() const;
	const int &getMaxSocket() const;
	const fd_set &getSets(int i) const;


	//undone
	void createDatabase();
	
private:
	int _socket_listen;
	int _max_socket;
	fd_set _sets[3];
	std::map<int ,std::string> _users; // <socketId, name>
	std::map<std::string, std::string> _users_db; // <name, password>
};

// WIP
class Client 
{
public:
	Client(char *ip, char *port);
	~Client();
	void login();
	void registerUser();
	void connectToServer();
	void receiveMessage();
	void sendMessage();
	void getReadyDescriptors(int timeout_sec, int timeout_usec);
	const int &getSocket() const;
	const fd_set &getSets(int i) const;
private:
	Client();
	int _socket;
	std::string _name;
	std::string _password;
	char *_ip;
	char *_port;
	fd_set _sets[3];
};
