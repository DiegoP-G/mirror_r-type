#pragma once
#include "GraphicsManager.hpp"
#include "allComponentsInclude.hpp"
#include "components.hpp"
#include "ecs.hpp"
#include "entityManager.hpp"
#include <cmath>
#include <random>

// System for movement
class MovementSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, VelocityComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();
            auto &velocity = entity->getComponent<VelocityComponent>();
            transform.position += velocity.velocity * deltaTime;
        }
    }
};

inline Vector2D getActualPosition(Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    Vector2D position = transform.position;

    // Utiliser hasComponent au lieu d'un try-catch
    if (entity->hasComponent<CenteredComponent>())
    {
        auto &centered = entity->getComponent<CenteredComponent>();
        position.x -= centered.offsetX;
        position.y -= centered.offsetY;
    }
    return position;
}

// System for rendering
class RenderSystem
{
  public:
    void update(EntityManager &entityManager)
    {
        if (!g_graphics)
            return;
        if (!g_graphics)
            return;

        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();
            Vector2D position = getActualPosition(entity);

            if (entity->hasComponent<AnimatedSpriteComponent>())
            {
                auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();
                g_graphics->drawAnimatedSprite(animatedSprite, position.x, position.y);
                continue;
            }

            auto &sprite = entity->getComponent<SpriteComponent>();

            if (!sprite.isVisible)
                continue;

            // If we have a texture, use it; otherwise draw a colored rectangle
            if (sprite.texture)
            {
                g_graphics->drawTexture(*sprite.texture, position.x, position.y, sprite.width, sprite.height);
            }
            else
            {
                // Draw colored rectangle
                g_graphics->drawRect(position.x, position.y, sprite.width, sprite.height, sprite.r, sprite.g, sprite.b,
                                     sprite.a);
            }
        }
    }
};

// System for game rules and logic
class GameLogicSystem
{
  private:
    float enemySpawnTimer = 0.0f;
    const float ENEMY_SPAWN_INTERVAL = 2.0f;
    const float ENEMY_SPEED = -200.0f;
    std::mt19937 rng;
    int score = 0;
    int stageCount = 0;  // 0 = normal, 1 = stage 1, 2 = stage 2, 3 = 1vs 1;
    int stageStatus = 0; // 0 = not started, 1 = in progress, 2 = ended;
  public:
    GameLogicSystem() : rng(std::random_device{}())
    {
    }

    void update(EntityManager &entityManager, float deltaTime)
    {
        // Spawn enemies
        enemySpawnTimer += deltaTime;
        // if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
        //     spawnEnemy(entityManager);
        //     enemySpawnTimer = 0.0f;
        // }
        if (stageStatus == 0)
        {
            stageStatus = 1;
            stageCount = 1;
            // spawnLaser1(entityManager);
            // spawnEnemies(entityManager);
        }
        else if (stageStatus == 1 && stageCount == 1)
        {
            if (entityManager.getEntitiesWithComponents<LaserWarningComponent>().empty())
            {
                stageStatus = 1;
                stageCount = 2;
            }
        }

        updateScore(entityManager);

        checkGameOverConditions(entityManager);
    }

    int getScore() const
    {
        return score;
    }

  private:
    void spawnLaser1(EntityManager &entityManager)
    {
        for (size_t i = 0; i < 8; i++)
        {
            float width = 900.0f;
            float height = 50.0f;
            float x = 450.0f;
            float y = 0.0f;
            float waitingTime = 0.0f;
            if (i < 3)
            {
                y = 100.0f + i % 3 * 200;
                waitingTime = 3.0f;
            }
            else if (i < 6)
            {
                y = 100.0f + i % 3 * 200;
                waitingTime = 14.0f;
            }
            else
            {
                y = 200.0f + i % 3 * 200;
                waitingTime = 8.5f;
            }

            auto &laser = entityManager.createEntity();
            laser.addComponent<TransformComponent>(width / 2,
                                                   y); // Centered horizontally
            laser.addComponent<CenteredComponent>(width / 2,
                                                  height / 2); // Centered
            laser.addComponent<SpriteComponent>(width, height, 255, 0,
                                                0); // Full height
            laser.addComponent<ColliderComponent>(width, height, false);
            laser.addComponent<LaserWarningComponent>(width, height, waitingTime, 1.5f,
                                                      3.0f); // Warning for 2s, active for 3s
        }
    }

