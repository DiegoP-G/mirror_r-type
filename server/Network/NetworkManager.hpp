
#include "ClientManager.hpp"
#include "NetworkMediator.hpp"
#include <string>

class ServerNetwork {
private:
    ClientManager clientManager;
    NetworkMediator* mediator;
    int listenSocket;

public:
    ServerNetwork(NetworkMediator* med);
    ~ServerNetwork();

    bool startListening(int port);
    void acceptClients();
    void receiveData();
    void sendData(int clientSocket, const std::string& data);
    void disconnectClient(int clientSocket);
    void receive(NetworkMediatorEvent type, const std::string& data);
};
