// === Server.cpp ===
#include "../../transferData/structTransfer.hpp"
#include "../../transferData/transferData.hpp"
#include <arpa/inet.h>
#include <sstream>

#define PORT 8080

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        return -1;
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        return -1;
    }

    std::cout << "Serveur prêt, en attente de connexion...\n";

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept failed");
        return -1;
    }

    ship data(0, 10, 10);
    std::stringstream ss;
    data.serialize(ss);
    std::string serializedData = ss.str();

    sendFrameTCP(new_socket, OPCODE_SHIP_INFO, serializedData);
    std::cout << "Données envoyées au client\n";
    close(new_socket);
    close(server_fd);
    return 0;
}