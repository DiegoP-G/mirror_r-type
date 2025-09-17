#pragma once
#include <exception>
#include <string>

class ConfigurationException : public std::exception {
protected:
    std::string message;
public:
    explicit ConfigurationException(const std::string& msg) : message(msg) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class ConfigurationFileException : public ConfigurationException {
public:
    explicit ConfigurationFileException(const std::string& filePath)
        : ConfigurationException("Configuration file error: " + filePath) {}
};

class ConfigurationKeyNotFoundException : public ConfigurationException {
public:
    explicit ConfigurationKeyNotFoundException(const std::string& key)
        : ConfigurationException("Configuration key not found: " + key) {}
};

class ConfigurationTypeException : public ConfigurationException {
public:
    explicit ConfigurationTypeException(const std::string& key)
        : ConfigurationException("Type mismatch for configuration key: " + key) {}
};

class ConfigurationParseException : public ConfigurationException {
public:
    explicit ConfigurationParseException(const std::string& msg)
        : ConfigurationException("Configuration parse error: " + msg) {}
};
