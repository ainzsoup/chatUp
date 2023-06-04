#pragma once
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_timeval.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <sodium.h>
#define HEADER                                                                                                         \
	"_________ .__            __                \n\\_   ___ \\|  |__ _____ "                                           \
	"_/  |_ __ ________  \n/    \\  \\/|  |  \\\\__  \\\\   __\\  |  \\____ "                                          \
	"\\ \n\\     \\___|   Y  \\/ __ \\|  | |  |  /  |_> > \n \\______  /___| "                                         \
	" (____  /__| |____/|   __/ \n        \\/     \\/     \\/           |__| "                                         \
	"  \n"
enum sets {
	MASTER,
	READ,
	WRITE,
};

enum status {
	CONNECTED,
	EXPECTING_LOGIN_NAME,
	EXPECTING_PASSWORD,
	EXPECTING_NEW_NAME,
	EXPECTING_NEW_PASSWORD,
	IN_MENU,
};

class Database {
  public:
	Database(std::string name);
	~Database();
	void addUser(const std::string &username, std::string password);
	std::string fetchHash(std::string username);
	bool userExists(std::string username);
	bool verifyPassword(const std::string &username, std::string password);
	void close();

  private:
	Database();
	sqlite3 *_db;
	int rc;
};

class Client {
  public:
	Client();
	Client(int socket);
	~Client();
	void setName(std::string name);
	const std::string &getName() const;
	void setStatus(int status);
	const int &getStatus() const;
	std::string _color;
	static std::vector<std::string> _colorsList;

  private:
	int _socket;
	std::string _name;
	int _status;
};

class Server {
  public:
	Server(std::string name);
	~Server();
	void setupSocket(char *port);
	void run(int ac, char **av);
	void getReadyDescriptors(int timeout_sec, int timeout_usec);
	void acceptConnection();
	void receiveMessage(int i);
	void sendMessage();
	void broadcastMessage(int sender, char *msg, int bytes_received);
	void sendWelcomeMessage(int i);
	void handleClient(int client);
	void announce(std::string msg, int exclude);
	void parseName(std::string name, bool _new);
	const int &getSocketListen() const;
	const int &getMaxSocket() const;
	const fd_set &getSets(int i) const;
	const std::string &getName() const;
	void get_client_option(int client);
	int get_client_login_name(int client);
	int get_client_new_name(int client);
	int get_client_new_password(int client);
	int get_client_password(int client);

  private:
	Database _db;
	std::string _name;
	int _socket_listen;
	int _max_socket;
	fd_set _sets[3];
	std::map<int, Client> _clients; // <socketId, client>
};
