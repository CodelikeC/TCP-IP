#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BROADCAST_PORT 50000
#define BROADCAST_IP "255.255.255.255"
#define MAX_RETRIES 3
#define TIMEOUT_SECONDS 2
#define SECRET_KEY "MySecretKey123"

bool verify_response(const std::string& response) {
    size_t pos = response.find("|SIGNATURE:");
    if (pos == std::string::npos) return false;

    std::string receivedHash = response.substr(pos + 11);
    std::string expectedHash = std::to_string(std::hash<std::string>{}(SECRET_KEY));

    return receivedHash == expectedHash;
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    struct sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(BROADCAST_PORT);
    broadcastAddr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    std::string message = "DISCOVER_SERVER";
    
    for (int i = 0; i < MAX_RETRIES; ++i) {
        sendto(sock, message.c_str(), message.length(), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        std::cout << "Attempt " << (i + 1) << " to discover server...\n";

        char buffer[1024];
        struct sockaddr_in serverAddr{};
        socklen_t addrLen = sizeof(serverAddr);

        if (recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen) > 0) {
            buffer[1023] = '\0';
            std::string response = buffer;
            if (verify_response(response)) {
                std::cout << "Valid server found at: " << response << "\n";
                break;
            }
        }

        sleep(TIMEOUT_SECONDS);
    }

    close(sock);
    return 0;
}
