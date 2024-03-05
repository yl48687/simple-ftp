#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sstream>
#include <cstdlib>
#include <fstream>

using namespace std;

constexpr int BUFFER_SIZE = 1024;

void handleGet(int clientSocket, const string& fileName);
void handlePut(int clientSocket, const string& fileName);
void handleDelete(int clientSocket, const string& fileName);
void handleList(int clientSocket);
void handleChangeDirectory(const string& directory, int clientSocket);
void handleMakeDirectory(const string& directory, int clientSocket);
void handlePrintWorkingDirectory(int clientSocket);

void handleClient(int clientSocket) {
    cout << "Client connected." << endl;
    
    while (true) {
        // Handle commands from the client
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1) {
            perror("Error: Unable to receive data from client.");
            close(clientSocket);
            return;
        } // if
        
        buffer[bytesReceived] = '\0';
    
        cout << "Received command from client: " << buffer << endl;
    
        // Parse the received command
        string command(buffer);
    
        istringstream iss(command);
        string cmd;
        iss >> cmd;
    
        // Handle commands
        if (cmd == "get") {
            string fileName;
            iss >> fileName;
            // Call function to handle get command
            handleGet(clientSocket, fileName);
        } else if (cmd == "put") {
            string fileName;
            iss >> fileName; 
            // Call function to handle put command
            handlePut(clientSocket, fileName);
        } else if (cmd == "delete") {
            string fileName;
            iss >> fileName;
            // Call function to handle delete command
            handleDelete(clientSocket, fileName);
        } else if (cmd == "ls") {
            // Call function to handle ls command
            handleList(clientSocket);
        } else if (cmd == "cd") {
            string directory;
            iss >> directory;
            // Call function to handle cd command
            handleChangeDirectory(directory, clientSocket);
        } else if (cmd == "mkdir") {
            string directory;
            iss >> directory;
            // Call function to handle mkdir command
            handleMakeDirectory(directory, clientSocket);
        } else if (cmd == "pwd") {
            // Call function to handle pwd command
            handlePrintWorkingDirectory(clientSocket);
        } else if (cmd == "quit") {
            // Handle quit command (if needed)
            close(clientSocket);
            cout << "Client disconnected." << endl;
            exit(0);
        } else {
            // Handle invalid command
            cerr << "Error: Invalid command." << endl;
        } // if
    } // while
} // handleClient

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << "<port>" << endl;
        return 1;
    } // main

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error: Unable to create server socket.");
        return 1;
    } // if

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(stoi(argv[1]));
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error: Unable to bind server socket.");
        close(serverSocket);
        return 1;
    } // if

    if (listen(serverSocket, 5) == -1) {
        perror("Error: Unable to listen for incoming connections.");
        close(serverSocket);
        return 1;
    } // if

    cout << "Server started. Listening on port " << argv[1] << "..." << endl;

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressLen = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
        if (clientSocket == -1) {
            perror("Error: Unable to accept client connection.");
            continue;
        } // if
        
        handleClient(clientSocket);
    } // while
    
    close(serverSocket);
    exit(0);
} // main

// Function to handle get command
void handleGet(int clientSocket, const string& fileName) {
    // Open the file on the server
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        // If file cannot be opened, send an error message to the client
        string errorMessage = "Error: Unable to open file " + fileName;
        send(clientSocket, errorMessage.c_str(), errorMessage.length(), 0);
        return;
    }

    // Read the file content and send it to the client
    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = file.readsome(buffer, sizeof(buffer))) > 0) {
        int bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent == -1) {
            perror("Error: Unable to send file content to client.");
            return;
        }
    }

    // Close the file
    file.close();
}

void handlePut(int clientSocket, const string& fileName) {
    char nullTerminator = '\0';
    send(clientSocket, &nullTerminator, sizeof(nullTerminator), 0);
}

// Function to handle delete command
void handleDelete(int clientSocket, const string& fileName) {
    if (remove(fileName.c_str()) != 0) {
        cerr << "Error: Unable to delete file " << fileName << "." << endl;
        send(clientSocket, "Error: Unable to delete file.", 28, 0);
    } else {
        send(clientSocket, "File deleted successfully.", 26, 0);
    } // if
} // handleDelete

// Function to handle ls command
void handleList(int clientSocket) {
    DIR* directory = opendir(".");
    if (directory == nullptr) {
        // Error opening directory
        string errorMessage = "Error: Unable to open current directory.";
        send(clientSocket, errorMessage.c_str(), errorMessage.length(), 0);
        return;
    } // if

    // Read directory entries and send them to the client
    struct dirent* entry;
    bool firstFile = true;
    while ((entry = readdir(directory)) != nullptr) {
        string fileName = entry->d_name;
        if (fileName != "." && fileName != "..") {
            if (!firstFile) {
                // If not the first file, send a newline character before sending the file name
                send(clientSocket, "\n", 1, 0);
            } else {
                firstFile = false;
            } // if
            
            send(clientSocket, fileName.c_str(), fileName.length(), 0);
        } // if
    } // while

    closedir(directory);
    if (firstFile) {
        send(clientSocket, "Directory is empty.", strlen("Directory is empty."), 0);
    } // if
} // handleList

// Function to handle cd command
void handleChangeDirectory(const string& directory, int clientSocket) {
    if (directory == "..") {
        // Handle ".." explicitly
        if (chdir("..") == -1) {
            // Error changing directory
            cerr << "Error: Unable to change directory to parent directory." << endl;
            send(clientSocket, "Error: Unable to change directory to parent directory.", strlen("Error: Unable to change directory to parent directory."), 0);
        } else {
            // Directory changed successfully
            send(clientSocket, "Directory changed to parent directory.", strlen("Directory changed to parent directory."), 0);
        } // if
    } else {
        // Handle other directories
        if (chdir(directory.c_str()) == -1) {
            cerr << "Error: Unable to change directory to " << directory << "." << endl;
            send(clientSocket, "Error: Unable to change directory.", strlen("Error: Unable to change directory."), 0);
        } else {
            send(clientSocket, ("Directory changed to " + directory + ".").c_str(), strlen(("Directory changed to " + directory + ".").c_str()), 0);
        } // if
    } // if
} // handleChangeDirectory

// Function to handle mkdir command
void handleMakeDirectory(const string& directory, int clientSocket) {
    if (mkdir(directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        string errorMessage = "Error: Unable to create directory " + directory + ".";
        send(clientSocket, errorMessage.c_str(), errorMessage.length(), 0);
    } else {
        string successMessage = "Directory " + directory + " created successfully.";
        send(clientSocket, successMessage.c_str(), successMessage.length(), 0);
    } // if
} // handleMakeDirectory

// Function to handle pwd command
void handlePrintWorkingDirectory(int clientSocket) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        send(clientSocket, cwd, strlen(cwd), 0);
    } else {
        // Error getting current working directory
        string errorMessage = "Error: Unable to get current working directory.";
        send(clientSocket, errorMessage.c_str(), errorMessage.length(), 0);
    } // if
} // handlePrintWorkingDirectory