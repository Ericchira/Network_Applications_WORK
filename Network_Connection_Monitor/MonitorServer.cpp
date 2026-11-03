
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main() {

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock startup failed.\n";
        return 1;
    }

    // Create server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        cout << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(58000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket
    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {

        cout << "Binding failed. Port 58000 may already be in use.\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for client connections
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listening failed.\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "====================================\n";
    cout << "    NETWORK CONNECTION MONITOR\n";
    cout << "             SERVER\n";
    cout << "====================================\n";

    cout << "Server IP: 127.0.0.1\n";
    cout << "Port: 58000\n";
    cout << "Waiting for client connections...\n";

    // Accept client connection
    sockaddr_in clientAddress{};
    int clientLength = sizeof(clientAddress);

    SOCKET clientSocket = accept(
        serverSocket,
        (sockaddr*)&clientAddress,
        &clientLength
    );

    if (clientSocket == INVALID_SOCKET) {
        cout << "Client connection failed.\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Display client information
    char clientIP[INET_ADDRSTRLEN] = {};

    inet_ntop(
        AF_INET,
        &clientAddress.sin_addr,
        clientIP,
        sizeof(clientIP)
    );

    cout << "\nClient connected successfully!\n";
    cout << "Client IP Address: " << clientIP << "\n";
    cout << "Client Port: " << ntohs(clientAddress.sin_port) << "\n";

    // Send connection status to client
    string response = "CONNECTION SUCCESSFUL";

    int sent = send(
        clientSocket,
        response.c_str(),
        static_cast<int>(response.size()),
        0
    );

    if (sent > 0) {
        cout << "\nConnection status sent to client.\n";
    } else {
        cout << "\nFailed to send connection status.\n";
    }

    cout << "\nMonitoring session completed.\n";

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
