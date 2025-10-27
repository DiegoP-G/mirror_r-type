#include "ClientGame.hpp"
#include "iostream"
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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#endif
#include <regex>

void printUsage()
{
    std::cout << "Usage: ./GameServer -h <ip_address>" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h <ip_address>    Specify the IP address to bind the server" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./GameServer -h 0.0.0.0          # Listen on all interfaces" << std::endl;
    std::cout << "  ./GameServer -h 127.0.0.1        # Listen on localhost only" << std::endl;
    std::cout << "  ./GameServer -h 192.168.1.100    # Listen on specific IP" << std::endl;
}

bool getParams(int argc, char **argv)
{
    if (argc <= 2 || argc > 3)
    {
        printUsage();
        return false;
    }
    if (argc == 2 && std::string(argv[1]) == "-h")
    {
        printUsage();
        return false;
    }

    std::regex pattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$");
    if (!std::regex_match(argv[2], pattern))
    {
        printUsage();
        return false;
    }

    struct sockaddr_in sa;
    if (inet_pton(AF_INET, argv[2], &(sa.sin_addr)) == 1)
    {
        return true;
    }
    return false;
}

int main(int argc, char **argv)
{
    // if (!getParams(argc, argv))
    //     return 84;

    try
    {
        ClientGame clientGame;

        clientGame.init("0.0.0.0", 8081);
        clientGame.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
