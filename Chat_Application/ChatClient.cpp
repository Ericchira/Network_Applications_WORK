
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

    // Create client socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(54000);

    // Connect to server on the same laptop
    if (inet_pton(AF_INET, "127.0.0.1",
                  &serverAddress.sin_addr) != 1) {
        cout << "Invalid server IP address.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "=================================\n";
    cout << "        TCP CHAT CLIENT\n";
    cout << "=================================\n";
    cout << "Server IP: 127.0.0.1\n";
    cout << "Port: 54000\n";
    cout << "Connecting to server...\n";

    // Connect to server
    if (connect(clientSocket, (sockaddr*)&serverAddress,
                sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Connection failed. Make sure ChatServer is running.\n";
        cout << "Winsock error: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "\nConnected to the server successfully!\n";
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
                cout << "\nServer disconnected.\n";
                break;
            }

            buffer[bytesReceived] = '\0';
            cout << "\nServer: " << buffer;
            cout << "You: ";
            cout.flush();
        }
    });

    // Send messages
    string message;

    while (connected) {
        cout << "You: ";

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

    if (receiver.joinable()) {
        receiver.join();
    }

    WSACleanup();

    cout << "\nDisconnected from server.\n";

    return 0;
}
