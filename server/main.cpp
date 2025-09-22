#include "Game/GameLoader.hpp"
#include <exception>

int main(int argc, char const *argv[])
{
    Orchestrator::GameLoader gameLoader;

    try
    {
        gameLoader.initialize("");
    }
    catch (std::exception &e)
    {
        std::cerr << "initialize: Caught an error: " << std::endl;
        std::cerr << e.what() << std::endl;
    }

    try
    {
        gameLoader.startGameLoop();
    }
    catch (std::exception &e)
    {
        std::cerr << "startServer: Caught an error: " << std::endl;
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
