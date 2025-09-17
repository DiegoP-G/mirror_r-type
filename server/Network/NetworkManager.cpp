#include "NetworkManager.hpp"
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>     // For close()
#include <netinet/in.h>

ServerNetwork::ServerNetwork(NetworkMediator* med)
    : mediator(med), listenSocket(-1) {}

ServerNetwork::~ServerNetwork() {
    if (listenSocket != -1) {
        close(listenSocket);
    }
}

bool ServerNetwork::startListening(int port) {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        std::cerr << "Failed to create listen socket\n";
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return false;
    }

    if (listen(listenSocket, 5) < 0) {
        std::cerr << "Listen failed\n";
        return false;
    }

    std::cout << "Server listening on port " << port << "\n";
    return true;
}

void ServerNetwork::acceptClients() {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientSock = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientLen);

    if (clientSock < 0) {
        std::cerr << "Failed to accept client\n";
        return;
    }

    Client newClient("Player", clientSock);
    newClient.setSocket(clientSock);
    newClient.setConnected(true);

    clientManager.addClient(newClient);

    std::cout << "Accepted new client, socket: " << clientSock << "\n";
}

void ServerNetwork::receiveData() {
    auto& clients = clientManager.getClientsMap();

    char buffer[1024];
    for (auto& [sock, client] : clients) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string msg(buffer);

            receive(NetworkMediatorEvent::TCP, msg);
        }
    }
}

void ServerNetwork::sendData(int clientSocket, const std::string& data) {
    Client* client = clientManager.getClient(clientSocket);
    if (client) {
        client->sendMessage(data);
    }
}

void ServerNetwork::disconnectClient(int clientSocket) {
    clientManager.removeClient(clientSocket);
    close(clientSocket);
    std::cout << "Disconnected client: " << clientSocket << "\n";
}

void ServerNetwork::receive(NetworkMediatorEvent type, const std::string& data) {
    if (mediator) {
        mediator->notify(static_cast<void*>(this), static_cast<int>(type), data);
    }

    std::cout << "[ServerNetwork] Received (" 
              << (type == NetworkMediatorEvent::TCP ? "TCP" : "UDP") 
              << "): " << data << "\n";
}
