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
#include <zlib.h>

/*
bool tryCompressLZ4(const std::string &in, std::string &out,
    int minThreshold, int margin)
{
    out.clear();

    const int srcSize = (int)in.size();
    if (srcSize < minThreshold) return false;

    const int maxDst = LZ4_compressBound(srcSize);
    std::vector<char> dst(maxDst);
    const int compressedSize =
        LZ4_compress_default(in.data(), dst.data(), srcSize, maxDst);
    if (compressedSize <= 0) return false;

    const int headerSize = sizeof(uint32_t);
    if (compressedSize + headerSize >= srcSize - margin) return false;

    out.resize(headerSize + compressedSize);
    uint32_t orig = (uint32_t)srcSize;
    std::memcpy(out.data(), &orig, sizeof(uint32_t));
    std::memcpy(out.data() + headerSize, dst.data(), compressedSize);
    return true;
}*/

bool tryCompressZlib(const std::string &in, std::string &out, int minThreshold, int margin)
{
    out.clear();

    const int srcSize = static_cast<int>(in.size());
    if (srcSize < minThreshold)
        return false;

    // Resize output to hold header + worst-case compressed size
    const int headerSize = sizeof(uint32_t);
    out.resize(headerSize + compressBound(srcSize));

    // Reserve space for header
    unsigned long destLen = static_cast<unsigned long>(out.size() - headerSize);

    int res = compress2(reinterpret_cast<unsigned char *>(out.data() + headerSize), // compressed data
                        &destLen,
                        reinterpret_cast<const unsigned char *>(in.data()), // input data
                        srcSize, Z_BEST_SPEED);

    if (res != Z_OK)
    {
        out.clear();
        return false;
    }

    // Check if compression is worthwhile
    if (destLen + headerSize >= static_cast<unsigned long>(srcSize - margin))
    {
        out.clear();
        return false;
    }

    // Store original size in first 4 bytes
    uint32_t origSize = static_cast<uint32_t>(srcSize);
    std::memcpy(out.data(), &origSize, headerSize);

    // Resize out to actual frame size
    out.resize(headerSize + destLen);
    return true;
}

/*
bool LZ4DecompressPayload(const std::string &in, std::string &out)
{
    out.clear();
    if (in.size() < sizeof(uint32_t)) return false;

    uint32_t origSize = 0;
    std::memcpy(&origSize, in.data(), sizeof(uint32_t));
    if (origSize == 0) return false;

    const char *src = in.data() + sizeof(uint32_t);
    const int srcSize = (int)(in.size() - sizeof(uint32_t));

    out.resize(origSize);
    const int decompressed = LZ4_decompress_safe(src, out.data(), srcSize, (int)origSize);
    if (decompressed != (int)origSize) {
        out.clear();
        return false;
    }
    return true;
}*/

bool ZlibDecompressPayload(const std::string &in, std::string &out)
{
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
}

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
// ...existing code...

std::tuple<uint8_t, std::string> receiveFrameTCP(SOCKET socket, std::string &buffer)
{
    char temp[4096];

    std::cout << "[DEBUG] Buffer size before read: " << buffer.size() << std::endl;

    ssize_t bytesRead = read(socket, temp, sizeof(temp));

    if (bytesRead < 0)
    {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK)
        {
            // No data available right now on non-blocking socket - not an error
            std::cout << "[DEBUG] No data available (WOULDBLOCK)" << std::endl;
        }
        else
        {
            std::cerr << "[DEBUG] Read error: " << error << std::endl;
            return {OPCODE_CLOSE_CONNECTION, ""};
        }
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No data available right now on non-blocking socket - not an error
            std::cout << "[DEBUG] No data available (WOULDBLOCK)" << std::endl;
        }
        else
        {
            perror("[DEBUG] Read error");
            return {OPCODE_CLOSE_CONNECTION, ""};
        }
#endif
    }
    else if (bytesRead == 0)
    {
        // Connection closed by peer ONLY if buffer is empty
        if (buffer.empty())
        {
            std::cout << "[DEBUG] Connection closed by peer" << std::endl;
            return {OPCODE_CLOSE_CONNECTION, ""};
        }
        else
        {
            // If buffer has data, this is just a read with no new data
            std::cout << "[DEBUG] No new data (0 bytes read), but buffer has " << buffer.size() << " bytes"
                      << std::endl;
        }
    }
    else
    {
        std::cout << "[DEBUG] Buffer size after read: " << buffer.size() + bytesRead << " (read " << bytesRead
                  << " bytes)" << std::endl;
    }

    // Ajouter les nouvelles données au buffer
    if (bytesRead > 0)
    {
        buffer.append(temp, bytesRead);
    }

    // Vérifier qu'on a au moins 2 octets pour le header
    if (buffer.size() < 2)
    {
        std::cout << "[DEBUG] Incomplete header (need 2 bytes, have " << buffer.size() << ")" << std::endl;
        return {OPCODE_INCOMPLETE_DATA, ""};
    }

    uint8_t opcode = buffer[0];
    uint8_t payloadLenByte = buffer[1];
    size_t offset = 2;
    uint64_t payloadLen = payloadLenByte;

    std::cout << "[DEBUG] Opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
    std::cout << "[DEBUG] Payload length byte: " << (int)payloadLenByte << std::endl;

    if (payloadLenByte == 254)
    {
        if (buffer.size() < 4)
        {
            std::cout << "[DEBUG] Incomplete extended length (need 4 bytes, have " << buffer.size() << ")" << std::endl;
            return {OPCODE_INCOMPLETE_DATA, ""};
        }
        payloadLen = ((uint8_t)buffer[2] << 8) | (uint8_t)buffer[3];
        offset = 4;
    }
    else if (payloadLenByte == 255)
    {
        if (buffer.size() < 10)
        {
            std::cout << "[DEBUG] Incomplete extended length (need 10 bytes, have " << buffer.size() << ")"
                      << std::endl;
            return {OPCODE_INCOMPLETE_DATA, ""};
        }
        payloadLen = 0;
        for (int i = 0; i < 8; ++i)
            payloadLen = (payloadLen << 8) | (uint8_t)buffer[2 + i];
        offset = 10;
    }

    size_t totalFrameSize = offset + payloadLen;
    std::cout << "[DEBUG] Total expected frame size: " << totalFrameSize << " bytes" << std::endl;
    std::cout << "[DEBUG] Current buffer size: " << buffer.size() << " bytes" << std::endl;

    if (buffer.size() < totalFrameSize)
    {
        std::cout << "[DEBUG] Incomplete frame (need " << totalFrameSize << " bytes, have " << buffer.size() << ")"
                  << std::endl;
        return {OPCODE_INCOMPLETE_DATA, ""};
    }

    std::cout << "[DEBUG] Frame complete! Extracting payload..." << std::endl;
    std::string payload = buffer.substr(offset, payloadLen);
    buffer.erase(0, totalFrameSize);
    std::cout << "[DEBUG] Payload extracted (" << payloadLen << " bytes), buffer remaining: " << buffer.size()
              << " bytes" << std::endl;

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