    void spawnEnemies(EntityManager &entityManager)
    {
        // float enemyHeight = 20.0f;
        float y;
        for (size_t i = 0; i < 10; i++)
        {
            y = 100 + i * 50;

            auto &enemy = entityManager.createEntity();
            enemy.addComponent<TransformComponent>(700, y);
            enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
            enemy.addComponent<SpriteComponent>(20.0f, 20.0f, 0, 255, 0);
            enemy.addComponent<ColliderComponent>(20.0f, 20.0f, true);
            enemy.addComponent<EnemyComponent>(1, 0.2f, 2); // Type 2 = Sine wave movement and 3 bullets
        }
    }

    void updateScore(EntityManager &entityManager)
    {
        // Score logic here - e.g., increment score when enemy passes player
        auto enemies = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();
        for (auto &enemy : enemies)
        {
            auto &transform = enemy->getComponent<TransformComponent>();
            auto &enemyComp = enemy->getComponent<EnemyComponent>();

            // If enemy passed player position and wasn't counted yet
            // if (transform.position.x < 80.0f && !enemyComp.counted) {
            //     enemyComp.counted = true;
            //     score++;
            // }
        }
    }

    void checkGameOverConditions(EntityManager &entityManager)
    {
        // Additional game over logic here
    }
};

class LaserWarningSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<LaserWarningComponent>();
        for (auto &entity : entities)
        {
            auto &laser = entity->getComponent<LaserWarningComponent>();
            auto &sprite = entity->getComponent<SpriteComponent>();
            if (!entity->hasComponent<TransformComponent>())
            {
                std::cerr << "Entity with LaserWarningComponent missing TransformComponent!" << std::endl;
                continue; // Skip this entity if it doesn't have a TransformComponent
            }
            auto &transform = entity->getComponent<TransformComponent>();
            if (laser.appearanceTime > 0.0f)
            {
                laser.appearanceTime -= deltaTime;
                sprite.isVisible = false;
                continue; // Wait until appearance time is over
            }
            if (!laser.isActive)
            {
                if (!laser.warningShown)
                {
                    laser.warningShown = true;
                    sprite.isVisible = true;
                    sprite.r = 200; // Red color for warning
                    sprite.g = 0;
                    sprite.b = 0;
                    sprite.height = laser.height / 4;
                    transform.position.y += (laser.height - sprite.height) / 2;
                    sprite.width = laser.width;
                }
                laser.warningTime -= deltaTime;
                if (laser.warningTime <= 0.0f)
                {
                    laser.isActive = true;
                    if (entity->hasComponent<ColliderComponent>())
                    {
                        auto &collider = entity->getComponent<ColliderComponent>();
                        collider.isActive = true; // Enable collision when laser is active
                    }
                    sprite.r = 255;
                    transform.position.y -= (laser.height - sprite.height) / 2;
                    sprite.height = laser.height;

                    // Activer le laser (collision, dégâts, sprite différent, etc.)
                }
            }
            else
            {
                // Phase active
                laser.activeTime -= deltaTime;
                if (laser.activeTime <= 0.0f)
                {
                    // Désactiver ou détruire le laser
                    entity->destroy();
                }
            }
        }
    }
};

