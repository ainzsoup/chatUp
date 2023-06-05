#include "classes.hpp"
#include "sodium/crypto_pwhash.h"
#include <sodium.h>

int execute_oauth_server() {
	int fds[2];
	pipe(fds);
	if (fork() == 0) {
		dup2(fds[1], STDOUT_FILENO);
		close(fds[0]);
		close(fds[1]);
		execv("./oauth_server", NULL);
	} else {
		close(fds[1]);
	}
	return fds[0];
}

void handle_oauth_server(int oauth_server, Server &server) {
	char hex_state[9];
	char hex_login_len[9];
	char login[1024];
	read(oauth_server, hex_state, 8);
	read(oauth_server, hex_login_len, 8);
	hex_state[8] = '\0';
	hex_login_len[8] = '\0';
	int login_len = strtol(hex_login_len, NULL, 16);
	int client = strtol(hex_state, NULL, 16);
	read(oauth_server, login, login_len);
	login[login_len] = '\0';
	if (client != 0) {
		std::string welcome = "Welcome, " + std::string(login) + "!\n";
		send(client, welcome.c_str(), welcome.length(), 0);
		server.getClients()[client].setStatus(CONNECTED);
		server.getClients()[client].setName("⁴²" + std::string(login));
	}

}

int main(int ac, char **av) {
	if (ac < 3) {
		std::cerr << "Usage: " << av[0] << " port name" << std::endl;
		return 1;
	}

	std::cout << HEADER << std::endl;
	int oauth_server = execute_oauth_server();
	Server server(av[2]);
	try {
		server.setupSocket(av[1]);
		server.addClient(oauth_server);
		std::cout << "waiting for connections..." << std::endl;
		while (true) {
			usleep(100);
			server.getReadyDescriptors(0, 0);
			for (int i = 0; i <= server.getMaxSocket(); ++i) {
				if (FD_ISSET(i, &server.getSets(READ))) {
					if (i == server.getSocketListen()) // new connection
						server.acceptConnection();
					else if (i == STDIN_FILENO) // server input
						server.sendMessage();
					else if (i == oauth_server) // oauth server
						handle_oauth_server(oauth_server, server);
					else // client message
						server.handleClient(i);
				}
			}
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "quitting..." << std::endl;
	close(server.getSocketListen());

	return 0;
}
