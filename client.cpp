#include "server.hpp"
Client::Client()
{
}

Client::Client(int socket) : _socket(socket), _status(EXPECTING_NAME)
{
}

Client::~Client()
{
}

void Client::setName(std::string name)
{
	_name = name;
}

const std::string &Client::getName() const
{
	return _name;
}

void Client::setStatus(int status)
{
	_status = status;
}

const int &Client::getStatus() const
{
	return _status;
}

 
