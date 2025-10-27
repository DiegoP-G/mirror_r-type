#include "GameMediator.hpp"
#include "../Network/NetworkManager.hpp"
#include "transferData/hashUtils.hpp"
#include <iostream>
#include <memory>

std::string generateLobbyUid()
{
    static int counter = 0;
    return "lobby_" + std::to_string(++counter);
}

GameMediator::GameMediator() : _networkManager(*new NetworkManager(*this)), _lobbyManager(*new LobbyManager(*this))
{

    _mediatorMap = {
        {GameMediatorEvent::SetupNetwork, [this](const std::string &, const std::string &, int) -> void {}},

        {GameMediatorEvent::TickNetwork,
         [this](const std::string &, const std::string &, int) -> void { _networkManager.updateAllPoll(); }},

        {GameMediatorEvent::EntityCreated,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_ENTITY_CREATE);
         }},

        {GameMediatorEvent::EntityDestroyed,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_ENTITY_DESTROY);
         }},

        {GameMediatorEvent::MovementUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_MOVEMENT_UPDATE);
         }},

        {GameMediatorEvent::MovementUpdateZlib,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_MOVEMENT_UPDATE_ZLIB);
         }},

        {GameMediatorEvent::HealthUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_HEALTH_UPDATE);
         }},

        {GameMediatorEvent::HealthUpdateZlib,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_HEALTH_UPDATE_ZLIB);
         }},

        {GameMediatorEvent::ProjectilesUpdateZlib,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_PROJECTILES_UPDATE_ZLIB);
         }},

        // Input joueur
        {GameMediatorEvent::PlayerInput,
         [this](const std::string &data, const std::string &lobbyUid, int clientFd) -> void {
             InputComponent inputComp;
             int playerId = deserializePlayerInput(data, inputComp);
             std::shared_ptr<Lobby> lobby = _lobbyManager.getLobbyOfPlayer(playerId);
             if (!lobby)
             {
                 std::cerr << "[PlayerInput] Player not in a lobby.\n";
                 return;
             }
             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             rtype->handlePlayerInput(data);
         }},

        {GameMediatorEvent::LobbyInfoUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_LOBBY_INFO);
         }},

        {GameMediatorEvent::UpdateWave,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_UPDATE_WAVE);
         }},

        {GameMediatorEvent::UpdateScore,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_UPDATE_SCORE);
         }},

        {GameMediatorEvent::GameOver,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_GAME_OVER);
         }},

        {GameMediatorEvent::PlayerDead,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_PLAYER_DEAD);
         }},

        {GameMediatorEvent::GameStateUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_GAME_STATE_UPDATE);
             std::cout << "Finished updating states" << std::endl;
         }},

        {GameMediatorEvent::CreateLobby,
         [this](const std::string &data, const std::string &, int) -> void { _lobbyManager.createLobby(data); }},

        {GameMediatorEvent::JoinLobby,
         [this](const std::string &data, const std::string &, int clientFd) -> void {
             auto lobby = _lobbyManager.getLobby(data);

             std::cout << "[JoinLobby] Client " << clientFd << " joining " << data << std::endl;
             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             lobby->addPlayer(clientFd);
             rtype->createPlayer(clientFd);
             _networkManager.sendAllEntitiesToClient(clientFd);
         }},
        {GameMediatorEvent::PlayerDisconnected,
         [this](const std::string &, const std::string &, int clientFd) -> void {
             std::shared_ptr<Lobby> lobby = _lobbyManager.getLobbyOfPlayer(clientFd);
             if (!lobby)
             {
                 std::cerr << "[PlayerDisconnected] Player not in a lobby.\n";
                 return;
             }
             std::cout << "lobby player size before remove: " << lobby->getPlayers().size() << std::endl;
             lobby->removePlayer(clientFd);

             std::cout << "lobby player size after remove: " << lobby->getPlayers().size() << std::endl;
             if (lobby->getPlayers().size() == 0)
             {
                 std::cout << "[PlayerDisconnected] Lobby " << lobby->getUid() << " is empty. Removing it."
                           << std::endl;
                 _lobbyManager.removeLobby(lobby->getUid());
             }
         }},
        {GameMediatorEvent::LoginReqest,
         [this](const std::string &data, const std::string &, int clientFd) -> void {
             std::vector<uint8_t> encryptedData(data.begin(), data.end());
             std::optional<std::string> decryptedData =
                 decryptAESAppendedTag(_networkManager.getAesKey(), _networkManager.getAesIV(), encryptedData);

             if (!decryptedData.has_value())
             {
                 std::cerr << "Server failed to decrypt client data\n";
                 return;
             }

             size_t offset = 0;
             std::string username = deserializeString((*decryptedData).substr(offset));
             offset += sizeof(int) + username.size();
             std::string password = deserializeString((*decryptedData).substr(offset));

             bool success = _networkManager.getClientManager().checkLoginCreds(username, password);
             std::string message;

             if (success)
             {
                 message = "Welcome, " + username + "!";
                 std::cout << "[Server] Login successful for " << username << std::endl;
             }
             else
             {
                 message = "Invalid username or password";
                 std::cout << "[Server] Login failed for " << username << std::endl;
             }

             // Send response : first byte: success flag, rest: message
             std::string response;
             response.push_back(success ? 1 : 0);
             response.append(message);
             std::string serializedResponse = serializeString(response);

             std::cout << "Sending login response to : " << clientFd << "\n";
             _networkManager.getTCPManager().sendMessage(clientFd, OPCODE_LOGIN_RESPONSE, serializedResponse);
         }},
        {GameMediatorEvent::SigninRequest,
         [this](const std::string &data, const std::string &, int clientFd) -> void {
             std::cout << "[GameMediator] SigninRequest event triggered for clientFd: " << clientFd << std::endl;

             std::vector<uint8_t> encryptedData(data.begin(), data.end());
             std::optional<std::string> decryptedData =
                 decryptAESAppendedTag(_networkManager.getAesKey(), _networkManager.getAesIV(), encryptedData);

             if (!decryptedData.has_value())
             {
                 std::cerr << "Server failed to decrypt client data\n";
                 return;
             }

             size_t offset = 0;
             std::string username = deserializeString((*decryptedData).substr(offset));
             offset += sizeof(int) + username.size();
             std::string password = deserializeString((*decryptedData).substr(offset));
             std::string message;
             bool success = false;
             try
             {
                 _networkManager.getClientManager().addNewPlayerEntry(username, password);
                 message = "Welcome, " + username + "!";
                 success = true;
             }
             catch (std::exception &e)
             {
                 message = e.what();
                 success = false;
             }

             // Send response : first byte: success flag, rest: message
             std::string response;
             response.push_back(success ? 1 : 0);
             response.append(message);
             std::string serializedResponse = serializeString(response);

             std::cout << "Sending signin response to : " << clientFd << "\n";
             _networkManager.getTCPManager().sendMessage(clientFd, OPCODE_SIGNIN_RESPONSE, serializedResponse);
         }},
        {GameMediatorEvent::LoginResponse,
         [this](const std::string &data, const std::string &, int clientFd) -> void {}},
        {GameMediatorEvent::SigninResponse,
         [this](const std::string &data, const std::string &, int clientFd) -> void {}},
        {GameMediatorEvent::ServerPubKey,
         [this](const std::string &data, const std::string &, int clientFd) -> void {}},
        {GameMediatorEvent::ClientIVKey,
         [this](const std::string &data, const std::string &, int clientFd) -> void {
             const char *raw_data = data.data();
             const unsigned char *unsigned_char_ptr = reinterpret_cast<const unsigned char *>(raw_data);
             auto optDecryptedAes =
                 decryptClientAesWithPublicKey(_networkManager.getServerPubKey(), unsigned_char_ptr, data.size());

             if (!optDecryptedAes.has_value())
             {
                 return;
             }
             auto decryptedAes = *optDecryptedAes;

             std::vector<uint8_t> client_aes_key(decryptedAes.begin(), decryptedAes.begin() + AES_KEY_BYTES);
             std::vector<uint8_t> client_aes_iv(decryptedAes.begin() + AES_KEY_BYTES, decryptedAes.end());

             _networkManager.setAesIV(client_aes_iv);
             _networkManager.setAesKey(client_aes_key);
         }},
    };
}

void GameMediator::notify(const int &event, const std::string &data, const std::string &lobbyUid, int clientFd)
{
    const GameMediatorEvent &gameEvent = static_cast<GameMediatorEvent>(event);
    auto it = _mediatorMap.find(gameEvent);

    if (it != _mediatorMap.end())
    {
        it->second(data, lobbyUid, clientFd);
    }
    else
    {
        std::cerr << "[ERROR] Unknown event received by mediator: " << event << std::endl;
    }
}

std::vector<std::string> GameMediator::getAllActiveEntitiesFromLobby(int clientFd)
{
    std::shared_ptr<Lobby> lobby = _lobbyManager.getLobbyOfPlayer(clientFd);
    if (!lobby)
        return {};
    std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
    return rtype->serializeAllActiveEntities();
}
