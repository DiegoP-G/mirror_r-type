// === Client.cpp ===
#include "../../transferData/structTransfer.hpp"
#include "../../transferData/transferData.hpp"
#include <arpa/inet.h>

#define PORT 8081

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

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

    std::cout << "Connecté au serveur\n";
    std::string buffer;

    sendFrameTCP(sock, 10, "hello");
    while (1)
    {
    }

    // std::tuple<uint8_t, std::string> receivedData = receiveFrameTCP(sock, buffer);
    // uint8_t opcode = std::get<0>(receivedData);
    // std::string payloadData = std::get<1>(receivedData);

    // if (opcode == OPCODE_SHIP_INFO)
    // {
    //     std::stringstream ss(payloadData);
    //     ship receivedTranferData;
    //     receivedTranferData.deserialize(ss);

    //     std::cout << "\n=== Données après désérialisation ===\n";
    //     receivedTranferData.print();
    // }

    close(sock);
    return 0;
}
