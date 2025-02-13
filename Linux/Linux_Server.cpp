#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#define BROADCAST_PORT 50000
#define SECRET_KEY "MySecretKey123"  // Simple security measure

std::string generate_response() {
    return "SERVER_IP:192.168.1.100|SIGNATURE:" + std::to_string(std::hash<std::string>{}(SECRET_KEY));
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(BROADCAST_PORT);

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    std::cout << "Server ready to respond to discovery requests...\n";

    while (true) {
        char buffer[1024];
        struct sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);

        int recvLen = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);
        if (recvLen > 0) {
            buffer[recvLen] = '\0';
            std::cout << "Received discovery request from " << inet_ntoa(clientAddr.sin_addr) << "\n";

            std::string response = generate_response();
            sendto(sock, response.c_str(), response.length(), 0, (struct sockaddr*)&clientAddr, addrLen);
        }
    }

    close(sock);
    return 0;
}
