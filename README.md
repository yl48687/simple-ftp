# Simple FTP
The Simple FTP Client and Server project introduces the client-server model of distributed systems through the implementation of basic File Transfer Protocol (FTP) functionalities. The project entails creating two programs: an FTP client, `myftp.c`, and an FTP server, `myftpserver.c`. These programs enable users to transfer files between a client machine and a server over a network connection.

## Design Overview
The design of the FTP client and server revolves around establishing a communication channel between them, handling incoming connections, and executing file transfer commands. Both client and server programs are designed to ensure robustness, efficiency, and ease of use for file transfer operations. The server maintains the current working directory for each client connection, while the client relays user commands to the server and displays results and error messages as necessary.

## Functionality
`myftpserver.c`:
- Accepts a port number as a command-line parameter.
- Listens for incoming connections and executes commands received from clients.
- Handles commands such as get, put, delete, ls, cd, mkdir, pwd, and quit.
- Manages the current working directory for each client connection.
- Provides error messages to clients upon command failure.

`myftp.c`:
- Takes server machine name and port number as command-line parameters.
- Displays a user prompt and accepts commands for file transfer operations.
- Relays user commands to the server and displays results and error messages.
- Terminates upon receiving the quit command.

## File Structure and Content
```
simple-ftp/
├── client/
│   ├── file1.txt
│   ├── file2.txt
│   ├── file4.txt
│   ├── Makefile
│   └── myftp.cpp
├── README.md
└── server/
    ├── file2.txt
    ├── file3.txt
    ├── Makefile
    └── myftpserver.cpp
```