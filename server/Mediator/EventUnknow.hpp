#include <exception>
#include <string>

#pragma once

#include <exception>
#include <string>

class UnknownEventException : public std::exception {
    std::string message;

public:
    explicit UnknownEventException(int eventId)
        : message("Unknown event received by Mediator: " + std::to_string(eventId)) {}

    explicit UnknownEventException(const std::string& eventName)
        : message("Unknown event received by Mediator: " + eventName) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

