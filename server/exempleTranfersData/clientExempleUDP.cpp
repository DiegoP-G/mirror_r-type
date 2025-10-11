#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>

#include "../../transferData/structTransfer.hpp"
#include "../../transferData/transferData.hpp"

#define SERVER_PORT 8081

int sendInTcp()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        return -1;
    }

    std::string buffer;

    sendFrameTCP(sock, 10, "hello");
    //  std::cout << "Message TCP envoyé au serveur.\n";

    close(sock);
    return 0;
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len = sizeof(servaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket failed");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendFrameUDP(sockfd, 80, "Ping depuis le client!", servaddr, len);
    // std::cout << "Message UDP envoyé au serveur.\n";

    sendInTcp();
    close(sockfd);
    return 0;
}
