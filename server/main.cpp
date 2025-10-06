#include "Game/GameLoop.hpp"
#include <exception>



int main(int argc, char const *argv[])
{
    Orchestrator::GameLoop gameLoop;

    gameLoop.loop();
    return 0;
}
