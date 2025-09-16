#pragma once
#include "network_messages.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <functional>

class UDPSocket {
private:
    int socketFd;
    struct sockaddr_in address;
    bool isServer;
    
public:
    UDPSocket() : socketFd(-1), isServer(false) {
        memset(&address, 0, sizeof(address));
    }
    
    ~UDPSocket() {
        close();
    }
    
    bool createServer(int port) {
        socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketFd < 0) {
            return false;
        }
        
        // Set socket to non-blocking
        int flags = fcntl(socketFd, F_GETFL, 0);
        fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);
        
        // Allow address reuse
        int opt = 1;
        setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(socketFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            ::close(socketFd);
            socketFd = -1;
            return false;
        }
        
        isServer = true;
        return true;
    }
    
    bool createClient(const std::string& serverIP, int serverPort) {
        socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketFd < 0) {
            return false;
        }
        
        // Set socket to non-blocking
        int flags = fcntl(socketFd, F_GETFL, 0);
        fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);
        
        address.sin_family = AF_INET;
        address.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverIP.c_str(), &address.sin_addr);
        
        isServer = false;
        return true;
    }
    
    bool sendMessage(const NetworkMessage& message, const struct sockaddr_in* targetAddr = nullptr) {
        auto data = message.serialize();
        
        const struct sockaddr_in* addr = targetAddr ? targetAddr : &address;
        
        ssize_t sent = sendto(socketFd, data.data(), data.size(), 0, 
                             (const struct sockaddr*)addr, sizeof(*addr));
        
        return sent == static_cast<ssize_t>(data.size());
    }
    
    bool receiveMessage(NetworkMessage& message, struct sockaddr_in& senderAddr) {
        uint8_t buffer[1024];
        socklen_t addrLen = sizeof(senderAddr);
        
        ssize_t received = recvfrom(socketFd, buffer, sizeof(buffer), 0,
                                   (struct sockaddr*)&senderAddr, &addrLen);
        
        if (received > 0) {
            message = NetworkMessage::deserialize(buffer, received);
            return true;
        }
        
        return false;
    }
    
    void close() {
        if (socketFd >= 0) {
            ::close(socketFd);
            socketFd = -1;
        }
    }
    
    bool isValid() const {
        return socketFd >= 0;
    }
};