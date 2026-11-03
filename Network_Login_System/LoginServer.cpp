
#include <iostream>
#include <string>
#include <winsock2.h>

using namespace std;

int main() {

    WSADATA wsaData;

    // Start Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock startup failed.\n";
        return 1;
    }

    // Create server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(59000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to port
    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {

        cout << "Binding failed. Port may already be in use.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for clients
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listening failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "====================================\n";
    cout << " NETWORK LOGIN AUTHENTICATION SERVER\n";
    cout << "====================================\n";
    cout << "Port: 59000\n";
    cout << "Waiting for login requests...\n";

    while (true) {

        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET) {
            cout << "Client connection failed.\n";
            continue;
        }

        char buffer[512] = {};

        int received = recv(clientSocket, buffer,
                            sizeof(buffer) - 1, 0);

        if (received > 0) {

            buffer[received] = '\0';

            string credentials(buffer);

            string correctUsername = "eric";
            string correctPassword = "12345";

            string expectedCredentials =
                correctUsername + "|" + correctPassword;

            string response;

            if (credentials == expectedCredentials) {
                response = "LOGIN SUCCESSFUL";
                cout << "User authenticated successfully.\n";
            } else {
                response = "INVALID USERNAME OR PASSWORD";
                cout << "Login attempt rejected.\n";
            }

            send(clientSocket, response.c_str(),
                 static_cast<int>(response.length()), 0);
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
