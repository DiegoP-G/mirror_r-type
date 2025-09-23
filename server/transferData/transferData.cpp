#include "structTransfer.hpp"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <tuple>
#include <unistd.h>

void sendFrameTCP(int socket, uint8_t opcode, const std::string& payload)
{
    std::vector<uint8_t> frame;

    frame.push_back(opcode);

    size_t payloadLen = payload.size();
    if (payloadLen <= 253) {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    } else if (payloadLen <= 65535) {
        frame.push_back(254);
        frame.push_back((payloadLen >> 8) & 0xFF);
        frame.push_back(payloadLen & 0xFF);
    } else {
        frame.push_back(255);
        for (int i = 7; i >= 0; --i)
            frame.push_back((payloadLen >> (8 * i)) & 0xFF);
    }

    frame.insert(frame.end(), payload.begin(), payload.end());
    write(socket, frame.data(), frame.size());
}
std::tuple<uint8_t, std::string> receiveFrameTCP(int socket, std::string& buffer)
{
    char temp[32000];

    // Lire des données jusqu’à avoir au moins 2 octets pour l’en-tête
    while (buffer.size() < 2) {
        ssize_t bytesRead = read(socket, temp, sizeof(temp));
        if (bytesRead <= 0)
            return std::tuple<uint8_t, std::string>(OPCODE_CLOSE_CONNECTION, "");
        buffer.append(temp, bytesRead);
    }

    uint8_t opcode = buffer[0];
    uint8_t payloadLenByte = buffer[1];
    uint64_t payloadLen = payloadLenByte;
    size_t offset = 2;

    if (payloadLenByte == 254) {
        if (buffer.size() < offset + 2)
            return std::tuple<uint8_t, std::string>(OPCODE_INCOMPLETE_DATA, "");

        payloadLen = ((uint8_t)buffer[offset] << 8) | (uint8_t)buffer[offset + 1];
        offset += 2;
    } else if (payloadLenByte == 255) {
        if (buffer.size() < offset + 8)
            return std::tuple<uint8_t, std::string>(OPCODE_INCOMPLETE_DATA, "");

        payloadLen = 0;
        for (int i = 0; i < 8; ++i) {
            payloadLen = (payloadLen << 8) | (uint8_t)buffer[offset + i];
        }
        offset += 8;
    }

    // S'assurer que tout le payload est là
    if (buffer.size() < offset + payloadLen) {
        // Lire plus si nécessaire
        while (buffer.size() < offset + payloadLen) {
            ssize_t bytesRead = read(socket, temp, sizeof(temp));
            if (bytesRead <= 0)
                return std::tuple<uint8_t, std::string>(OPCODE_CLOSE_CONNECTION, "");
            buffer.append(temp, bytesRead);
        }
    }

    std::string payload = buffer.substr(offset, payloadLen);
    buffer.erase(0, offset + payloadLen); // Consomme les données utilisées

    return std::tuple<uint8_t, std::string>(opcode, payload);
}

void sendFrameUDP(int sockfd, uint8_t opcode, const std::string& payload,
                  const struct sockaddr_in& addr, socklen_t addrlen)
{
    std::vector<uint8_t> frame;
    frame.push_back(opcode);                 // 1 octet : opcode
    frame.insert(frame.end(), payload.begin(), payload.end()); // payload brut

    sendto(sockfd, frame.data(), frame.size(), 0,
           (const struct sockaddr*)&addr, addrlen);
}

std::tuple<uint8_t, std::string> receiveFrameUDP(int sockfd,
                                                 struct sockaddr_in& addr,
                                                 socklen_t& addrlen)
{
    char buffer[32000];
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                         (struct sockaddr*)&addr, &addrlen);

    if (n <= 0)
        return {OPCODE_CLOSE_CONNECTION, ""};

    uint8_t opcode = buffer[0];
    std::string payload(buffer + 1, n - 1);
    return {opcode, payload};
}


