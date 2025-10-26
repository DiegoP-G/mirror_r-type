#include "Game/GameLoop.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#else
#include "../transferData/transferData.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <exception>
#include <iostream>

int main(int argc, char const *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "[Error] WSAStartup failed with error code: " << WSAGetLastError() << std::endl;
        return 1;
    }
    std::cout << "[Windows] Winsock initialized successfully" << std::endl;
#endif

    try
    {
        std::cout << "Starting server..." << std::endl;
        Orchestrator::GameLoop gameLoop;

        std::cout << "Server initialized, starting main loop..." << std::endl;
        gameLoop.loop();

        std::cout << "Server stopped gracefully." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Error] Exception caught: " << e.what() << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

#ifdef _WIN32
    WSACleanup();
    std::cout << "[Windows] Winsock cleaned up" << std::endl;
#endif
    return 0;
}