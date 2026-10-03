#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define PORT "7080"
#define BUFLEN 512

// https://learn.microsoft.com/en-us/windows/win32/winsock/initializing-winsock

int test() {
    // Initialize Winsock
    WSADATA wsaData;
    int status = WSAStartup(MAKEWORD(2,2), &wsaData); // need to manually ensure version 2.2 for some reason
    if (status != 0) {
        printf("WSAStartup failed: %d\n", status);
        return 1;
    }

    struct addrinfo *result = NULL; 
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    status = getaddrinfo(NULL, PORT, &hints, &result);
    if (status != 0) {
        printf("getaddrinfo failed: %d\n", status);
        WSACleanup();
        return 1;
    }

    SOCKET sock = INVALID_SOCKET;
    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    status = bind(sock, result->ai_addr, (int)result->ai_addrlen);
    if (status == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // addrinfo needs to be cleaned up
    freeaddrinfo(result);

    // start listening
    if (listen(sock, 1) == SOCKET_ERROR) { // allow only 1 connection in the backlog
        printf("Listen failed with error: %d\n", WSAGetLastError() );
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // accept a client
    SOCKET client_sock;
    client_sock = accept(sock, NULL, NULL);
    if (client_sock == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // no need to keep the original socket anymore, we just want the one client socket
    closesocket(sock);

    // Receive until the peer shuts down the connection
    char buffer[BUFLEN];
    do {
        status = recv(client_sock, buffer, BUFLEN, 0);
        if (status > 0) {
            printf("Bytes received: %d\n", status);

            // placeholder action after receiving data (echoing it back)
            status = send(client_sock, buffer, status, 0);
            if (status == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(client_sock);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", status);
        } else if (status == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(client_sock);
            WSACleanup();
            return 1;
        }

    } while (status > 0);

    // final shutdown and cleanup
    status = shutdown(client_sock, SD_SEND); // shutdown can fail but there's nothing to do
    closesocket(client_sock);
    WSACleanup();

    return 0;
}