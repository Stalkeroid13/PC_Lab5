#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define HTML_CONTENT "<html><head><title>Simple Web Server</title></head><body><h1>Welcome to the Simple Web Server!</h1><p>This is a simple HTML page served by a C++ program.</p></body></html>"

using namespace std;

void sendResponse(SOCKET clientSocket, const string &response)
{
    send(clientSocket, response.c_str(), response.size(), 0);
}

void handleRequest(SOCKET clientSocket)
{
    string httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: " + to_string(strlen(HTML_CONTENT))
                          + "\r\n\r\n" + HTML_CONTENT;
    sendResponse(clientSocket, httpResponse);
    closesocket(clientSocket);
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET serverSocket;
    sockaddr_in serverAddr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port " << PORT << "...\n";

    while (true)
    {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr,
                                     &clientAddrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            cerr << "Accept failed\n";
            continue;
        }
        char client_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_addr, INET_ADDRSTRLEN);
        cout << "Connection accepted from " << client_addr << ":" <<
                ntohs(clientAddr.sin_port) << endl;
        handleRequest(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
