#include "classes.hpp"
#include "sodium/crypto_pwhash.h"
#include <sodium.h>

int main(int ac, char **av) {
	if (ac < 3) {
		std::cerr << "Usage: " << av[0] << " port name" << std::endl;
		return 1;
	}
	std::cout << HEADER << std::endl;
	std::string password;

	std::cout << "Enter password: ";
	std::getline(std::cin, password);
	char hashed_password[crypto_pwhash_STRBYTES];

	if (crypto_pwhash_str(hashed_password, password.c_str(), password.length(), crypto_pwhash_OPSLIMIT_MIN,
						  crypto_pwhash_MEMLIMIT_MIN) != 0) {
		/* out of memory */
	}
	std::cout << "Password hashed: " << hashed_password << std::endl;
	// Verify the password.
	std::cout << "Enter password again: ";
	std::getline(std::cin, password);

	if (crypto_pwhash_str_verify(hashed_password, password.c_str(), password.length()) != 0) {
		/* wrong password */
		std::cerr << "Wrong password" << std::endl;
		return 1;
	}
	std::cout << "Password verified" << std::endl;

	// Print the hash.
	Server server(av[2]);
	try {
		server.setupSocket(av[1]);
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
