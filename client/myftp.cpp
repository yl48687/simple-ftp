#include <iostream>
#include <sys/socket.h>
#include <sstream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << "<serve_ip> <port>" << endl;
        return 1;
    } // if

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating client socket.");
        return 1;
    } // if

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(stoi(argv[2]));
    inet_pton(AF_INET, argv[1], &(serverAddress.sin_addr));

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error connecting to the server.");
        close(clientSocket);
        return 1;
    } // if

    // Start handling user commands
    while (true) {
        // Prompt user for command input
        cout << "myftp> ";
        string command;
        getline(cin, command);

        // Send the command to the server
        int bytesSent = send(clientSocket, command.c_str(), command.length(), 0);
        if (bytesSent == -1) {
            perror("Error sending command to server.");
            close(clientSocket);
            return 1;
        } // if

        // Check if the user entered the quit command
        if (command == "quit") {
            close(clientSocket);
            return 0;
        } // if

        // Receive response from the server
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1) {
            perror("Error receiving response from server.");
            close(clientSocket);
            return 1;
        } // if

        // Display response received from the server
        buffer[bytesReceived] = '\0';
        
        // Check if the command is "get"
        istringstream iss(command);
        string cmd;
        iss >> cmd;

        if (cmd == "get") {
            string fileName;
            iss >> fileName;
            // Open a new file to write the received content
            ofstream outputFile(fileName, ios::binary);
            if (!outputFile.is_open()) {
                cerr << "Error: Unable to create file " << fileName << " for writing." << endl;
                continue;
            }
            // Write the received content into the file
            outputFile.write(buffer, bytesReceived);
            outputFile.close();
        } else {
            cout << buffer << endl;
        } // if
    } // while
    
    close(clientSocket);
    return 0;
} // main