#include "ClientGame.hpp"
#include "iostream"
#include <arpa/inet.h>

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

bool getParams(char **argv)
{
    if (std::string(argv[1]) == "-h") {
        struct sockaddr_in sa;
        if (inet_pton(AF_INET, argv[2], &(sa.sin_addr)) == 1) {
            return true;
        }
    }
    printUsage();
    return false;
}

int main(int argc, char **argv)
{
    if (!getParams(argv))
        return 84;

    try
    {
        ClientGame clientGame;

        clientGame.init(argv[2], 8081);
        clientGame.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
