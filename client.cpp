#include "server.hpp"
#include <ctime>

Client::Client() {}

const char *colors[] = {
    "\033[91m", // Light Red
    "\033[92m", // Light Green
    "\033[93m", // Light Yellow
    "\033[94m", // Light Blue
    "\033[95m", // Light Magenta
    "\033[96m", // Light Cyan
    "\033[97m", // Bright White
};
std::vector<std::string>
    Client::_colorsList(colors, colors + sizeof(colors) / sizeof(colors[0]));
Client::Client(int socket)
    : _socket(socket), _status(EXPECTING_NAME), _color("\033[0m") {
	srand(time(NULL));
    int colorIndex = rand() % _colorsList.size();
    _color = _colorsList[colorIndex];
}

Client::~Client() {}

void Client::setName(std::string name) { _name = name; }

const std::string &Client::getName() const { return _name; }

void Client::setStatus(int status) { _status = status; }

const int &Client::getStatus() const { return _status; }