// System for collision detection
class CollisionSystem
{
  public:
    void update(EntityManager &entityManager)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, ColliderComponent>();
        for (size_t i = 0; i < entities.size(); i++)
        {
            if (!entities[i]->isActive())
                continue;

            auto &collider1 = entities[i]->getComponent<ColliderComponent>();
            if (!collider1.isActive)
                continue;

            // Obtenir la position réelle avec le helper
            Vector2D position1 = getActualPosition(entities[i]);

            // Mettre à jour la hitbox avec la position calculée
            collider1.hitbox.x = position1.x;
            collider1.hitbox.y = position1.y;

            for (size_t j = i + 1; j < entities.size(); j++)
            {
                if (!entities[j]->isActive())
                    continue;
                auto &collider2 = entities[j]->getComponent<ColliderComponent>();
                if (!collider2.isActive)
                    continue;
                // De même pour la deuxième entité
                Vector2D position2 = getActualPosition(entities[j]);

                collider2.hitbox.x = position2.x;
                collider2.hitbox.y = position2.y;

                // Vérification de collision
                if (collider1.hitbox.intersects(collider2.hitbox))
                {
                    handleCollision(entities[i], entities[j]);
                }
            }
        }
    }

  private:
    void handleCollision(Entity *a, Entity *b)
    {
        // std::cout << "Collision detected between Entity " << a->getID()
        //   << " and Entity " << b->getID() << std::endl;
        // Check for laser collisions first
        bool aIsLaser = a->hasComponent<LaserWarningComponent>();
        bool bIsLaser = b->hasComponent<LaserWarningComponent>();
        bool aIsPlayer = a->hasComponent<PlayerComponent>();
        bool bIsPlayer = b->hasComponent<PlayerComponent>();

        // Player hit by active laser
        if ((aIsLaser && bIsPlayer) || (bIsLaser && aIsPlayer))
        {
            Entity *laser = aIsLaser ? a : b;
            Entity *player = aIsPlayer ? a : b;

            auto &laserComp = laser->getComponent<LaserWarningComponent>();

            // Only damage if laser is active (not in warning phase)
            if (laserComp.isActive && laserComp.appearanceTime <= 0.0f)
            {
                std::cout << "Player hit by active laser!" << std::endl;

                // Damage or destroy player
                if (player->hasComponent<HealthComponent>())
                {
                    auto &health = player->getComponent<HealthComponent>();
                    health.health -= 50.0f; // Heavy laser damage
                    if (health.health <= 0)
                    {
                        player->destroy();
                        std::cout << "Player destroyed by laser!" << std::endl;
                    }
                }
                else
                {
                    // If no health component, destroy immediately
                    player->destroy();
                    std::cout << "Player destroyed by laser (no health)!" << std::endl;
                }
            }
            return; // Exit early to avoid other collision checks
        }

        // Projectile hit laser (bullets can destroy warning lasers)
        bool aIsProjectile = a->hasComponent<ProjectileComponent>();
        bool bIsProjectile = b->hasComponent<ProjectileComponent>();

        if ((aIsProjectile && bIsLaser) || (bIsProjectile && aIsLaser))
        {
            Entity *projectile = aIsProjectile ? a : b;
            Entity *laser = aIsLaser ? a : b;

            auto &laserComp = laser->getComponent<LaserWarningComponent>();

            // Can only destroy laser if it's still in warning phase
            if (!laserComp.isActive && laserComp.warningShown)
            {
                std::cout << "Laser warning destroyed by projectile!" << std::endl;
                laser->destroy();
                projectile->destroy();
            }
            return;
        }

        // For Flappy Bird: Check if bird hits pipe
        bool aIsJumper = a->hasComponent<JumpComponent>();
        bool bIsJumper = b->hasComponent<JumpComponent>();

        // Bird (has JumpComponent) hits anything else = game over
        if (aIsJumper || bIsJumper)
        {
            Entity *jumper = aIsJumper ? a : b;
            jumper->destroy(); // Destroy the jumping entity (bird)
            return;
        }

        // Original R-Type collision logic for backwards compatibility
        bool aIsEnemy = a->hasComponent<EnemyComponent>();
        bool bIsEnemy = b->hasComponent<EnemyComponent>();

        // Player hit by enemy
        if ((aIsPlayer && bIsEnemy) || (bIsPlayer && aIsEnemy))
        {
            Entity *player = aIsPlayer ? a : b;
            if (player->hasComponent<HealthComponent>())
            {
                auto &health = player->getComponent<HealthComponent>();
                health.health -= 10.0f; // Example damage
                if (health.health <= 0)
                {
                    player->destroy();
                }
            }
            else
            {
                player->destroy();
            }
        }

        // Projectile hit something (but not laser, already handled above)
        if (aIsProjectile || bIsProjectile)
        {
            Entity *projectile = aIsProjectile ? a : b;
            Entity *target = aIsProjectile ? b : a;

            auto &projComp = projectile->getComponent<ProjectileComponent>();

            // Don't hit your owner
            if (target->getID() == projComp.owner)
            {
                return;
            }

            // Damage target if it has health
            if (target->hasComponent<HealthComponent>())
            {
                auto &health = target->getComponent<HealthComponent>();
                health.health -= projComp.damage;
                if (health.health <= 0)
                {
                    target->destroy();
                }
            }

            // Destroy projectile on hit
            projectile->destroy();
        }
    }
};

