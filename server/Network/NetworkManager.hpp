
#include "ClientManager.hpp"
#include "NetworkMediator.hpp"
#include <string>

class NetworkManager {
private:
    ClientManager _clientManager;
    NetworkMediator* _mediator;
    int _tcpSocket;
    int _udpSocket;

public:
    NetworkManager(NetworkMediator* med);
    ~NetworkManager();

    bool startListening(int port);
    void acceptClients();
    void receiveData();
    void disconnectClient(int clientSocket);
    void receive(NetworkMediatorEvent type, const std::string& data);
};
