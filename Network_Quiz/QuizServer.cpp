
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

// Send a complete message
bool sendLine(SOCKET sock, const string& message) {
    string data = message + "\n";
    int total = 0;

    while (total < static_cast<int>(data.size())) {
        int sent = send(sock, data.c_str() + total,
                        static_cast<int>(data.size()) - total, 0);

        if (sent <= 0) return false;
        total += sent;
    }

    return true;
}

// Receive a complete message
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

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        cout << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(56000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Binding failed. Port 56000 may already be in use.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        cout << "Listening failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "====================================\n";
    cout << "       NETWORK QUIZ SERVER\n";
    cout << "====================================\n";
    cout << "Port: 56000\n";
    cout << "Waiting for a quiz participant...\n";

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

    char clientIP[INET_ADDRSTRLEN] = {};

    inet_ntop(AF_INET, &clientAddress.sin_addr,
              clientIP, sizeof(clientIP));

    cout << "\nClient connected successfully!\n";
    cout << "Client IP address: " << clientIP << "\n";

    // Quiz questions
    const string questions[] = {
        "What does TCP stand for? | 1. Transmission Control Protocol | 2. Transfer Computer Program | 3. Technical Connection Process",
        "Which device connects different networks? | 1. Keyboard | 2. Router | 3. Monitor",
        "What does IP stand for? | 1. Internet Protocol | 2. Internal Program | 3. Information Port",
        "Which protocol is connection-oriented? | 1. UDP | 2. FTP | 3. TCP",
        "Which number identifies a network service? | 1. Port number | 2. Screen number | 3. File number"
    };

    const string correctAnswers[] = {
        "1", "2", "1", "3", "1"
    };

    const int totalQuestions = 5;

    int score = 0;
    string answer;
    bool completed = true;

    // Start quiz session
    if (!sendLine(clientSocket, "WELCOME") ||
        !sendLine(clientSocket, to_string(totalQuestions))) {
        cout << "Could not initialize quiz session.\n";
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Send questions and receive answers
    for (int i = 0; i < totalQuestions; i++) {

        cout << "\nSending question " << i + 1 << "...\n";

        if (!sendLine(clientSocket, questions[i])) {
            cout << "Could not send question. Client disconnected.\n";
            completed = false;
            break;
        }

        if (!receiveLine(clientSocket, answer)) {
            cout << "Client disconnected before answering.\n";
            completed = false;
            break;
        }

        cout << "Client answered: " << answer << "\n";

        if (answer == correctAnswers[i]) {
            score++;
            sendLine(clientSocket, "CORRECT");
            cout << "Answer is correct.\n";
        } else {
            sendLine(clientSocket, "WRONG");
            cout << "Answer is incorrect.\n";
        }
    }

    // Send final score only after all questions are answered
    if (completed) {
        sendLine(clientSocket, "FINAL");
        sendLine(clientSocket, to_string(score));
    }

    // Display results
    cout << "\n====================================\n";
    cout << "           QUIZ RESULTS\n";
    cout << "====================================\n";

    cout << "Score: " << score << " out of "
         << totalQuestions << "\n";

    if (score >= 4) {
        cout << "Performance: Excellent!\n";
    } else if (score >= 3) {
        cout << "Performance: Good!\n";
    } else {
        cout << "Performance: Keep practising!\n";
    }

    cout << "\nQuiz session ended.\n";

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
