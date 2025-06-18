#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm> // For std::remove
#include <mutex>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

using namespace std;

vector<SOCKET> clients; // Store client sockets
mutex mtx;              // Mutex for thread-safe access to clients vector

void broadcastMessage(const string& message, SOCKET senderSocket) {
    lock_guard<mutex> lock(mtx);
    for (SOCKET client : clients) {
        if (client != senderSocket) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            // Client disconnected
            lock_guard<mutex> lock(mtx);
            auto it = remove(clients.begin(), clients.end(), clientSocket);
            if (it != clients.end()) {
                clients.erase(it, clients.end());
            }
            closesocket(clientSocket);
            break;
        }
        buffer[bytesReceived] = '\0';
        string message = "Client " + to_string(clientSocket) + ": " + buffer;
        broadcastMessage(message, clientSocket);
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return -1;
    }

    // Create server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket!" << endl;
        WSACleanup();
        return -1;
    }

    // Bind socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Port number
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Failed to bind socket!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // Listen for connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Failed to listen on socket!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    cout << "Server is running and waiting for connections..." << endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Failed to accept client connection!" << endl;
            continue;
        }

        lock_guard<mutex> lock(mtx);
        clients.push_back(clientSocket);
        thread(handleClient, clientSocket).detach();
        cout << "Client " << clientSocket << " connected!" << endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}