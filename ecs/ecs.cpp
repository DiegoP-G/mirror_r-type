#include "ecs.hpp"
#include "components/InputComponent.hpp"
#include <cstring>

const std::string &serializePlayerInput(const InputComponent &player, int playerId)
{
    static std::string serializedData;
    serializedData.clear();
    serializedData += "PlayerID:" + std::to_string(playerId) + ";";
    serializedData += "InputComponent:";
    serializedData += " up=" + std::to_string(player.up);
    serializedData += " down=" + std::to_string(player.down);
    serializedData += " left=" + std::to_string(player.left);
    serializedData += " right=" + std::to_string(player.right);
    serializedData += " fire=" + std::to_string(player.fire);

    return serializedData;
}

int deserializePlayerInput(const std::string &data, InputComponent &input)
{
    size_t playerIdPos = data.find("PlayerID:");
    size_t inputCompPos = data.find("InputComponent:");

    if (playerIdPos == std::string::npos || inputCompPos == std::string::npos)
        return false;

    size_t idEndPos = data.find(";", playerIdPos);
    if (idEndPos == std::string::npos)
        return false;

    std::string idStr = data.substr(playerIdPos + 9, idEndPos - (playerIdPos + 9));
    int playerId = std::stoi(idStr);

    std::string inputData = data.substr(inputCompPos + 15);
    int up, down, left, right, fire;

    sscanf(inputData.c_str(), " up=%d down=%d left=%d right=%d fire=%d", &up, &down, &left, &right, &fire);

    input.up = up;
    input.down = down;
    input.left = left;
    input.right = right;
    input.fire = fire;
    return playerId;
}