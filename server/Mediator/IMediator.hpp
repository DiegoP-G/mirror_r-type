
#pragma once

#include "../Include/Errors/EventUnknown.hpp"
#include <string>

class IMediator
{
  public:
    virtual ~IMediator() = default;

    // Notify the mediator about an event
    virtual void notify(const int &event, const std::string &data = "", const std::string &lobbyUID = "",
                        int clientFd = -1) = 0;

    virtual void sendMessageToAll(const std::string &data) = 0;

  protected:
  private:
};
