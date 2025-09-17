#include "Game/GameLoader.hpp"


int main(int argc, char const *argv[])
{
    Orchestrator::GameLoader gameLoader;

    gameLoader.initialize("");
    gameLoader.startServer();
    return 0;
}
