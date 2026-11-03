
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

vector<SOCKET> clients;
mutex clientsMutex;

void broadcastMessage(const string& message, SOCKET sender) {

    lock_guard<mutex> lock(clientsMutex);

    for (SOCKET client : clients) {

        if (client != sender) {

            int result = send(
                client,
                message.c_str(),
                static_cast<int>(message.length()),
                0
            );

            if (result == SOCKET_ERROR) {
                cout << "A message could not be delivered.\n";
            }
        }
    }
}

void handleClient(SOCKET clientSocket) {

    char buffer[1024];

    broadcastMessage(
        "A new user has joined the chat.\n",
        clientSocket
    );

    while (true) {

        int received = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (received <= 0) {
            break;
        }

        string message(buffer, received);

        if (message == "/exit") {
            break;
        }

        cout << "Received: " << message << "\n";

        broadcastMessage(message, clientSocket);
    }

    {
        lock_guard<mutex> lock(clientsMutex);

        clients.erase(
            remove(clients.begin(), clients.end(), clientSocket),
            clients.end()
        );
    }

    closesocket(clientSocket);

    broadcastMessage(
        "A user has left the chat.\n",
        INVALID_SOCKET
    );

    cout << "A client disconnected.\n";
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
    serverAddress.sin_port = htons(61000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        serverSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        cout << "Binding failed. Port 61000 may be in use.\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {

        cout << "Listening failed.\n";

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "====================================\n";
    cout << "       MULTI-CLIENT CHAT SERVER\n";
    cout << "====================================\n";
    cout << "Server Port: 61000\n";
    cout << "Waiting for users to join...\n";

    while (true) {

        SOCKET clientSocket = accept(
            serverSocket,
            nullptr,
            nullptr
        );

        if (clientSocket == INVALID_SOCKET) {
            cout << "Client connection failed.\n";
            continue;
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        cout << "A new client connected.\n";

        thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
