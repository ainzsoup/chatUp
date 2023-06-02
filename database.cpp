#include "classes.hpp"

Database::Database(std::string name) {
	rc = sqlite3_open(name.c_str(), &_db);
	if (rc) {
		std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(_db) << std::endl;
		sqlite3_close(_db);
		throw std::runtime_error("Error opening SQLite database");
	}
	const char *createSQL = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT);";
	if (sqlite3_exec(_db, createSQL, nullptr, nullptr, nullptr) != SQLITE_OK) {
		sqlite3_close(_db);
		throw std::runtime_error("Error creating table");
	}
}

Database::~Database() {}

void Database::addUser(std::string &username, std::string &password) {
	std::string insertSQL = "INSERT INTO users (username, password) VALUES ('" + username + "', '" + password + "');";
	if (sqlite3_exec(_db, insertSQL.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
		sqlite3_close(_db);
		throw std::runtime_error("Error adding user");
	}
}

std::string Database::fetchPassword(std::string &username) {
	std::string password;
	std::string selectSQL = "SELECT password FROM users WHERE username = '" + username + "';";
	rc = sqlite3_exec(
		_db, selectSQL.c_str(),
		[](void *data, int argc, char **argv, char **colName) {
			if (argc > 0) {
				std::string *password = static_cast<std::string *>(data);
				*password = argv[0];
			}
			return 0;
		},
		&password, nullptr);

	if (rc != SQLITE_OK && rc != SQLITE_DONE) {
		sqlite3_close(_db);
		throw std::runtime_error("Error fetching password");
	}

	return password;
}

void Database::close() { sqlite3_close(_db); }

bool Database::userExists(std::string &username) {
	std::string selectSQL = "SELECT username FROM users WHERE username = '" + username + "';";
	bool exists = false;
	rc = sqlite3_exec(
		_db, selectSQL.c_str(),
		[](void *data, int argc, char **argv, char **colName) {
			if (argc > 0) {
				bool *exists = static_cast<bool *>(data);
				*exists = true;
			}
			return 0;
		},
		&exists, nullptr);
	if (rc != SQLITE_OK) {
		sqlite3_close(_db);
		throw std::runtime_error("Error checking if user exists");
	}
	return exists;
}

// int main()
// {
// 	Database db("mydatabase.db");
//
// 	std::string username = "test";
// 	std::string password = "passwordtest";
//
// 	db.addUser(username, password);
//
// 	std::cout << db.userExists(username) << std::endl;
// 	std::cout << db.fetchPassword(username) << std::endl;
// 	return 0;
//
// }
