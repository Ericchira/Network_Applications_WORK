
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

using namespace std;

// Get this computer's IPv4 address
string getLocalIP() {

    char hostName[256] = {};

    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
        return "Unavailable";
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* results = nullptr;

    if (getaddrinfo(hostName, nullptr, &hints, &results) != 0) {
        return "Unavailable";
    }

    string ip = "Unavailable";

    for (addrinfo* current = results;
         current != nullptr;
         current = current->ai_next) {

        sockaddr_in* address =
            reinterpret_cast<sockaddr_in*>(current->ai_addr);

        char ipAddress[INET_ADDRSTRLEN] = {};

        inet_ntop(AF_INET, &address->sin_addr,
                  ipAddress, sizeof(ipAddress));

        if (string(ipAddress) != "127.0.0.1") {
            ip = ipAddress;
            break;
        }
    }

    freeaddrinfo(results);
    return ip;
}

int main() {

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(60000);

    // Local testing: connect to this computer
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    cout << "====================================\n";
    cout << " NETWORK IP INFORMATION CLIENT\n";
    cout << "====================================\n";

    cout << "Client Hostname: ";

    char hostName[256] = {};

    if (gethostname(hostName, sizeof(hostName)) == 0) {
        cout << hostName << "\n";
    }

    cout << "Client IPv4: " << getLocalIP() << "\n";
    cout << "Connecting to server...\n";

    if (connect(clientSocket, (sockaddr*)&serverAddress,
                sizeof(serverAddress)) == SOCKET_ERROR) {

        cout << "Connection failed.\n";
        cout << "Check that the server is running.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "\nConnection established successfully!\n";

    char buffer[1024] = {};

    int received = recv(clientSocket, buffer,
                        sizeof(buffer) - 1, 0);

    cout << "\n====================================\n";
    cout << "       NETWORK INFORMATION\n";
    cout << "====================================\n";

    if (received > 0) {
        buffer[received] = '\0';
        cout << buffer << "\n";
    } else {
        cout << "No information received from server.\n";
    }

    cout << "\nNetwork information request completed.\n";

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
