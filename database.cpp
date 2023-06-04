#include "classes.hpp"

Database::Database(std::string name) {
	rc = sqlite3_open(name.c_str(), &_db);
	if (rc) {
		std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(_db) << std::endl;
		sqlite3_close(_db);
		throw std::runtime_error("Error opening SQLite database");
	}
	const char *createSQL = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, passwordhash TEXT);";
	if (sqlite3_exec(_db, createSQL, nullptr, nullptr, nullptr) != SQLITE_OK) {
		sqlite3_close(_db);
		throw std::runtime_error("Error creating table");
	}
}

Database::~Database() {}

void Database::addUser(const std::string &username, std::string password) {

	char hashed_password[crypto_pwhash_STRBYTES];
	if (crypto_pwhash_str(hashed_password, password.c_str(), password.length(), crypto_pwhash_OPSLIMIT_MIN,
						  crypto_pwhash_MEMLIMIT_MIN) != 0) {
		throw std::runtime_error("Error hashing password");
	}
	std::string insertSQL = "INSERT INTO users (username, passwordhash) VALUES ('" + username + "', '" + hashed_password + "');";
	if (sqlite3_exec(_db, insertSQL.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
		sqlite3_close(_db);
		throw std::runtime_error("Error adding user");
	}
}

std::string Database::fetchHash(std::string username) {
	std::string password;
	std::string selectSQL = "SELECT passwordhash FROM users WHERE username = '" + username + "';";
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

bool Database::userExists(std::string username) {
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

bool Database::verifyPassword(const std::string &username, std::string password) {
	std::string hashed_password = fetchHash(username);
	return !crypto_pwhash_str_verify(hashed_password.c_str(), password.c_str(), password.length());
}

