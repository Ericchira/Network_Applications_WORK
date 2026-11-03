
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock initialization failed.\n";
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
    serverAddress.sin_port = htons(54000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Bind failed. Error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for connections
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "=================================\n";
    cout << "       TCP CHAT SERVER\n";
    cout << "=================================\n";
    cout << "Server IP: 127.0.0.1\n";
    cout << "Port: 54000\n";
    cout << "Waiting for a client...\n";

    // Accept one client
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == INVALID_SOCKET) {
        cout << "Client connection failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "\nClient connected successfully!\n";
    cout << "You can now exchange messages.\n";
    cout << "Type /quit to disconnect.\n\n";

    atomic<bool> connected(true);

    // Receive messages in a separate thread
    thread receiver([&]() {
        char buffer[1024];

        while (connected) {
            int bytesReceived = recv(
                clientSocket, buffer, sizeof(buffer) - 1, 0
            );

            if (bytesReceived <= 0) {
                connected = false;
                cout << "\nClient disconnected.\n";
                break;
            }

            buffer[bytesReceived] = '\0';
            cout << "\nClient: " << buffer;
            cout << "Server: ";
            cout.flush();
        }
    });

    // Send messages
    string message;

    while (connected) {
        cout << "Server: ";

        if (!getline(cin, message)) {
            break;
        }

        if (message == "/quit") {
            break;
        }

        message += "\n";

        if (send(clientSocket, message.c_str(),
                 static_cast<int>(message.size()), 0) == SOCKET_ERROR) {
            cout << "Message sending failed.\n";
            break;
        }
    }

    // Close connection
    connected = false;
    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    closesocket(serverSocket);

    if (receiver.joinable()) {
        receiver.join();
    }

    WSACleanup();

    cout << "\nChat server stopped.\n";

    return 0;
}
