
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
    serverAddress.sin_port = htons(57000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );

    cout << "====================================\n";
    cout << "       NETWORK CALCULATOR CLIENT\n";
    cout << "====================================\n";

    cout << "Connecting to calculator server...\n";

    // Connect to server
    if (connect(
        clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        cout << "Connection failed. Start CalculatorServer first.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected successfully!\n";

    double num1, num2;
    char operation;

    // Get numbers from user
    cout << "\nEnter the first number: ";
    cin >> num1;

    cout << "Enter operation (+, -, *, /): ";
    cin >> operation;

    cout << "Enter the second number: ";
    cin >> num2;

    // Prepare calculation request
    string request =
        to_string(num1) + " " +
        operation + " " +
        to_string(num2);

    // Send request to server
    int sent = send(
        clientSocket,
        request.c_str(),
        static_cast<int>(request.size()),
        0
    );

    if (sent == SOCKET_ERROR || sent == 0) {
        cout << "Failed to send calculation request.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Receive result from server
    char buffer[256] = {};

    int received = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (received > 0) {

        buffer[received] = '\0';

        cout << "\n====================================\n";
        cout << "          CALCULATION RESULT\n";
        cout << "====================================\n";

        cout << num1 << " " << operation << " " << num2
             << " = " << buffer << "\n";

    } else {
        cout << "Could not receive result from server.\n";
    }

    cout << "\nThank you for using Network Calculator!\n";

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
