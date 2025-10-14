#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

class RTypeServer;
class GameMediator;

class Lobby
{
  public:
    Lobby(const std::string &uid, GameMediator &gameMediator);
    ~Lobby();

    Lobby(const Lobby &) = delete;
    Lobby &operator=(const Lobby &) = delete;

    void run();  // Starts and runs the main game loop
    void stop(); // Stops the loop
    void addPlayer(int fd);
    void removePlayer(int fd);

    std::string getUid() const;
    std::vector<int> getPlayers();
    bool isRunning() const;

    float getDeltaTime();

  private:
    void update(); // Now uses stored _deltaTime

    std::string _uid;
    std::unordered_set<int> _connectedClients;
    std::unique_ptr<RTypeServer> _rtypeGame;
    std::atomic<bool> _running;
    std::mutex _mutex;

    float _deltaTime; // Stores time between frames
};
