
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;
namespace fs = std::filesystem;

bool sendAll(SOCKET socket, const char* data, int length) {
    int totalSent = 0;

    while (totalSent < length) {
        int sent = send(socket, data + totalSent,
                        length - totalSent, 0);

        if (sent == SOCKET_ERROR || sent == 0) {
            return false;
        }

        totalSent += sent;
    }

    return true;
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
    serverAddress.sin_port = htons(55000);

    // Connect to server on the same laptop
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    cout << "====================================\n";
    cout << "      NETWORK FILE SHARING CLIENT\n";
    cout << "====================================\n";
    cout << "Connecting to server...\n";

    if (connect(clientSocket, (sockaddr*)&serverAddress,
                sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Connection failed. Start FileServer first.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected successfully!\n";

    string filePath;

    cout << "Enter the full path of the file to send: ";
    getline(cin, filePath);

    if (filePath.size() >= 2 &&
        filePath.front() == '"' && filePath.back() == '"') {
        filePath = filePath.substr(1, filePath.size() - 2);
    }

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        cout << "File does not exist or is not a regular file.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    string fileName = fs::path(filePath).filename().string();
    unsigned long long fileSize = fs::file_size(filePath);

    if (fileName.empty() || fileName.size() > 255) {
        cout << "Invalid file name.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    ifstream inputFile(filePath, ios::binary);

    if (!inputFile) {
        cout << "Could not open the file.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Send file name length
    unsigned int nameLength =
        static_cast<unsigned int>(fileName.size());

    bool success = sendAll(
        clientSocket,
        reinterpret_cast<const char*>(&nameLength),
        sizeof(nameLength));

    // Send file name
    if (success) {
        success = sendAll(clientSocket, fileName.data(),
                          static_cast<int>(nameLength));
    }

    // Send file size
    if (success) {
        success = sendAll(
            clientSocket,
            reinterpret_cast<const char*>(&fileSize),
            sizeof(fileSize));
    }

    if (!success) {
        cout << "Could not send file information.\n";
        inputFile.close();
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Sending file: " << fileName << "\n";

    char buffer[4096];
    unsigned long long totalSent = 0;

    while (inputFile) {
        inputFile.read(buffer, sizeof(buffer));
        streamsize bytesRead = inputFile.gcount();

        if (bytesRead <= 0) {
            break;
        }

        if (!sendAll(clientSocket, buffer,
                     static_cast<int>(bytesRead))) {
            cout << "File transfer failed.\n";
            inputFile.close();
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        totalSent += static_cast<unsigned long long>(bytesRead);
    }

    inputFile.close();

    if (totalSent != fileSize) {
        cout << "File transfer incomplete.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "File sent successfully!\n";
    cout << "Waiting for server confirmation...\n";

    char response[256] = {};
    int bytesReceived = recv(clientSocket, response,
                             sizeof(response) - 1, 0);

    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        cout << "Server: " << response << "\n";
    }

    closesocket(clientSocket);
    WSACleanup();

    cout << "File sharing client stopped.\n";

    return 0;
}
