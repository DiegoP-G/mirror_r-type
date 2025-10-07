#include "../../transferData/structTransfer.hpp"
#include "../../transferData/transferData.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

#define PORT 8080

int main()
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket failed");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        return -1;
    }

    // std::cout << "Serveur UDP prêt sur port " << PORT << "\n";

    // 1️⃣ Attente d’un message du client pour récupérer son adresse
    auto [recvOpcode, recvPayload] = receiveFrameUDP(sockfd, cliaddr, len);
    //  std::cout << "Message reçu du client : opcode=" << (int)recvOpcode << " payload=" << recvPayload << "\n";

    // 2️⃣ Préparation d’une réponse sérialisée
    ship data(0, 10, 10);
    std::stringstream ss;
    data.serialize(ss);

    // 3️⃣ Envoi de la réponse avec un opcode cohérent
    sendFrameUDP(sockfd, OPCODE_SHIP_INFO, ss.str(), cliaddr, len);

    //  std::cout << "Données envoyées au client\n";

    close(sockfd);
    return 0;
}
