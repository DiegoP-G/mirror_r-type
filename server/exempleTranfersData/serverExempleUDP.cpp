#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include "../transferData/structTransfer.hpp"
#include "../transferData/transferData.hpp"

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // Création socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket failed"); return -1; }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        return -1;
    }

    std::cout << "Serveur UDP simplifié prêt sur port " << PORT << "\n";

    // Attente message
    auto [opcode, payload] = receiveFrameUDP(sockfd, cliaddr, len);
    std::cout << "Reçu opcode=" << (int)opcode << " payload=" << payload << "\n";

    // Réponse
    sendFrameUDP(sockfd, 42, "Hello UDP Client!", cliaddr, len);

    close(sockfd);
    return 0;
}