// InputSystem - se contente de traduire les entrées en modifications de
// composants
class InputSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        // First, handle SFML events and update InputComponents
        handleEvents(entityManager);

        // Then update velocities based on input
        auto entities = entityManager.getEntitiesWithComponents<InputComponent, VelocityComponent>();

        for (auto &entity : entities)
        {
            auto &input = entity->getComponent<InputComponent>();
            auto &velocity = entity->getComponent<VelocityComponent>();

            // Reset velocity
            velocity.velocity.x = 0.0f;
            velocity.velocity.y = 0.0f;

            // Update velocity based on input
            const float PLAYER_SPEED = 200.0f;
            if (input.up)
                velocity.velocity.y = -PLAYER_SPEED;
            if (input.down)
                velocity.velocity.y = PLAYER_SPEED;
            if (input.left)
                velocity.velocity.x = -PLAYER_SPEED;
            if (input.right)
                velocity.velocity.x = PLAYER_SPEED;
        }
    }

  private:
    void handleEvents(EntityManager &entityManager)
    {
        if (!g_graphics)
            return;

        sf::Event event;
        while (g_graphics->getWindow().pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                // You might want to add a way to signal game exit
                // For now, we'll just handle input events
            }

            if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
            {
                bool isPressed = (event.type == sf::Event::KeyPressed);

                // Update all entities with InputComponent
                auto inputEntities = entityManager.getEntitiesWithComponents<InputComponent>();

                for (auto &entity : inputEntities)
                {
                    auto &input = entity->getComponent<InputComponent>();

                    switch (event.key.code)
                    {
                    case sf::Keyboard::Up:
                        input.up = isPressed;
                        break;
                    case sf::Keyboard::Down:
                        input.down = isPressed;
                        break;
                    case sf::Keyboard::Left:
                        input.left = isPressed;
                        break;
                    case sf::Keyboard::Right:
                        input.right = isPressed;
                        break;
                    case sf::Keyboard::Space:
                        input.fire = isPressed;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
};

// PlayerSystem - gère les comportements spécifiques au joueur
class PlayerSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

        for (auto &entity : entities)
        {
            auto &input = entity->getComponent<InputComponent>();

            // Fire a bullet when space is pressed
            if (input.fire)
            {
                fire(entityManager, entity);
                input.fire = false; // Reset fire input
            }

            if (!entity->hasComponent<AnimatedSpriteComponent>())
                continue;
            auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();
            // Determine direction based on input
            AnimatedSpriteComponent::Direction direction = AnimatedSpriteComponent::Default;
            if (input.up)
            {
                direction = AnimatedSpriteComponent::Up;
            }
            else if (input.down)
            {
                direction = AnimatedSpriteComponent::Down;
            }

            // Update animation based on direction
            animatedSprite.updateAnimation(direction);
        }
    }

  private:
    void fire(EntityManager &entityManager, Entity *player)
    {
        auto &transform = player->getComponent<TransformComponent>();

        // Create bullet entity
        auto &bullet = entityManager.createEntity();

        // Position the bullet at the player's position
        bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, // Offset to fire from the front
                                                transform.position.y + 16.0f  // Center height
        );

        // Add bullet components
        bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
        bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);                  // Red bullet
        bullet.addComponent<ColliderComponent>(8.0f, 8.0f);                     // Small hitbox
        bullet.addComponent<ProjectileComponent>(10.0f, 2.0f, player->getID()); // Damage, lifetime, owner
    }
};

// System for projectile lifetime management
class ProjectileSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<ProjectileComponent>();

        for (auto &entity : entities)
        {
            auto &projectile = entity->getComponent<ProjectileComponent>();

            // Update lifetime
            projectile.remainingLife -= deltaTime;
            if (projectile.remainingLife <= 0)
            {
                entity->destroy();
            }
        }
    }
};

