
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

// Send a complete message to the server
bool sendLine(SOCKET sock, const string& message) {
    string data = message + "\n";
    int total = 0;

    while (total < static_cast<int>(data.size())) {
        int sent = send(
            sock,
            data.c_str() + total,
            static_cast<int>(data.size()) - total,
            0
        );

        if (sent <= 0) return false;
        total += sent;
    }

    return true;
}

// Receive a complete message from the server
bool receiveLine(SOCKET sock, string& message) {
    message.clear();
    char ch;

    while (true) {
        int received = recv(sock, &ch, 1, 0);

        if (received <= 0) return false;
        if (ch == '\n') break;
        if (ch != '\r') message += ch;

        if (message.size() > 1000) return false;
    }

    return true;
}

int main() {

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock startup failed.\n";
        return 1;
    }

    // Create client socket
    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (clientSocket == INVALID_SOCKET) {
        cout << "Could not create client socket.\n";
        WSACleanup();
        return 1;
    }

    // Server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(56000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );

    cout << "====================================\n";
    cout << "        NETWORK QUIZ CLIENT\n";
    cout << "====================================\n";

    cout << "Connecting to quiz server...\n";

    // Connect to server
    if (connect(
        clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        cout << "Connection failed. Start QuizServer first.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected successfully!\n";
    cout << "Server IP address: 127.0.0.1\n";

    string message;
    string totalText;

    // Receive welcome message and number of questions
    if (!receiveLine(clientSocket, message) ||
        !receiveLine(clientSocket, totalText)) {

        cout << "Could not receive quiz information.\n";

        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    int totalQuestions = stoi(totalText);

    cout << "\nWelcome to the Network Quiz!\n";
    cout << "There are " << totalQuestions << " questions.\n";

    int answered = 0;
    bool completed = true;

    // Answer quiz questions
    for (int i = 0; i < totalQuestions; i++) {

        if (!receiveLine(clientSocket, message)) {
            cout << "Connection lost while receiving a question.\n";
            completed = false;
            break;
        }

        cout << "\n------------------------------------\n";
        cout << "QUESTION " << i + 1
             << " OF " << totalQuestions << "\n";
        cout << "------------------------------------\n";

        cout << message << "\n";

        string answer;

        cout << "\nEnter your answer (1, 2 or 3): ";
        getline(cin, answer);

        // Validate answer
        while (answer != "1" &&
               answer != "2" &&
               answer != "3") {

            cout << "Invalid choice. Enter 1, 2 or 3: ";
            getline(cin, answer);
        }

        // Send answer to server
        if (!sendLine(clientSocket, answer)) {
            cout << "Could not send your answer.\n";
            completed = false;
            break;
        }

        // Receive feedback
        if (!receiveLine(clientSocket, message)) {
            cout << "Could not receive answer feedback.\n";
            completed = false;
            break;
        }

        answered++;

        if (message == "CORRECT") {
            cout << "Correct answer! Well done.\n";
        } else {
            cout << "Incorrect answer. Try the next question.\n";
        }
    }

    // Receive final score
    if (completed && answered == totalQuestions) {

        string finalMessage;
        string scoreText;

        if (receiveLine(clientSocket, finalMessage) &&
            finalMessage == "FINAL" &&
            receiveLine(clientSocket, scoreText)) {

            cout << "\n====================================\n";
            cout << "             QUIZ RESULT\n";
            cout << "====================================\n";

            cout << "Your final score: " << scoreText
                 << " out of " << totalQuestions << "\n";

        } else {
            cout << "Could not receive the final score.\n";
        }
    }

    cout << "\nThank you for participating!\n";

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
