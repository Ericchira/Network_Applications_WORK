
#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

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
    serverAddress.sin_port = htons(55000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "====================================\n";
    cout << "      NETWORK FILE SHARING SERVER\n";
    cout << "====================================\n";
    cout << "Port: 55000\n";
    cout << "Waiting for file sender...\n";

    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == INVALID_SOCKET) {
        cout << "Client connection failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Client connected successfully!\n";

    // Receive file name length
    unsigned int nameLength = 0;

    int received = recv(clientSocket,
                        reinterpret_cast<char*>(&nameLength),
                        sizeof(nameLength), MSG_WAITALL);

    if (received != sizeof(nameLength) ||
        nameLength == 0 || nameLength > 255) {
        cout << "Invalid file name information.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Receive file name
    string fileName(nameLength, '\0');

    received = recv(clientSocket, &fileName[0],
                    nameLength, MSG_WAITALL);

    if (received != static_cast<int>(nameLength)) {
        cout << "Could not receive file name.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Keep only the base file name
    size_t position = fileName.find_last_of("/\\");

    if (position != string::npos) {
        fileName = fileName.substr(position + 1);
    }

    if (fileName.empty() || fileName == "." || fileName == "..") {
        cout << "Invalid file name.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Receive file size
    unsigned long long fileSize = 0;

    received = recv(clientSocket,
                    reinterpret_cast<char*>(&fileSize),
                    sizeof(fileSize), MSG_WAITALL);

    if (received != sizeof(fileSize)) {
        cout << "Could not receive file size.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Create destination folder
    if (system("mkdir ReceivedFiles 2>nul") != 0) {
        cout << "Could not create destination folder.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    string outputPath = "ReceivedFiles/" + fileName;
    ofstream outputFile(outputPath, ios::binary);

    if (!outputFile) {
        cout << "Could not create received file.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Receiving file: " << fileName << "\n";
    cout << "File size: " << fileSize << " bytes\n";

    char buffer[4096];
    unsigned long long totalReceived = 0;

    while (totalReceived < fileSize) {
        int bytesToRead = static_cast<int>(
            min(static_cast<unsigned long long>(sizeof(buffer)),
                fileSize - totalReceived));

        int bytesReceived = recv(clientSocket, buffer, bytesToRead, 0);

        if (bytesReceived <= 0) {
            cout << "File transfer interrupted.\n";
            outputFile.close();
            closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        outputFile.write(buffer, bytesReceived);

        if (!outputFile) {
            cout << "File writing failed.\n";
            outputFile.close();
            closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        totalReceived += bytesReceived;
    }

    outputFile.close();

    cout << "\nFile received successfully!\n";
    cout << "Saved at: " << outputPath << "\n";

    const char* confirmation = "File received successfully!";
    send(clientSocket, confirmation,
         static_cast<int>(strlen(confirmation)), 0);

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    cout << "File sharing server stopped.\n";

    return 0;
}
