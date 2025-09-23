#include <string>

#pragma once

enum NetworkECSMediatorEvent {
    SendData,
    UpdateData
};

class NetworkECSMediator {
    private:
        // ECS
        // Sender
        // Receiver
    public:
        void notify(const NetworkECSMediator &event, const std::string &data);
};