#include <iostream> 
using namespace std; 
#include <cstring> 
#include <unistd.h>
#include <winsock2.h> 
#include <openssl/evp.h> 
#include <openssl/rand.h> 

#include <stdio.h>

#include <ws2tcpip.h> 

// for windows // 

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Libcrypto.lib")

#define BROADCAST_PORT 50000 
#define aes_key "0123456789abcdef" // 16 bytes key for aes - 128 
#define aes_iv "abcedef0123456789" // 16 byte IV (vector)

void encrypt_message(const string &plaintext, string &ciphertext)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new(); 
    unsigned char encrypted [1024]; 
    int len; 
    int cipher_text_len; 

    EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, (unsigned char*)aes_key, (unsigned char *)aes_iv) ;
    EVP_EncryptUpdate(ctx, encrypted, &len, (unsigned char *)plaintext.c_str(), plaintext.length());
    cipher_text_len =  len; 

    EVP_EncryptFinal_ex (ctx, encrypted + len,  &len); 
    EVP_CIPHER_CTX_free(ctx);
}

int main ()
{
    cout <<"Improved client with windows and openSSL" << endl; 
    WSADATA wsad;
    SOCKET serversocket; 
    sockaddr_in serverAddr, clientAddr; // sever address,, client address; 
    char buffer[1024];
    int client_len = sizeof(clientAddr);
    if (WSAStartup(MAKEWORD(2,2), &wsad))
    {
        cout <<"Wsa start up was failed" << endl; 
        return 1 ; 
    }

    serversocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
    if (serversocket == INVALID_SOCKET)
    {
        cout <<"Socket creations was failed" <<endl; 
        cout << WSAGetLastError() << endl; 
        WSACleanup(); 
        return 1 ; 
    } 
    
    cout <<"UDP discovery server is... running!" << endl;
    
    while (true)
    {
        int recvlen = recvfrom(serversocket, buffer, sizeof(buffer), 0,(struct sockaddr*) &clientAddr, &client_len); 
        if (recvlen > 0)
        {
            buffer[recvlen] = '/0'; 
            cout <<"Recieved request from" << inet_ntoa(clientAddr.sin_addr) << endl; 
        }

        string response  = "Server_IP : 192.168.1.100";
        string encryptedResponse; 
        encrypt_message(response, encryptedResponse); 
        
        sendto(serversocket, encryptedResponse.c_str(), encryptedResponse.size(), 0, (struct sockaddr*) &clientAddr, client_len); 

    }

    closesocket(serversocket); 
    WSACleanup(); 
    return 0; 
}
