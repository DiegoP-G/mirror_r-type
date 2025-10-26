#pragma once
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

constexpr uint8_t OPCODE_SHIP_INFO = 0x03; // Sent by server give ship info

class ship
{
  public:
    int id;
    int posX;
    int posY;

    ship(int _id = 0, int _posX = 0, int _posY = 0) : id(_id), posX(_posX), posY(_posY){};

    void serialize(std::ostream &os) const
    {
        os.write(reinterpret_cast<const char *>(&id), sizeof(id));
        os.write(reinterpret_cast<const char *>(&posX), sizeof(posX));
        os.write(reinterpret_cast<const char *>(&posY), sizeof(posY));
    }

    void deserialize(std::istream &is)
    {
        is.read(reinterpret_cast<char *>(&id), sizeof(id));
        is.read(reinterpret_cast<char *>(&posX), sizeof(posX));
        is.read(reinterpret_cast<char *>(&posY), sizeof(posY));
    }

    void print() const
    {
        std::cout << "SHIP(" << id << ")" << std::endl << "posX: " << posX << ", posY: " << posY << std::endl;
    }
};
