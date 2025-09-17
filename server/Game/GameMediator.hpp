#pragma once
    #include <iostream>
    #include <string>
    #include <vector>
    #include "../Mediator/IMediator.hpp"
    #include "../Include/Errors/EventUnknown.hpp"


class GameLogic;
class CombatManager;
class ClientManager;
class NetworkMediator;



class GameMediator : public IMediator {
    enum GameMediatorEvent {
        PlayerDamaged,
        PlayerJoined,
        ChatMessage
    };
    std::string toString(GameMediatorEvent event) {
        switch (event) {
            case PlayerDamaged: return "PlayerDamaged";
            case PlayerJoined:  return "PlayerJoined";
            case ChatMessage:   return "ChatMessage";
            default:            return "UnknownEvent";
        }
    }

public:
    GameMediator()=default;
    ~GameMediator()=default;

    void notify(void* sender, const int& event, const std::string& data) override;
    inline void sendMessageToAll(const std::string& data) {return;};


};
