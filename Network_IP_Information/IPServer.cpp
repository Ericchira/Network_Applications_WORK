
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

// Get the computer's IPv4 address
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

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(60000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {

        cout << "Binding failed. Port 60000 may be in use.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listening failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    char hostName[256] = {};
    gethostname(hostName, sizeof(hostName));

    string serverIP = getLocalIP();

    cout << "====================================\n";
    cout << " NETWORK IP INFORMATION SERVER\n";
    cout << "====================================\n";
    cout << "Hostname: " << hostName << "\n";
    cout << "Server IPv4: " << serverIP << "\n";
    cout << "Port: 60000\n";
    cout << "Waiting for client connections...\n";

    while (true) {

        sockaddr_in clientAddress{};
        int clientLength = sizeof(clientAddress);

        SOCKET clientSocket = accept(
            serverSocket,
            (sockaddr*)&clientAddress,
            &clientLength
        );

        if (clientSocket == INVALID_SOCKET) {
            cout << "Client connection failed.\n";
            continue;
        }

        char clientIP[INET_ADDRSTRLEN] = {};

        inet_ntop(AF_INET, &clientAddress.sin_addr,
                  clientIP, sizeof(clientIP));

        cout << "\nClient connected!\n";
        cout << "Client IP: " << clientIP << "\n";
        cout << "Client Port: "
             << ntohs(clientAddress.sin_port) << "\n";

        string information =
            "SERVER HOSTNAME: " + string(hostName) +
            "\nSERVER IPv4: " + serverIP +
            "\nSERVER PORT: 60000" +
            "\nCONNECTION STATUS: SUCCESSFUL";

        send(clientSocket, information.c_str(),
             static_cast<int>(information.length()), 0);

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
