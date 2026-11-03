
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main() {

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock startup failed.\n";
        return 1;
    }

    // Create server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        cout << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(57000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Binding failed. Port 57000 may already be in use.\n";
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
    cout << "       NETWORK CALCULATOR SERVER\n";
    cout << "====================================\n";
    cout << "Port: 57000\n";
    cout << "Waiting for calculator client...\n";

    // Accept client connection
    sockaddr_in clientAddress{};
    int clientLength = sizeof(clientAddress);

    SOCKET clientSocket = accept(
        serverSocket,
        (sockaddr*)&clientAddress,
        &clientLength
    );

    if (clientSocket == INVALID_SOCKET) {
        cout << "Client connection failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "\nClient connected successfully!\n";

    string request;
    char buffer[256];

    // Receive calculation request
    int received = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (received <= 0) {
        cout << "Could not receive calculation request.\n";
    } else {

        buffer[received] = '\0';
        request = buffer;

        double num1, num2;
        char operation;

        stringstream input(request);

        if (!(input >> num1 >> operation >> num2)) {
            cout << "Invalid calculation request.\n";
            string error = "ERROR: Invalid calculation request.";
            send(clientSocket, error.c_str(),
                 static_cast<int>(error.size()), 0);
        } else {

            double result = 0;
            bool valid = true;

            cout << "Received: " << num1 << " "
                 << operation << " " << num2 << "\n";

            // Perform calculation
            switch (operation) {

                case '+':
                    result = num1 + num2;
                    break;

                case '-':
                    result = num1 - num2;
                    break;

                case '*':
                    result = num1 * num2;
                    break;

                case '/':
                    if (num2 == 0) {
                        valid = false;
                        cout << "Error: Division by zero.\n";
                    } else {
                        result = num1 / num2;
                    }
                    break;

                default:
                    valid = false;
                    cout << "Invalid operation.\n";
            }

            string response;

            if (valid) {
                ostringstream output;
                output << setprecision(12) << result;
                response = output.str();

                cout << "Result: " << response << "\n";
            } else {
                response = "ERROR: Invalid operation or division by zero.";
            }

            // Send result to client
            send(
                clientSocket,
                response.c_str(),
                static_cast<int>(response.size()),
                0
            );
        }
    }

    cout << "\nCalculator session ended.\n";

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
