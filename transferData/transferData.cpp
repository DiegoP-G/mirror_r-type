#include "transferData.hpp"
#include "opcode.hpp"
#include <cstring>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <iostream> // For std::cerr in perror replacement
#include <winsock2.h>
#include <ws2tcpip.h>
#define ssize_t int
#define write(fd, buf, len) send(fd, (const char *)(buf), len, 0)
#define read(fd, buf, len) recv(fd, (char *)(buf), len, 0)
#define perror(msg)                                                                                                    \
    {                                                                                                                  \
        int err = WSAGetLastError();                                                                                   \
        std::cerr << msg << ": " << err << std::endl;                                                                  \
    }
#define EAGAIN WSAEWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#include <windows.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <tuple>
#include <vector>

#ifdef COMPRESSION_ENABLED
#include <zlib.h>
#endif

bool sendFrameTCP(SOCKET fd, uint8_t opcode, const std::string &payload)
{
    std::vector<uint8_t> frame;

    frame.push_back(opcode);

    size_t payloadLen = payload.size();
    if (payloadLen <= 253)
    {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    }
    else if (payloadLen <= 65535)
    {
        frame.push_back(254);
        frame.push_back((payloadLen >> 8) & 0xFF);
        frame.push_back(payloadLen & 0xFF);
    }
    else
    {
        frame.push_back(255);
        for (int i = 7; i >= 0; --i)
            frame.push_back((payloadLen >> (8 * i)) & 0xFF);
    }

    frame.insert(frame.end(), payload.begin(), payload.end());

    // Envoi avec gestion des écritures partielles
    ssize_t sent = 0;
    ssize_t total = frame.size();

    while (sent < total)
    {
        ssize_t n = write(fd, frame.data(), total - sent);

        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Socket buffer plein, on réessaiera plus tard
                std::cout << "[TCP] Socket " << fd << " buffer full (EAGAIN), will retry later\n";
                return false;
            }
            perror("write failed");
            return false;
        }
        sent += n;
    }

    std::cout << "[TCP] Sent frame (opcode: " << static_cast<int>(opcode) << ", payload size: " << payloadLen
              << ", total frame size: " << total << ") to client " << fd << "\n";
    return true;
}

std::tuple<uint8_t, std::string> receiveFrameTCP(SOCKET socket, std::string &buffer)
{
    // Ensure we have at least 2 bytes for header
    while (buffer.size() < 2)
    {
        char temp[4096];
        ssize_t bytesRead = read(socket, temp, sizeof(temp));
        if (bytesRead < 0)
        {
            perror("read failed");
            return {OPCODE_CLOSE_CONNECTION, ""};
        }
        if (bytesRead == 0)
        {
            if (buffer.empty())
                return {OPCODE_CLOSE_CONNECTION, ""}; // connection closed
            else
                continue; // wait for more data
        }
        buffer.append(temp, bytesRead);
    }

    uint8_t opcode = buffer[0];
    uint8_t payloadLenByte = buffer[1];
    size_t offset = 2;
    uint64_t payloadLen = payloadLenByte;

    if (payloadLenByte == 254)
    {
        while (buffer.size() < 4)
        {
            char temp[4096];
            ssize_t bytesRead = read(socket, temp, sizeof(temp));
            if (bytesRead <= 0)
            {
                if (bytesRead == 0 && buffer.size() < 4)
                    continue; // wait for more data
                perror("read failed");
                return {OPCODE_CLOSE_CONNECTION, ""};
            }
            buffer.append(temp, bytesRead);
        }
        payloadLen = ((uint8_t)buffer[2] << 8) | (uint8_t)buffer[3];
        offset = 4;
    }
    else if (payloadLenByte == 255)
    {
        while (buffer.size() < 10)
        {
            char temp[4096];
            ssize_t bytesRead = read(socket, temp, sizeof(temp));
            if (bytesRead <= 0)
            {
                if (bytesRead == 0 && buffer.size() < 10)
                    continue;
                perror("read failed");
                return {OPCODE_CLOSE_CONNECTION, ""};
            }
            buffer.append(temp, bytesRead);
        }
        payloadLen = 0;
        for (int i = 0; i < 8; ++i)
            payloadLen = (payloadLen << 8) | (uint8_t)buffer[2 + i];
        offset = 10;
    }

    // Wait until full payload is available
    while (buffer.size() < offset + payloadLen)
    {
        char temp[4096];
        ssize_t bytesRead = read(socket, temp, sizeof(temp));
        if (bytesRead <= 0)
        {
            if (bytesRead == 0 && buffer.size() < offset + payloadLen)
                continue;
            perror("read failed");
            return {OPCODE_CLOSE_CONNECTION, ""};
        }
        buffer.append(temp, bytesRead);
    }

    std::string payload = buffer.substr(offset, payloadLen);
    buffer.erase(0, offset + payloadLen);

    return {opcode, payload};
}

