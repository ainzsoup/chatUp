#include "server.hpp"

client::client(int socket) : _socket(socket), _status(EXPECTING_NAME)
{
}

client::~client()
{
}

void client::setName(std::string name)
{
	_name = name;
}

const std::string &client::getName() const
{
	return _name;
}

void client::setStatus(int status)
{
	_status = status;
}

const int &client::getStatus() const
{
	return _status;
}

 
