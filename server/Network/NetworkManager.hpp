
#include "ClientManager.hpp"
#include "NetworkMediator.hpp"
#include <poll.h>
#include <string>
#include <memory>

#define SERVER_PORT 8080

class NetworkManager
{
  private:
    ClientManager _clientManager;
    NetworkMediator _mediator;
    int _tcpSocket;
    int _udpSocket;
    std::vector<pollfd> _fds;

  public:
    NetworkManager();
    ~NetworkManager();

    void setupSockets(int port);
    void setupPolls();
    void acceptClients();
    void receiveData();
    void disconnectClient(int clientSocket);
    void pollOnce();
};
