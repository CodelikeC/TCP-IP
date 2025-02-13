#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

#define SERVER_IP "192.168.1.13" // IPv4 // 
#define SERVER_PORT 4433 // client port// 
#define AF_INET 2 

void InitOpenSSL() 
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX* CreateClientContext() 
{
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        std::cerr << "Failed to create SSL context.\n";
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    InitOpenSSL();
    SSL_CTX* ctx = CreateClientContext();

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) 
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, clientSocket);

    if (SSL_connect(ssl) <= 0) {
        std::cerr << "SSL handshake failed.\n";
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server securely!\n";
    SSL_read(ssl, buffer, sizeof(buffer));
    std::cout << "Server: " << buffer << "\n";

    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(clientSocket);
    SSL_CTX_free(ctx);
    WSACleanup();
    EVP_cleanup();

    return 0;
}

