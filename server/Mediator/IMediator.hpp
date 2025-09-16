/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** IMediator
*/

#pragma once

#include <string>
#include "EventUnknow.hpp"

class IMediator {
    public:
          virtual ~IMediator() = default;

        // Notify the mediator about an event
        virtual void notify(void* sender, const int& event, const std::string& data) = 0;

        virtual void sendMessageToAll(const std::string& data) = 0;

    protected:
    private:
};


