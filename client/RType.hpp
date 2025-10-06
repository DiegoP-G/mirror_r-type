#include "../ecs/IComponent.hpp"
#include "../ecs/systems.hpp"
#include "../ecs/textBox.hpp"
#include "NetworkECSMediator.hpp"
#include <mutex>

class RTypeGame
{
  private:
    int _playerId;
    std::mutex _mutex;
    EntityManager entityManager;

    // Systems
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    CollisionSystem collisionSystem;
    BoundarySystem boundarySystem;        // Generic boundary checking
    OffscreenCleanupSystem cleanupSystem; // Generic cleanup
    InputSystem inputSystem;              // Use existing generic input system
    PlayerSystem playerSystem;
    EnemySystem enemySystem;
    LaserWarningSystem laserWarningSystem;
    GameLogicSystem gameLogicSystem;
    BackgroundSystem backgroundSystem;

    NetworkECSMediator _med;

    Entity *player = nullptr;
    bool gameOver = false;

    bool running = false;

    int score = 0;

    std::function<void(const char *)> _networkCb;

    const float ENEMY_SPEED = -200.0f;

  public:
    RTypeGame() = default;

    std::mutex &getMutex()
    {
        return _mutex;
    }

    void findMyPlayer();

    EntityManager &getEntityManager()
    {
        return entityManager;
    }
    void setPlayerId(int id)
    {
        _playerId = id;
    };


    bool init(NetworkECSMediator med, std::function<void(const char *)> networkCb);

    void createTextures();

    void createBackground();

    void createPlayer();

    void handleEvents();
    // void handleEvents();

    void sendInputPlayer();

    void update(float deltaTime);

    void render();
    // void render();
    

    void restart();

    void run();
};