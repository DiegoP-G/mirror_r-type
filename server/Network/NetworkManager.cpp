#include "NetworkManager.hpp"
#include "NetworkMediator.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

NetworkManager::NetworkManager() : _networkMediator(*this)
{
}

NetworkManager::~NetworkManager()
{
}

void NetworkManager::updateAllPoll()
{
    //  _networkMediator.notify(const int &event)
}
