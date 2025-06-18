#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

using namespace std;

void receiveMessages(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cerr << "Disconnected from server!" << endl;
            break;
        }
        buffer[bytesReceived] = '\0';
        cout << buffer << endl;
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return -1;
    }

    // Create client socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket!" << endl;
        WSACleanup();
        return -1;
    }

    // Connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Port number
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Server IP

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Failed to connect to server!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    cout << "Connected to server! Start typing messages..." << endl;

    thread(receiveMessages, clientSocket).detach();

    string message;
    while (true) {
        getline(cin, message);
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
// For running this code we use
//g++ server.cpp -o server -lws2_32
//g++ client.cpp -o client -lws2_32
// ./server.exe
// ./client.exe