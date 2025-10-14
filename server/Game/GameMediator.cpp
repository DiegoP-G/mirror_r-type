#include "GameMediator.hpp"
#include "../Network/NetworkManager.hpp"
#include <iostream>
#include <memory>

std::string generateLobbyUid()
{
    static int counter = 0;
    return "lobby_" + std::to_string(++counter);
}

std::pair<std::string, std::string> deserializeLobbyMessage(const std::string &msg)
{
    size_t sep = msg.find('|');
    if (sep == std::string::npos)
        return {"", msg};
    return {msg.substr(0, sep), msg.substr(sep + 1)};
}

GameMediator::GameMediator() : _networkManager(*new NetworkManager(*this)), _lobbyManager(*new LobbyManager(*this))
{

    _mediatorMap = {
        // Network setup & tick
        {GameMediatorEvent::SetupNetwork, [this](const std::string &) -> void {}},

        {GameMediatorEvent::TickNetwork, [this](const std::string &) -> void { _networkManager.updateAllPoll(); }},

        // Player added
        {GameMediatorEvent::AddPlayer,
         [this](const std::string &data) -> void {
             std::shared_ptr<Lobby> lobby = _lobbyManager.getLobby("lobby_1");
             if (!lobby)
             {
                 std::cout << "[AddPlayer] Lobby 'lobby_1' not found, creating it.\n";
                 lobby = _lobbyManager.createLobby("lobby_1");
             }

             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             int id = deserializeInt(data);

             rtype->createPlayer(data);
             lobby->addPlayer(deserializeInt(data));
         }},

        // Entity creation
        {GameMediatorEvent::EntityCreated,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_ENTITY_CREATE);
         }},

        // Entity destruction
        {GameMediatorEvent::EntityDestroyed,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_ENTITY_DESTROY);
         }},

        // Movement updates (UDP per lobby)
        {GameMediatorEvent::MovementUpdate,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (!lobby)
             {
                 std::cerr << "[MovementUpdate] Unknown lobby: " << lobbyUid << std::endl;
                 return;
             }
             _networkManager.sendDataToLobbyUDP(lobby, payload, OPCODE_MOVEMENT_UPDATE);
         }},

        // Health updates (UDP per lobby)
        {GameMediatorEvent::HealthUpdate,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (!lobby)
             {
                 std::cerr << "[HealthUpdate] Unknown lobby: " << lobbyUid << std::endl;
                 return;
             }
             _networkManager.sendDataToLobbyUDP(lobby, payload, OPCODE_HEALTH_UPDATE);
         }},

        // Player input handling
        {GameMediatorEvent::PlayerInput,
         [this](const std::string &data) -> void {
             InputComponent inputComp;
             int playerId = deserializePlayerInput(data, inputComp);
             std::shared_ptr<Lobby> lobby = _lobbyManager.getLobbyOfPlayer(playerId);
             if (!lobby)
             {
                 std::cerr << "[PlayerInput] Player not assigned to a lobby.\n";
                 return;
             }
             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             rtype->handlePlayerInput(data);
         }},

        // Lobby info updates (TCP)
        {GameMediatorEvent::LobbyInfoUpdate,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (!lobby)
             {
                 std::cerr << "[LobbyInfoUpdate] Could not find lobby " << lobbyUid << std::endl;
                 return;
             }
             _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_LOBBY_INFO);
         }},

        // Wave update (TCP per lobby)
        {GameMediatorEvent::UpdateWave,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_UPDATE_WAVE);
         }},

        // Score update (TCP per lobby)
        {GameMediatorEvent::UpdateScore,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_UPDATE_SCORE);
         }},

        // Game over (TCP per lobby)
        {GameMediatorEvent::GameOver,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_GAME_OVER);
         }},

        // Player dead (TCP per lobby)
        {GameMediatorEvent::PlayerDead,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_PLAYER_DEAD);
         }},

        // Game state update (TCP per lobby)
        {GameMediatorEvent::GameStateUpdate,
         [this](const std::string &data) -> void {
             auto [lobbyUid, payload] = deserializeLobbyMessage(data);
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, payload, OPCODE_GAME_STATE_UPDATE);
         }},

        // Create lobby
        {GameMediatorEvent::CreateLobby, [this](const std::string &) -> void { _lobbyManager.createLobby("lobby_1"); }},
    };
}

void GameMediator::notify(const int &event, const std::string &data, const std::string &lobbyUid)
{
    const GameMediatorEvent &gameEvent = static_cast<GameMediatorEvent>(event);
    auto it = _mediatorMap.find(gameEvent);

    if (it != _mediatorMap.end())
    {
        // Pack the lobby UID and data together (format: "lobbyUid|data")
        std::string combinedData = lobbyUid.size() > 1 ? lobbyUid + "|" + data : data;
        it->second(combinedData);
    }
    else
    {
        std::cerr << "[ERROR] Unknown event received by mediator: " << event << std::endl;
    }
}

std::vector<std::string> GameMediator::getAllActiveEntities()
{
    std::shared_ptr<Lobby> lobby = _lobbyManager.getLobby("lobby_1");
    if (!lobby)
        return {};
    std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
    return rtype->serializeAllActiveEntities();
}
