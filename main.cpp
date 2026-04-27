#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <ws2tcpip.h>
#include <thread>
#include <fstream>
#include <sstream>

#define PORT 8080
#define HTML_CONTENT "<html><head><title>Simple Web Server</title></head><body><h1>Welcome to the Simple Web Server!</h1><p>This is a simple HTML page served by a C++ program.</p></body></html>"

using namespace std;


// Функція для зчитування всього вмісту файлу в рядок
string getFileContent(const string &fileName)
{
    ifstream file(fileName, ios::binary);
    if (!file.is_open())
    {
        return ""; // Повертаємо пустий рядок, якщо файлу немає
    }
    // Ефективний спосіб зчитати весь файл через ітератори
    return string(istreambuf_iterator(file), istreambuf_iterator<char>());
}

void sendResponse(SOCKET clientSocket, const string &response)
{
    send(clientSocket, response.c_str(), response.size(), 0);
}

void handleRequest(SOCKET clientSocket)
{
    char buffer[2048];
    // Отримуємо сирий HTTP-запит від браузера
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0)
    {
        cerr << "0 bytes received\n";
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';
    char method[10], path[256], protocol[20];

    // Розбиваємо перший рядок: "GET /index.html HTTP/1.1"
    if (sscanf(buffer, "%s %s %s", method, path, protocol) >= 2)
    {
        auto requestedPath = string(path);

        // Обробка кореневого запиту
        if (requestedPath == "/")
            requestedPath = "/index.html";

        // Перетворюємо мережевий шлях у локальне ім'я файлу
        string fileName = requestedPath.substr(1);

        // Пробуємо отримати контент
        string content = getFileContent(fileName);
        string httpResponse;

        // Файл не знайдено - віддаємо 404
        if (content.empty())
        {
            httpResponse = "HTTP/1.1 404 Not Found\r\n" "Content-Length: 0\r\n" "Connection: close\r\n\r\n";
            cout << "404 Not Found: " << fileName << endl;
        }
        else
        {
            // Файл знайдено — формуємо успішну відповідь
            httpResponse = "HTTP/1.1 200 OK\r\n" "Content-Type: text/html\r\n" "Content-Length: "
                           + to_string(content.size()) + "\r\n" "Connection: close\r\n\r\n" + content;
            cout << "200 OK: served " << fileName << endl;
        }

        send(clientSocket, httpResponse.c_str(), httpResponse.size(), 0);
    }

    closesocket(clientSocket);
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sockaddr_in serverAddr{};
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed!\n";
        WSACleanup();
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed!\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        cerr << "Listen failed!\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port " << PORT << "...\n";

    while (true)
    {
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientAddrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            cerr << "Accept failed!\n";
            continue;
        }

        thread(handleRequest, clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
