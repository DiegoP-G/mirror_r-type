#include "transferData.hpp"
#include "opcode.hpp"
#include <cstring>
#include <sys/socket.h>
#include <tuple>
#include <unistd.h>
#include <vector>

bool sendFrameTCP(int fd, uint8_t opcode, const std::string &payload)
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
        ssize_t n = write(fd, frame.data() + sent, total - sent);

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

std::tuple<uint8_t, std::string> receiveFrameTCP(int socket, std::string &buffer)
{
    char temp[4096];

    std::cout << "[DEBUG] Buffer size before read: " << buffer.size() << std::endl;

    ssize_t bytesRead = read(socket, temp, sizeof(temp));
    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Pas de données disponibles, mais ne pas fermer la connexion
            std::cout << "[DEBUG] No data available (EAGAIN/EWOULDBLOCK)" << std::endl;
            bytesRead = 0; // ✅ Correct
            
            // Si le buffer est vide, retourner INCOMPLETE_DATA
            if (buffer.empty())
            {
                return {OPCODE_INCOMPLETE_DATA, ""};
            }
            // Sinon, continuer à traiter le buffer existant
        }
        else
        {
            std::cout << "[DEBUG] Connection closed (read error: " << strerror(errno) << ")" << std::endl;
            return {OPCODE_CLOSE_CONNECTION, ""};
        }
    }
    else if (bytesRead == 0)
    {
        // ⚠️ ATTENTION : read() == 0 signifie que le peer a fermé la connexion
        std::cout << "[DEBUG] Connection closed by peer (read 0 bytes)" << std::endl;
        return {OPCODE_CLOSE_CONNECTION, ""};
    }

    // Ajouter les nouvelles données au buffer
    if (bytesRead > 0)
    {
        buffer.append(temp, bytesRead);
        std::cout << "[DEBUG] Buffer size after read: " << buffer.size() << " (read " << bytesRead << " bytes)" << std::endl;
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
        if (buffer.size() < offset + 2)
        {
            std::cout << "[DEBUG] Incomplete extended length (254, need " << (offset + 2) << " bytes, have " << buffer.size() << ")" << std::endl;
            return {OPCODE_INCOMPLETE_DATA, ""};
        }
        payloadLen = ((uint8_t)buffer[offset] << 8) | (uint8_t)buffer[offset + 1];
        offset += 2;
        std::cout << "[DEBUG] Extended payload length (254): " << payloadLen << std::endl;
    }
    else if (payloadLenByte == 255)
    {
        if (buffer.size() < offset + 8)
        {
            std::cout << "[DEBUG] Incomplete extended length (255, need " << (offset + 8) << " bytes, have " << buffer.size() << ")" << std::endl;
            return {OPCODE_INCOMPLETE_DATA, ""};
        }
        payloadLen = 0;
        for (int i = 0; i < 8; ++i)
            payloadLen = (payloadLen << 8) | (uint8_t)buffer[offset + i];
        offset += 8;
        std::cout << "[DEBUG] Extended payload length (255): " << payloadLen << std::endl;
    }

    std::cout << "[DEBUG] Total expected frame size: " << (offset + payloadLen) << " bytes" << std::endl;
    std::cout << "[DEBUG] Current buffer size: " << buffer.size() << " bytes" << std::endl;

    // Vérifier si tout le payload est là
    if (buffer.size() < offset + payloadLen)
    {
        std::cout << "[DEBUG] Incomplete payload (need " << (offset + payloadLen) << " bytes, have " << buffer.size() << ")" << std::endl;
        return {OPCODE_INCOMPLETE_DATA, ""};
    }

    std::cout << "[DEBUG] Frame complete! Extracting payload..." << std::endl;

    // Extraire le payload complet
    std::string payload = buffer.substr(offset, payloadLen);
    buffer.erase(0, offset + payloadLen); // Consomme le frame utilisé

    std::cout << "[DEBUG] Payload extracted (" << payload.size() << " bytes), buffer remaining: " << buffer.size() << " bytes" << std::endl;

    return {opcode, payload};
}

void sendFrameUDP(int sockfd, uint8_t opcode, const std::string &payload, const struct sockaddr_in &addr,
                  socklen_t addrlen)
{
    std::vector<uint8_t> frame;
    frame.push_back(opcode);                                   // 1 octet : opcode
    frame.insert(frame.end(), payload.begin(), payload.end()); // payload brut

    sendto(sockfd, frame.data(), frame.size(), 0, (const struct sockaddr *)&addr, addrlen);
}

std::tuple<uint8_t, std::string> receiveFrameUDP(int sockfd, struct sockaddr_in &addr, socklen_t &addrlen)
{
    char buffer[32000];
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addrlen);

    if (n <= 0)
        return {OPCODE_CLOSE_CONNECTION, ""};

    uint8_t opcode = buffer[0];
    std::string payload(buffer + 1, n - 1);
    return {opcode, payload};
}
