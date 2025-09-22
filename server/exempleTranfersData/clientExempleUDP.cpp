#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>

// Déclarations des fonctions qu'on a déjà faites
void sendFrameUDP(int sockfd, uint8_t opcode, const std::string &payload,
                  const struct sockaddr_in &addr, socklen_t addrlen);

std::tuple<uint8_t, std::string>
receiveFrameUDP(int sockfd, struct sockaddr_in &addr, socklen_t &addrlen);

#define SERVER_PORT 8080

int main() {
  int sockfd;
  struct sockaddr_in servaddr;
  socklen_t len = sizeof(servaddr);

  // Création socket UDP
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("socket failed");
    return -1;
  }

  // Remplissage de l'adresse serveur
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_PORT);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

  // Envoi d’un message au serveur
  sendFrameUDP(sockfd, 1, "Ping depuis le client!", servaddr, len);
  std::cout << "Message envoyé au serveur.\n";

  // Attente de la réponse
  auto [opcode, payload] = receiveFrameUDP(sockfd, servaddr, len);
  std::cout << "Réponse du serveur: opcode=" << (int)opcode << " payload=\""
            << payload << "\"\n";

  close(sockfd);
  return 0;
}
