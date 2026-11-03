
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main() {

    WSADATA wsaData;

    // Start Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock startup failed.\n";
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
    serverAddress.sin_port = htons(59000);

    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // Connect to server
    if (connect(clientSocket, (sockaddr*)&serverAddress,
                sizeof(serverAddress)) == SOCKET_ERROR) {

        cout << "Could not connect to login server.\n";
        cout << "Start the server and try again.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "====================================\n";
    cout << " NETWORK LOGIN AUTHENTICATION CLIENT\n";
    cout << "====================================\n";

    string username;
    string password;

    cout << "Enter username: ";
    getline(cin, username);

    cout << "Enter password: ";
    getline(cin, password);

    // Prepare login credentials
    string credentials = username + "|" + password;

    // Send credentials to server
    int sent = send(clientSocket, credentials.c_str(),
                    static_cast<int>(credentials.length()), 0);

    if (sent == SOCKET_ERROR) {
        cout << "Failed to send login details.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Receive server response
    char buffer[256] = {};

    int received = recv(clientSocket, buffer,
                        sizeof(buffer) - 1, 0);

    cout << "\n====================================\n";
    cout << "          LOGIN RESULT\n";
    cout << "====================================\n";

    if (received > 0) {
        buffer[received] = '\0';
        cout << buffer << "\n";
    } else {
        cout << "No response received from server.\n";
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
