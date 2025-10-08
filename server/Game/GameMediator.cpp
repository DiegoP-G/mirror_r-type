/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"
#include "../Network/NetworkManager.hpp"

GameMediator::GameMediator() : _networkManager(*new NetworkManager(*this)), _rTypeServer(*new RTypeServer(*this))
{
    _mediatorMap = {
        // Logique de jeu
        {GameMediatorEvent::TickLogic, [this](const std::string &data) -> void { _rTypeServer.run(std::stof(data)); }},

        // Input joueur
        {GameMediatorEvent::InitECS, [this](const std::string &data) -> void { _rTypeServer.init(); }},

        // Réseau
        {GameMediatorEvent::SetupNetwork, [this](const std::string &data) -> void {}},

        {GameMediatorEvent::TickNetwork, [this](const std::string &data) -> void { _networkManager.updateAllPoll(); }},

        // Joueurs
        {GameMediatorEvent::AddPlayer, [this](const std::string &data) -> void { _rTypeServer.createPlayer(data); }},

        // Création d'entité (TCP - fiable)
        {GameMediatorEvent::EntityCreated,
         [this](const std::string &data) -> void { _networkManager.sendDataAllClientTCP(data, OPCODE_ENTITY_CREATE); }},

        // Destruction d'entité (TCP - fiable)
        {GameMediatorEvent::EntityDestroyed,
         [this](const std::string &data) -> void {
             _networkManager.sendDataAllClientTCP(data, OPCODE_ENTITY_DESTROY);
         }},

        // Updates de mouvement (UDP - rapide)
        {GameMediatorEvent::MovementUpdate,
         [this](const std::string &data) -> void {
             _networkManager.sendDataAllClientUDP(data, OPCODE_MOVEMENT_UPDATE);
         }},

        {GameMediatorEvent::HealthUpdate,
         [this](const std::string &data) -> void { _networkManager.sendDataAllClientUDP(data, OPCODE_HEALTH_UPDATE); }},

        // Input joueur
        {GameMediatorEvent::PlayerInput,
         [this](const std::string &data) -> void { _rTypeServer.handlePlayerInput(data); }},

        {GameMediatorEvent::LobbyInfoUpdate, // contains playerReady / playerMax
         [this](const std::string &data) -> void {
             std::cout << "sending" << std::endl;
             _networkManager.sendDataAllClientTCP(data, OPCODE_LOBBY_INFO);
         }},
        // Wave update
        {GameMediatorEvent::UpdateWave,
         [this](const std::string &data) -> void { _networkManager.sendDataAllClientTCP(data, OPCODE_UPDATE_WAVE); }},
        
        // Score update
        {GameMediatorEvent::UpdateScore,
         [this](const std::string &data) -> void { _networkManager.sendDataAllClientTCP(data, OPCODE_UPDATE_SCORE); }},

 
        {GameMediatorEvent::GameOver,
         [this](const std::string &data) -> void { _networkManager.sendDataAllClientTCP(data, OPCODE_GAME_OVER); }},
        {GameMediatorEvent::PlayerDead,
         [this](const std::string &data) -> void { _networkManager.sendDataAllClientTCP(data, OPCODE_PLAYER_DEAD); }},
        {GameMediatorEvent::GameStateUpdate,
         [this](const std::string &data) -> void {
             _networkManager.sendDataAllClientTCP(data, OPCODE_GAME_STATE_UPDATE);
         }},
    };
}

void GameMediator::notify(const int &event, const std::string &data)
{
    const GameMediatorEvent &gameEvent = static_cast<GameMediatorEvent>(event);
    auto it = _mediatorMap.find(gameEvent);

    if (it != _mediatorMap.end())
    {
        it->second(data);
    }
    else
    {
        throw UnknownEventException(gameEvent);
    }
}

std::vector<std::string> GameMediator::getAllActiveEntities()
{
    return _rTypeServer.serializeAllActiveEntities();
}