// System for enemy behavior
class EnemySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();

        for (auto &entity : entities)
        {
            auto &enemy = entity->getComponent<EnemyComponent>();
            auto &transform = entity->getComponent<TransformComponent>();

            // Update enemy behavior based on type
            switch (enemy.type)
            {
            case 0: // Basic enemy - moves left
                if (entity->hasComponent<VelocityComponent>())
                {
                    entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, 0);
                }
                break;

            case 1: // Sine wave movement
                if (entity->hasComponent<VelocityComponent>())
                {
                    static float time = 0;
                    time += deltaTime;
                    entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, sinf(time * 1.0f) * 500.0f);
                }
                break;
            }
            if (enemy.currentCooldown > 0)
            {
                enemy.currentCooldown -= deltaTime;
            }
            else
            {
                enemy.currentCooldown = enemy.attackCooldown;
                enemyFire(entityManager, entity);
            }

            // Destroy enemies that go off screen
            if (transform.position.x < -50.0f)
            {
                entity->destroy();
            }
        }
    }

  private:
    void enemyFire(EntityManager &entityManager, Entity *enemy)
    {
        auto &enemyComponent = enemy->getComponent<EnemyComponent>();
        if (enemyComponent.shootingType == 1)
        {

            auto &transform = enemy->getComponent<TransformComponent>();

            // Create projectile entity
            auto &projectile = entityManager.createEntity();

            // Position the projectile at the enemy's position
            projectile.addComponent<TransformComponent>(
                transform.position.x - 20.0f, // Offset to fire from the front of the enemy
                transform.position.y + 10.0f  // Center height
            );

            // Add projectile components
            projectile.addComponent<VelocityComponent>(-200.0f, 0.0f); // Fast horizontal movement
            projectile.addComponent<SpriteComponent>();                // Add sprite (different from
                                                                       // player projectiles)
            projectile.addComponent<ColliderComponent>(10.0f,
                                                       5.0f);                         // Small hitbox
            projectile.addComponent<ProjectileComponent>(5.0f, 3.0f, enemy->getID()); // Damage, lifetime, owner
        }
        else if (enemyComponent.shootingType == 2)
        {
            auto &transform = enemy->getComponent<TransformComponent>();

            for (int i = 0; i < 3; i++)
            {
                // std::cout << "i = " << i;
                auto &projectile = entityManager.createEntity();
                projectile.addComponent<TransformComponent>(
                    transform.position.x - 20.0f, // Offset to fire from the front of the enemy
                    transform.position.y + 10.0f  // Center height
                );
                projectile.addComponent<SpriteComponent>(10.0f, 5.0f, 255, 255, 0);
                projectile.addComponent<VelocityComponent>(-200.0f, (i - 1) * 50.0f); // Spread pattern
                projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
            }
            // std::cout << std::endl;
        }
    }
};

// Flappy Bird specific systems - SHOULD BE GENERIC
class GravitySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<VelocityComponent, GravityComponent>();

        for (auto &entity : entities)
        {
            auto &velocity = entity->getComponent<VelocityComponent>();
            auto &gravity = entity->getComponent<GravityComponent>();

            // Apply gravity
            velocity.velocity.y += gravity.gravity * deltaTime;

            // Cap velocity at terminal velocity
            if (velocity.velocity.y > gravity.terminalVelocity)
            {
                velocity.velocity.y = gravity.terminalVelocity;
            }
        }
    }
};

// GENERIC Jump System - replaces BirdSystem
class JumpSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<VelocityComponent, InputComponent, JumpComponent>();

        for (auto &entity : entities)
        {
            auto &velocity = entity->getComponent<VelocityComponent>();
            auto &input = entity->getComponent<InputComponent>();
            auto &jump = entity->getComponent<JumpComponent>();

            // Jump when fire is pressed and can jump
            if (input.fire && jump.canJump)
            {
                velocity.velocity.y = jump.jumpStrength;
                input.fire = false; // Reset input
            }
        }
    }
};

// GENERIC Boundary System - replaces bird boundary check
class BoundarySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();

            // Generic boundary checking - could be configurable
            if (transform.position.y < 0 || transform.position.y > 600 - 32)
            {
                // Mark entity as dead or destroy it
                entity->destroy();
            }
        }
    }
};

// GENERIC Cleanup System - replaces PipeSystem cleanup
class OffscreenCleanupSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();

            // Generic offscreen cleanup
            if (transform.position.x < -100.0f)
            {
                entity->destroy();
            }
        }
    }
};
