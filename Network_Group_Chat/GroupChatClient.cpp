
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

atomic<bool> connected(true);

// Receives messages from the server
void receiveMessages(SOCKET clientSocket) {

    char buffer[1024];

    while (connected) {

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

        cout << "\n" << message;
        cout << "\nYou: ";
        cout.flush();
    }

    connected = false;
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
    serverAddress.sin_port = htons(61000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );

    cout << "====================================\n";
    cout << "        GROUP CHAT CLIENT\n";
    cout << "====================================\n";

    cout << "Connecting to chat server...\n";

    if (connect(
        clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        cout << "Connection failed.\n";
        cout << "Make sure the server is running.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to the group chat!\n";
    cout << "Type a message and press Enter.\n";
    cout << "Type /exit to leave the chat.\n\n";

    thread receiver(receiveMessages, clientSocket);

    string message;

    while (connected) {

        cout << "You: ";

        if (!getline(cin, message)) {
            break;
        }

        if (message == "/exit") {

            send(clientSocket, "/exit", 5, 0);
            break;
        }

        if (!connected) {
            break;
        }

        message += "\n";

        int sent = send(
            clientSocket,
            message.c_str(),
            static_cast<int>(message.length()),
            0
        );

        if (sent == SOCKET_ERROR) {
            cout << "Message sending failed.\n";
            break;
        }
    }

    connected = false;

    shutdown(clientSocket, SD_BOTH);

    if (receiver.joinable()) {
        receiver.join();
    }

    closesocket(clientSocket);
    WSACleanup();

    cout << "\nYou have left the group chat.\n";

    return 0;
}
