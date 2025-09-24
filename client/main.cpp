#include "ClientGame.hpp"
#include "NetworkECSMediator.hpp"
#include "iostream"

int main()
{

    try
    {
        NetworkECSMediator med;
        ClientGame clientGame(med);

        clientGame.init("127.0.0.1", 8081);
        clientGame.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
