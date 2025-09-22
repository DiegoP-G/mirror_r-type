#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>

#include "../transferData/structTransfer.hpp"
#include "../transferData/transferData.hpp"

#define SERVER_PORT 8080

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

    // 1️⃣ Envoi d’un message de "ping"
    sendFrameUDP(sockfd, 80, "Ping depuis le client!", servaddr, len);
    std::cout << "Message envoyé au serveur.\n";

    // 2️⃣ Attente d’une réponse
    auto [opcode, payload] = receiveFrameUDP(sockfd, servaddr, len);

    // 3️⃣ Vérifie bien l’opcode attendu
    if (opcode == OPCODE_SHIP_INFO)
    {
        std::stringstream ss(payload);
        ship receivedTransferData;
        receivedTransferData.deserialize(ss);

        std::cout << "\n=== Données après désérialisation ===\n";
        receivedTransferData.print();
    }
    else
    {
        std::cout << "Réponse inattendue : opcode=" << (int)opcode << "\n";
    }

    close(sockfd);
    return 0;
}
