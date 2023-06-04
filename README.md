# chatUp (terimnal-based)
ChatUp is a simple terminal-based chat application implemented in C++.
It allows users to communicate with each other in real-time.
![picture alt](https://github.com/ainzsoup/chatUp/blob/main/Screen%20Shot%202023-06-04%20at%204.53.28%20PM.png "Title is optional")

## Feautures
  * User Registraion: Users can create an account by registering with a username and password. The passwords are securely hashed using Libsodium library.
  * User Login: Registered users can log in to the application using their credentials.
  * Random Colors: When users join the chat, they are assigned random colors to distinguish their messages.
  * SQLite Database: The application uses an SQLite database to store user information, including usernames and hashed passwords.

## Prerequisites
Before running ChatUp, ensure that you have the following dependencies installed:
  * C++ Compiler supporting C++11
  * SQLite library
  * Libsodium library

## Installation
1. Clone the repository to your local machine:
```shell
git clone git@github.com:ainzsoup/chatUp.git
```
2. Navigate to the project directoty
```shell
cd ChatUp
```
2. Build the application using the provided Makefile:
```shell
make
```
3. run the application
```shell
./Chatup <port_number> <name>
```
the "name" will be used when the server sends messages in the chat.

## Usage
Upon running the application, you will be prompted to register or log in. Follow the on-screen instructions to create an account or enter your credentials to log in. Once logged in, you can start sending and receiving messages in the chat.

## Database
The application uses an SQLite database to store user information. The database file is named ChatUp.db and will be created automatically upon running the application. If you want to reset the database, you can use the following command:
```bash
make drop
```
this will remove the `ChatUp.db` file, and a new database will be created the next time you run the application.

## Contributing
Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or submit a pull request.
