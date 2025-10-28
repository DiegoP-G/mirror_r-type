#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

volatile bool running = true;

void signalHandler(int signum)
{
    running = false;
}

struct ClientInfo
{
    sockaddr_in address;
    std::string name;
};

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);

    int port = 5555;
    if (argc > 1)
    {
        port = std::atoi(argv[1]);
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Erreur WSAStartup" << std::endl;
        return 1;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Erreur création socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "❌ Erreur bind sur le port " << port << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << " Serveur VoIP démarré sur le port " << port << std::endl;
    std::cout << "En attente de clients..." << std::endl;

    std::map<std::string, ClientInfo> clients;
    char buffer[4096];

    while (running)
    {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int recvLen = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr *)&clientAddr, &clientLen);

        if (recvLen == SOCKET_ERROR)
        {
            continue;
        }

        std::string clientKey =
            std::string(inet_ntoa(clientAddr.sin_addr)) + ":" + std::to_string(ntohs(clientAddr.sin_port));

        // Message de contrôle (commence par "HELLO:")
        if (recvLen > 6 && std::strncmp(buffer, "HELLO:", 6) == 0)
        {
            std::string name(buffer + 6, recvLen - 6);
            clients[clientKey] = {clientAddr, name};
            std::cout << "✓ Nouveau client: " << name << " (" << clientKey << ")" << std::endl;
            std::cout << "  Clients connectés: " << clients.size() << std::endl;
            continue;
        }

        // Message de déconnexion
        if (recvLen == 3 && std::strncmp(buffer, "BYE", 3) == 0)
        {
            if (clients.find(clientKey) != clients.end())
            {
                std::cout << "✗ Déconnexion: " << clients[clientKey].name << std::endl;
                clients.erase(clientKey);
            }
            continue;
        }

        // Données audio : relayer à tous les autres clients
        int sentCount = 0;
        for (const auto &[key, client] : clients)
        {
            if (key != clientKey)
            { // Ne pas renvoyer à l'émetteur
                sendto(serverSocket, buffer, recvLen, 0, (sockaddr *)&client.address, sizeof(client.address));
                sentCount++;
            }
        }

        if (sentCount > 0)
        {
            std::string senderName = (clients.find(clientKey) != clients.end()) ? clients[clientKey].name : "Inconnu";
            std::cout << "🎵 Audio reçu de " << senderName << " (" << recvLen << " bytes) -> relayé à " << sentCount
                      << " client(s)" << std::endl;
        }
    }

    std::cout << "\n Arrêt du serveur..." << std::endl;
    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}