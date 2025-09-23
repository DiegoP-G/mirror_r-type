#include "ClientGame.hpp"
#include "iostream"

int main()
{
    
    try {
        ClientGame clientGame;

        clientGame.init("127.0.0.1", 8081);
        clientGame.start();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    
}
