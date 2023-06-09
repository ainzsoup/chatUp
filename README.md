# chatUp (terimnal-based)
![logo](https://github.com/ainzsoup/chatUp/blob/main/ChatUplogo.png.png)
ChatUp is a simple terminal-based chat application implemented in C++.
It allows users to communicate with each other in real-time.


## Feautures
  * Login with a 42 account (exclusively to users with valid Intra accounts. If you don't have an Intra account, you can still register and log in using the traditional account creation method.)
  * User Registraion: Users can create an account by registering with a username and password. The passwords are securely hashed using Libsodium library.
  * User Login: Registered users can log in to the application using their credentials.
  * Random Colors: When users join the chat, they are assigned random colors to distinguish their messages.
  * SQLite Database: The application uses an SQLite database to store user information, including usernames and hashed passwords.
 
## Prerequisites
Before running ChatUp, ensure that you have the following dependencies installed:
  * C++ Compiler supporting C++11
  * SQLite library

## Installation
1. Install the libsodium library with the provided Makefile:
```shell
make install-dependencies
```
3. Build the application
```shell
make
```
2. run the application
```shell
./chatup <port_number> <name>
```

## Usage
Run the server application usin the following command:
```bash
./chatUp <port> <server_name>
```
Replace `port` with the desired port number on which you want the server to listen for incoming connections.
Replace `server_name` with a desired name that will show up to connected users in the chat when the server sends a message.
Example: ./chatUp 5000 server
Open another terminal window or tab (separate from the one running the server).
In the new terminal, use the nc command to connect to the server. The command should be in the following format:
```bash
 nc <server_ip_address> <server_port>
 ```
 Replace `server_ip_address` with the IP address of the machine running the chat server and `server_port` with the port number specified when starting the server.
 
Example: nc 192.168.0.100 5000
 
After executing the nc command, you should be connected to the chat server. The application will prompt you to either register or log in.

Follow the on-screen instructions to create a new account or enter your existing credentials to log in.

Once logged in, you can start sending and receiving messages through the chat application.
## Database
The application uses an SQLite database to store user information. The database file is named ChatUp.db and will be created automatically upon running the application. If you want to reset the database, you can use the following command:
```bash
make drop
```
this will remove the `ChatUp.db` file, and a new database will be created the next time you run the application.

## Preview
![preview](https://github.com/ainzsoup/chatUp/blob/main/chatUp_preview.gif)

## Contributing
Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or submit a pull request.