void sendFrameUDP(SOCKET sockfd, uint8_t opcode, const std::string &payload, const struct sockaddr_in &addr,
                  socklen_t addrlen)
{
    std::vector<uint8_t> frame;
    frame.push_back(opcode);                                   // 1 octet : opcode
    frame.insert(frame.end(), payload.begin(), payload.end()); // payload brut

    sendto(sockfd, (const char *)frame.data(), frame.size(), 0, (const struct sockaddr *)&addr, addrlen);
}

std::tuple<uint8_t, std::string> receiveFrameUDP(SOCKET sockfd, struct sockaddr_in &addr, socklen_t &addrlen)
{
    char buffer[32000];
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addrlen);

    if (n <= 0)
        return {OPCODE_CLOSE_CONNECTION, ""};

    uint8_t opcode = buffer[0];
    std::string payload(buffer + 1, n - 1);
    return {opcode, payload};
}

bool tryCompressZlib(const std::string &in, std::string &out, int minThreshold, int margin)
{
#ifdef COMPRESSION_ENABLED
    out.clear();

    const int srcSize = static_cast<int>(in.size());
    if (srcSize < minThreshold)
        return false;

    const int headerSize = sizeof(uint32_t);
    out.resize(headerSize + compressBound(srcSize));

    unsigned long destLen = static_cast<unsigned long>(out.size() - headerSize);

    int res = compress2(reinterpret_cast<unsigned char *>(out.data() + headerSize), &destLen,
                        reinterpret_cast<const unsigned char *>(in.data()), srcSize, Z_BEST_SPEED);

    if (res != Z_OK)
    {
        out.clear();
        return false;
    }

    if (destLen + headerSize >= static_cast<unsigned long>(srcSize - margin))
    {
        out.clear();
        return false;
    }

    uint32_t origSize = static_cast<uint32_t>(srcSize);
    std::memcpy(out.data(), &origSize, headerSize);

    out.resize(headerSize + destLen);
    return true;
#else
    out.clear();
    return false;
#endif
}

bool ZlibDecompressPayload(const std::string &in, std::string &out)
{
#ifdef COMPRESSION_ENABLED
    out.clear();
    if (in.size() < sizeof(uint32_t))
        return false;

    unsigned long origSize = 0;
    std::memcpy(&origSize, in.data(), sizeof(uint32_t));
    if (origSize == 0)
        return false;

    const unsigned char *src = (const unsigned char *)(in.data() + sizeof(uint32_t));
    unsigned long srcSize = in.size() - sizeof(uint32_t);

    out.resize(origSize);
    int res = uncompress((unsigned char *)&out[0], &origSize, src, srcSize);
    if (res != Z_OK)
    {
        out.clear();
        return false;
    }
    return true;
#else
    std::cerr << "[transferData] Received compressed data but compression is disabled!" << std::endl;
    out.clear();
    return false;
#endif
}
