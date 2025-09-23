#include <arpa/inet.h>
#include <cstring> // for memcpy
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"

int main()
{
    const char *server_ip = "127.0.0.1";
    int tcp_port = 8081;
    int udp_port = 8081;

    // ---- TCP connection ----
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0)
    {
        perror("TCP socket");
        return 1;
    }

    sockaddr_in server_tcp{};
    server_tcp.sin_family = AF_INET;
    server_tcp.sin_port = htons(tcp_port);
    inet_pton(AF_INET, server_ip, &server_tcp.sin_addr);

    if (connect(tcp_sock, (sockaddr *)&server_tcp, sizeof(server_tcp)) < 0)
    {
        perror("TCP connect");
        close(tcp_sock);
        return 1;
    }

    std::cout << "[TCP] Connected to server." << std::endl;

    // ---- Receive CODE_UDP frame ----
    std::string tcpBuffer;
    auto [opcode, payload] = receiveFrameTCP(tcp_sock, tcpBuffer);

    if (opcode != OPCODE_CODE_UDP)
    {
        std::cerr << "[TCP] Expected OPCODE_CODE_UDP, got " << (int)opcode << std::endl;
        close(tcp_sock);
        return 1;
    }

    if (payload.size() < 4)
    {
        std::cerr << "[TCP] Invalid payload size for CODE_UDP." << std::endl;
        close(tcp_sock);
        return 1;
    }

    int code = deserializeInt(payload);
    std::cout << "[TCP] Received code_udp = " << std::to_string(code) << std::endl;

    // ---- UDP socket ----
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0)
    {
        perror("UDP socket");
        close(tcp_sock);
        return 1;
    }

    sockaddr_in server_udp{};
    server_udp.sin_family = AF_INET;
    server_udp.sin_port = htons(udp_port);
    inet_pton(AF_INET, server_ip, &server_udp.sin_addr);

    sendFrameUDP(udp_sock, OPCODE_UDP_AUTH, serializeInt(code), server_udp, sizeof(server_udp));
    std::cout << "[UDP] Sent OPCODE_UDP_AUTH with code_udp = " << std::to_string(code) << std::endl;

    sendFrameUDP(udp_sock, 10, "hello", server_udp, sizeof(server_udp));
    std::cout << "[UDP] Sent 10 with  = " << "hello" << std::endl;

    // ---- Optional: wait for server response ----
    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);
    auto [udpOpcode, udpResponse] = receiveFrameUDP(udp_sock, from_addr, from_len);

    if (udpOpcode != OPCODE_CLOSE_CONNECTION)
    {
        std::cout << "[UDP] Received response opcode " << (int)udpOpcode << " payload size=" << udpResponse.size()
                  << std::endl;
    }

    // ---- Cleanup ----
    close(tcp_sock);
    close(udp_sock);

    return 0;
}
