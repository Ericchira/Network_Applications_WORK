
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

    // Create client socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET) {
        cout << "Could not create client socket.\n";
        WSACleanup();
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(58000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );

    cout << "====================================\n";
    cout << "    NETWORK CONNECTION MONITOR\n";
    cout << "             CLIENT\n";
    cout << "====================================\n";

    cout << "Checking server connection...\n";
    cout << "Server IP Address: 127.0.0.1\n";
    cout << "Server Port: 58000\n";

    // Connect to server
    if (connect(
        clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        cout << "\nCONNECTION STATUS: FAILED\n";
        cout << "The server is unavailable.\n";
        cout << "Please start the monitor server first.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Receive connection status
    char buffer[256] = {};

    int received = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    cout << "\n====================================\n";
    cout << "        CONNECTION REPORT\n";
    cout << "====================================\n";

    if (received > 0) {

        buffer[received] = '\0';

        cout << "Connection Status: SUCCESSFUL\n";
        cout << "Server Response: " << buffer << "\n";
        cout << "Server IP: 127.0.0.1\n";
        cout << "Server Port: 58000\n";

    } else {

        cout << "Connection established, but no response received.\n";
    }

    cout << "\nMonitoring completed.\n";

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
