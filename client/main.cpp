#include "ClientGame.hpp"
#include "iostream"

int main()
{
    
    try {
        ClientGame clientGame;

        clientGame.init();
        clientGame.start();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    
}